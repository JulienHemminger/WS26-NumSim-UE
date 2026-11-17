#!/usr/bin/env python3

from __future__ import annotations

import argparse
import re
import subprocess
import sys
import tempfile
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_CONFIG = REPO_ROOT / "test" / "0_driven_cavity" / "input_parameters.txt"
DEFAULT_BINARY_CANDIDATES = [
    REPO_ROOT / "build" / "numsim",
    REPO_ROOT / "build" / "src" / "numsim",
]
DEFAULT_RESOLUTIONS = ["10", "20", "40", "80"]


def find_binary(explicit: str | None) -> Path:
    if explicit:
        candidate = Path(explicit).expanduser()
        if candidate.is_file():
            return candidate.resolve()
        raise FileNotFoundError(f"Binary '{candidate}' does not exist.")

    for candidate in DEFAULT_BINARY_CANDIDATES:
        if candidate.is_file():
            return candidate.resolve()

    raise FileNotFoundError(
        "No numsim binary was found. Build the project first."
    )


def parse_parameter_value(parameter_name: str, config_path: Path) -> str:
    pattern = re.compile(rf"^\s*{re.escape(parameter_name)}\s*=\s*([^#\s]+)", re.MULTILINE)
    match = pattern.search(config_path.read_text())
    if match is None:
        raise ValueError(f"Parameter '{parameter_name}' was not found in '{config_path}'.")
    return match.group(1)


def create_parameter_file(source: Path, destination: Path, n_cells_x: int, n_cells_y: int) -> None:
    text = source.read_text()

    def replace(current_text: str, name: str, value: str) -> str:
        pattern = re.compile(rf"(?m)^(\s*{re.escape(name)}\s*=\s*)([^#\r\n]+)")
        new_text, count = pattern.subn(rf"\g<1>{value}", current_text, count=1)
        if count != 1:
            raise ValueError(f"Parameter '{name}' could not be replaced in '{source}'.")
        return new_text

    updated = replace(text, "nCellsX", str(n_cells_x))
    updated = replace(updated, "nCellsY", str(n_cells_y))
    destination.write_text(updated)


def run_simulation(binary: Path, parameter_file: Path, working_directory: Path) -> tuple[float, str]:
    start = time.perf_counter()
    completed = subprocess.run(
        [str(binary), str(parameter_file)],
        cwd=str(working_directory),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
        timeout=900,
    )
    elapsed = time.perf_counter() - start
    output = completed.stdout.strip()

    if completed.returncode != 0:
        raise RuntimeError(
            f"Simulation failed for '{parameter_file}' with exit code {completed.returncode}.\n"
            f"Output:\n{output}"
        )

    return elapsed, output


def parse_resolution(value: str) -> tuple[int, int]:
    text = str(value).strip()
    if "x" in text.lower():
        parts = text.lower().split("x")
        if len(parts) != 2:
            raise ValueError(f"Invalid resolution format: '{text}'. Use 'NxN' or 'N'.")
        return (int(parts[0]), int(parts[1]))
    else:
        val = int(text)
        return (val, val)


def build_table(
    resolutions: list[str], config_path: Path, binary: Path
) -> list[tuple[int, int, float]]:
    parsed_resolutions = [parse_resolution(r) for r in resolutions]
    total_runs = len(parsed_resolutions)
    results: list[tuple[int, int, float]] = []

    with tempfile.TemporaryDirectory(prefix="numsim-runtime-") as temporary_directory:
        working_directory = Path(temporary_directory)

        for run_number, (nx, ny) in enumerate(parsed_resolutions, start=1):
            parameter_file = working_directory / f"input_{nx}x{ny}.txt"
            create_parameter_file(config_path, parameter_file, nx, ny)

            print(
                f"\n[{run_number}/{total_runs}] Running {nx}x{ny} simulation...",
                flush=True,
            )
            elapsed, _ = run_simulation(binary, parameter_file, working_directory)
            print(f"[{run_number}/{total_runs}] Finished {nx}x{ny}: {elapsed:.3f} s", flush=True)
            results.append((nx, ny, elapsed))

    return results


def print_table(results: list[tuple[int, int, float]]) -> None:
    print("\nRuntimes:")
    print("| resolution (n x n) | runtime [s] |")
    print("| --- | ---: |")
    for nx, ny, runtime in results:
        print(f"| {nx} x {ny} | {runtime:.3f} |")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Measure the runtime of the solver for several grid resolutions."
    )
    parser.add_argument(
        "--config",
        type=str,
        default=str(DEFAULT_CONFIG),
        help="Path to the input parameter file.",
    )
    parser.add_argument(
        "--binary",
        type=str,
        default=None,
        help="Path to the compiled numsim executable.",
    )
    parser.add_argument(
        "--resolutions",
        type=str,
        nargs="*",
        default=DEFAULT_RESOLUTIONS,
        help="Grid sizes to test (for example: --resolutions 10 20 10x20).",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    config_path = Path(args.config).expanduser()
    if not config_path.is_file():
        raise FileNotFoundError(f"Parameter file '{config_path}' does not exist.")

    if not args.resolutions:
        raise ValueError("Provide at least one grid resolution.")

    parsed_res = [parse_resolution(r) for r in args.resolutions]
    if any(nx <= 0 or ny <= 0 for nx, ny in parsed_res):
        raise ValueError("Grid resolutions must be positive integers.")

    config_path = config_path.resolve()
    binary = find_binary(args.binary)

    print(
        f"Running {len(parsed_res)} grid resolution(s) (1 run each)."
    )
    print(f"Binary: {binary}")
    print(f"Config: {config_path}", flush=True)

    results = build_table(args.resolutions, config_path, binary)
    print_table(results)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (FileNotFoundError, ValueError, RuntimeError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        raise SystemExit(1)