#!/usr/bin/env python3
"""Measure the runtime of the CFD solver for several grid resolutions.

The script reuses the standard configuration from test/0_driven_cavity/input_parameters.txt,
updates the grid size, and executes the compiled simulation for each resolution. The result is a
simple table that can be copied into the report for the Abnahme 1 runtime discussion.
"""

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
DEFAULT_RESOLUTIONS = [10, 20, 40, 80]
REPETITIONS = 10
BASELINE_RESOLUTION = 20


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
        "No numsim binary was found. Build the project first (for example: 'mkdir -p build && cd build && cmake .. && make install')."
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


def build_table(
    resolutions: list[int], config_path: Path, binary: Path
) -> list[tuple[int, float, float]]:
    measured_resolutions = sorted(set(resolutions) | {BASELINE_RESOLUTION})
    total_runs = len(measured_resolutions) * REPETITIONS
    runtimes_by_resolution: dict[int, list[float]] = {}
    run_number = 0

    with tempfile.TemporaryDirectory(prefix="numsim-runtime-") as temporary_directory:
        working_directory = Path(temporary_directory)

        for resolution in measured_resolutions:
            parameter_file = working_directory / f"input_{resolution}x{resolution}.txt"
            create_parameter_file(config_path, parameter_file, resolution, resolution)
            runtimes: list[float] = []

            print(
                f"\nMeasuring {resolution}x{resolution} "
                f"({REPETITIONS} repetitions; {total_runs - run_number} runs remaining)",
                flush=True,
            )
            for repetition in range(1, REPETITIONS + 1):
                run_number += 1
                print(
                    f"[{run_number}/{total_runs}] Starting "
                    f"{resolution}x{resolution}, run {repetition}/{REPETITIONS}...",
                    flush=True,
                )
                elapsed, _ = run_simulation(binary, parameter_file, working_directory)
                runtimes.append(elapsed)
                print(
                    f"[{run_number}/{total_runs}] Finished "
                    f"{resolution}x{resolution}, run {repetition}/{REPETITIONS}: "
                    f"{elapsed:.3f} s",
                    flush=True,
                )

            runtimes_by_resolution[resolution] = runtimes
            average = sum(runtimes) / len(runtimes)
            print(
                f"Completed {resolution}x{resolution}: average {average:.3f} s",
                flush=True,
            )

    baseline_runtimes = runtimes_by_resolution[BASELINE_RESOLUTION]
    baseline_average = sum(baseline_runtimes) / len(baseline_runtimes)
    return [
        (
            resolution,
            sum(runtimes_by_resolution[resolution]) / len(runtimes_by_resolution[resolution]),
            (
                sum(runtimes_by_resolution[resolution])
                / len(runtimes_by_resolution[resolution])
                / baseline_average
            ),
        )
        for resolution in measured_resolutions
    ]


def print_table(results: list[tuple[int, float, float]]) -> None:
    print("\nAverage runtimes (relative values are mean runtime / mean 20x20 runtime):")
    print("| resolution (n x n) | average runtime [s] | relative to 20x20 baseline |")
    print("| --- | ---: | ---: |")
    for resolution, average_runtime, relative in results:
        print(f"| {resolution} x {resolution} | {average_runtime:.3f} | {relative:.2f}x |")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Measure the runtime of the solver for several grid resolutions."
    )
    parser.add_argument(
        "--config",
        type=str,
        default=str(DEFAULT_CONFIG),
        help="Path to the input parameter file. Defaults to test/0_driven_cavity/input_parameters.txt.",
    )
    parser.add_argument(
        "--binary",
        type=str,
        default=None,
        help="Path to the compiled numsim executable. Defaults to the build directory.",
    )
    parser.add_argument(
        "--resolutions",
        type=int,
        nargs="*",
        default=DEFAULT_RESOLUTIONS,
        help="Grid sizes to test (for example: --resolutions 10 20 40 80).",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    config_path = Path(args.config).expanduser()
    if not config_path.is_file():
        raise FileNotFoundError(f"Parameter file '{config_path}' does not exist.")

    if not args.resolutions:
        raise ValueError("Provide at least one grid resolution.")
    if any(resolution <= 0 for resolution in args.resolutions):
        raise ValueError("Grid resolutions must be positive integers.")

    config_path = config_path.resolve()
    binary = find_binary(args.binary)

    resolutions = sorted(set(args.resolutions))
    measured_resolutions = sorted(set(resolutions) | {BASELINE_RESOLUTION})
    print(
        f"Running {len(measured_resolutions)} grid resolutions "
        f"({REPETITIONS} runs each; {len(measured_resolutions) * REPETITIONS} simulations total)."
    )
    if BASELINE_RESOLUTION not in resolutions:
        print("Adding 20x20 as the baseline resolution.")
    print(f"Binary: {binary}")
    print(f"Config: {config_path}", flush=True)

    results = build_table(resolutions, config_path, binary)
    print_table(results)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (FileNotFoundError, ValueError, RuntimeError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        raise SystemExit(1)

