import glob
import os
import numpy as np
import pyvista as pv
import matplotlib.pyplot as plt

def plot_vti_deviations(grid, deviations, filename, output_dir):
    point_fields = deviations["point_data"]

    if not point_fields:
        return

    nx, ny, nz = grid.dimensions

    if nz != 1:
        print(f"Plotting for 3D grids not implemented: {filename}")
        return

    number_of_fields = len(point_fields)

    fig, axes = plt.subplots(
        2, number_of_fields,
        figsize=(6 * number_of_fields, 10),
        squeeze=False
    )

    for i, (field_name, diff_data) in enumerate(point_fields.items()):
        absolute_diff = diff_data["absolute"]
        relative_diff = diff_data["relative"]

        # Vector fields: calculate the Euclidean norm
        if absolute_diff.ndim > 1:
            absolute_diff = np.linalg.norm(absolute_diff, axis=1)
            relative_diff = np.linalg.norm(relative_diff, axis=1)

        absolute_diff = absolute_diff.reshape((ny, nx))
        relative_diff = relative_diff.reshape((ny, nx))

        extent = (
            grid.origin[0],
            grid.origin[0] + (nx - 1) * grid.spacing[0],
            grid.origin[1],
            grid.origin[1] + (ny - 1) * grid.spacing[1]
        )

        # Absolute deviation
        image_abs = axes[0, i].imshow(
            absolute_diff,
            origin="lower",
            extent=extent,
            aspect="equal",
            cmap="viridis"
        )

        axes[0, i].set_title(f"{field_name} - Absolute")
        axes[0, i].set_xlabel("x")
        axes[0, i].set_ylabel("y")
        fig.colorbar(
            image_abs,
            ax=axes[0, i],
            label="Absolute deviation"
        )

        # Relative deviation
        image_rel = axes[1, i].imshow(
            relative_diff,
            origin="lower",
            extent=extent,
            aspect="equal",
            cmap="viridis"
        )

        axes[1, i].set_title(f"{field_name} - Relative")
        axes[1, i].set_xlabel("x")
        axes[1, i].set_ylabel("y")
        fig.colorbar(
            image_rel,
            ax=axes[1, i],
            label="Relative deviation"
        )

    fig.suptitle(f"Deviations: {filename}")
    fig.tight_layout()

    os.makedirs(output_dir, exist_ok=True)

    plot_filename = os.path.splitext(filename)[0] + ".png"
    fig.savefig(os.path.join(output_dir, plot_filename), dpi=150)
    plt.close(fig)

def compare_vti_directories(dir1, dir2, generatePlot, tolerance=1e-6):
    files1 = sorted(glob.glob(os.path.join(dir1, "output_*.vti")))
    files2 = sorted(glob.glob(os.path.join(dir2, "output_*.vti")))

    if not files1 or not files2:
        print("Error: No .vti files found in one or both directories.")
        return False

    if len(files1) != len(files2):
        print(f"Warning: File count mismatch ({len(files1)} vs {len(files2)})")

    failed = False
    plot_dir = "test/plots/vti_deviation_plots"

    for f1, f2 in zip(files1, files2):
        # These refer to the grid thats described by the current iterated vti file
        grid1 = pv.read(f1)
        grid2 = pv.read(f2)

        filename = os.path.basename(f1)
        print(f"Analyzing {filename}...")

        if grid1.dimensions != grid2.dimensions:
            print(f"  Grid dimension mismatch: {grid1.dimensions} vs {grid2.dimensions}")
            failed = True
            continue

        deviations = {
            "point_data": {},
            "cell_data": {},
            "field_data": {}
        }

        for data_type in ["point_data", "cell_data", "field_data"]:
            data1 = getattr(grid1, data_type)
            data2 = getattr(grid2, data_type)

            if set(data1.keys()) != set(data2.keys()):
                print(f"  Data fields differ in {data_type}")
                failed = True

            for field_name in data1.keys():
                if field_name not in data2:
                    continue

                arr1 = np.asarray(data1[field_name])
                arr2 = np.asarray(data2[field_name])

                if arr1.shape != arr2.shape:
                    print(
                        f"  Shape mismatch for {data_type} '{field_name}': "
                        f"{arr1.shape} vs {arr2.shape}"
                    )
                    failed = True
                    continue

                absolute_diff = np.abs(arr1 - arr2)

                # Relative deviation, using grid2 as reference
                relative_diff = np.zeros_like(absolute_diff, dtype=float)

                np.divide(
                    absolute_diff,
                    np.abs(arr2),
                    out=relative_diff,
                    where=(arr2 != 0)
                )

                # Reference is zero, but generated value is not zero
                relative_diff[(arr2 == 0) & (absolute_diff != 0)] = np.inf

                max_diff = np.max(absolute_diff) if absolute_diff.size else 0.0

                status = "MATCH" if max_diff <= tolerance else "DIFFERENT"

                print(
                    f"  [{data_type}: {field_name}] "
                    f"Max Diff: {max_diff:.3e} -> {status}"
                )

                if max_diff > tolerance:
                    failed = True

                deviations[data_type][field_name] = {
                    "absolute": absolute_diff,
                    "relative": relative_diff
                }

        if generatePlot:
            plot_vti_deviations(
                grid1,
                deviations,
                filename,
                plot_dir
            )

    return failed

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) == 3:
        generated_output = sys.argv[1]
        reference_output = sys.argv[2]
        
        fail = compare_vti_directories(generated_output, reference_output, True)
        
        if fail:
            sys.exit(1)
    else:
        print("Usage: python3 test/tools/compare_vti_outputs.py <generated_output> <reference_output>")
        sys.exit(1)