import re
import glob
import os
import numpy as np
import pyvista as pv
import matplotlib.pyplot as plt
import sys

def read_txt_file(filename):
    """
    Converts a txt file into a dictionary.
    """

    number_pattern = re.compile(
        r'[-+]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?'
    )

    data = {}

    with open(filename, "r", encoding="utf-8") as file:
        lines = file.readlines()

    # Metadata
    for line in lines:
        if line.startswith("t:"):
            data["t"] = float(line.split(":")[1])

        elif line.startswith("nCells:"):
            values = number_pattern.findall(line)
            data["nCells"] = (int(values[0]), int(values[1]))
            data["dx"] = float(values[2])
            data["dy"] = float(values[3])

    i = 0

    while i < len(lines):
        line = lines[i].strip()

        match = re.match(r"^(\w+)\s+\((\d+)x(\d+)\):", line)

        if match:
            name = match.group(1)

            # Format: (Columns x Rows)
            cols = int(match.group(2))
            rows = int(match.group(3))

            # Skip header
            i += 1

            column_indices = [
                int(x) for x in number_pattern.findall(
                    lines[i].split("|", 1)[1]
                )
            ]

            # Skip ------ line
            i += 2

            row_indices = []
            matrix = []

            for _ in range(rows):
                index_part, values_part = lines[i].split("|", 1)

                row_index = int(index_part.strip())

                row_values = [
                    float(x) for x in number_pattern.findall(values_part)
                ]

                row_indices.append(row_index)
                matrix.append(row_values)

                i += 1

            array = {
                row_idx: {
                    col_idx: matrix[row_pos][col_pos]
                    for col_pos, col_idx in enumerate(column_indices)
                }
                for row_pos, row_idx in enumerate(row_indices)
            }

            data[name] = {
                "values": array,
                "rows": row_indices,
                "cols": column_indices
            }

        else:
            i += 1

    return data

def compare_grids(grid1, grid2):
    deviations = {}

    def calculate_errors(values1, values2):
        absolute_error = abs(values1 - values2)

        if values2 == 0:
            relative_error = (
                0.0 if absolute_error == 0 else float("inf")
            )
        else:
            relative_error = absolute_error / abs(values2)

        return absolute_error, relative_error

    def store_errors(field, absolute_error, relative_error):
        deviations[field] = {
            "absolute": absolute_error,
            "relative": relative_error
        }

    # Scalar values
    for field in ["t", "dx", "dy"]:
        abs_error, rel_error = calculate_errors(
            grid1[field], grid2[field]
        )
        store_errors(field, abs_error, rel_error)

    # nCells: exact comparison
    n_cells_error = (
        0.0 if grid1["nCells"] == grid2["nCells"] else float("inf")
    )
    store_errors("nCells", n_cells_error, n_cells_error)

    # Matrix fields
    matrix_fields = ["u", "v", "p", "F", "G", "rhs"]

    for field in matrix_fields:
        matrix1 = grid1[field]["values"]
        matrix2 = grid2[field]["values"]

        max_absolute_error = 0.0
        max_relative_error = 0.0

        # Check row indices
        if matrix1.keys() != matrix2.keys():
            max_absolute_error = float("inf")
            max_relative_error = float("inf")

        else:
            for row_idx in matrix1:
                row1 = matrix1[row_idx]
                row2 = matrix2[row_idx]

                # Check column indices
                if row1.keys() != row2.keys():
                    max_absolute_error = float("inf")
                    max_relative_error = float("inf")
                    break

                for col_idx in row1:
                    abs_error, rel_error = calculate_errors(
                        row1[col_idx], row2[col_idx]
                    )

                    max_absolute_error = max(
                        max_absolute_error, abs_error
                    )
                    max_relative_error = max(
                        max_relative_error, rel_error
                    )

        store_errors(
            field,
            max_absolute_error,
            max_relative_error
        )

    return deviations

def plot_deviations(all_deviations, dir1, dir2):
    if not all_deviations:
        print("No deviations to plot.")
        return

    import textwrap

    filenames = list(all_deviations.keys())
    fields = [
        "t", "dx", "dy", "nCells",
        "u", "v", "p", "F", "G", "rhs"
    ]
    x = np.arange(len(filenames))

    # At most 8 decorations on x-axis
    tick_indices = np.unique(
        np.linspace(
            0, len(filenames) - 1,
            min(8, len(filenames)),
            dtype=int
        )
    )

    for field in fields:
        absolute_errors = [
            all_deviations[name][field]["absolute"]
            for name in filenames
        ]
        relative_errors = [
            all_deviations[name][field]["relative"]
            for name in filenames
        ]

        fig, axes = plt.subplots(
            2, 1, figsize=(12, 8), sharex=True
        )

        # Absolute error
        axes[0].plot(x, absolute_errors, marker="o")
        axes[0].set_title(f"{field}: Absolute Error")
        axes[0].set_ylabel("Absolute Error")
        axes[0].grid(True)

        # Relative error
        axes[1].plot(x, relative_errors, marker="o")
        axes[1].set_title(f"{field}: Relative Error")
        axes[1].set_ylabel("Relative Error")
        axes[1].set_xlabel("File")
        axes[1].grid(True)

        axes[1].set_xticks(tick_indices)
        axes[1].set_xticklabels(
            [filenames[i] for i in tick_indices],
            rotation=45,
            ha="right"
        )

        fig.suptitle(
            "Compared directories:\n"
            f"{dir1} vs. {dir2}",
            fontsize=10
        )

        fig.tight_layout(rect=[0, 0, 1, 0.92])

    plt.show()


def compare_txt_directories(
    dir1,
    dir2,
    generatePlot,
    absolute_tolerance=1e-6,
    relative_tolerance=1e-6
):
    files1 = sorted(glob.glob(os.path.join(dir1, "output_*.txt")))
    files2 = sorted(glob.glob(os.path.join(dir2, "output_*.txt")))

    if not files1 or not files2:
        print("Error: No .txt files found in one or both directories.")
        return {}

    if len(files1) != len(files2):
        print(f"Warning: File count mismatch ({len(files1)} vs {len(files2)})")

    failed = False
    all_deviations = {}

    for f1, f2 in zip(files1, files2):
        grid1 = read_txt_file(f1)
        grid2 = read_txt_file(f2)

        filename = os.path.basename(f1)
        print(f"Analyzing {filename}...")

        deviations = compare_grids(grid1, grid2)
        all_deviations[filename] = deviations

        for field, errors in deviations.items():
            abs_error = errors["absolute"]
            rel_error = errors["relative"]

            if (
                abs_error > absolute_tolerance
                or rel_error > relative_tolerance
            ):
                print(
                    f"WARNING: {filename}, field {field}: "
                    f"absolute error = {abs_error:.6e}, "
                    f"relative error = {rel_error:.6e}"
                )
                failed = True

    # Generate plots
    if generatePlot:
        plot_deviations(all_deviations, dir1, dir2)

    return failed

if __name__ == "__main__":
    import sys

    if len(sys.argv) == 1:
        # First compare lid_driven_cavity outputs
        print("Running tests to compare the generated output of our project with the reference output for the lid driven cavity scenario.")
        generated_output = "test\outputs\lid_driven_cavity_output"
        reference_output = "test\outputs\lid_driven_cavity_output"
        fail_lcd = compare_txt_directories(generated_output, reference_output, True)
        
        # Second, compare scenario3 outputs
        print("Running tests to compare the generated output of our project with the reference output for scenario 3.")
        generated_output = "test\outputs\scenario3_output"
        reference_output = "test\outputs\scenario3_output"
        fail_s3 = compare_txt_directories(generated_output, reference_output, True)

        if fail_lcd or fail_s3:
            sys.exit(1)

    elif len(sys.argv) == 4:
        compare_txt_directories(sys.argv[1], sys.argv[2], sys.argv[3])

    else:
        sys.exit(1)
        
"""
Usage:
python3 Exercise1/Submission/test/tools/compare_txt_outputs.py path/to/first/out path/to/second/out generatePlot
"""
