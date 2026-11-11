import glob
import os
import numpy as np
import pyvista as pv
import matplotlib.pyplot as plt
import matplotlib.animation as animation

def create_vti_animation(time_steps_data, output_dir):
    """
    Generates an animated MP4 (or GIF fallback) video from pre-computed time step deviations.
    """
    print("Generating animated deviation video... (takes a bit. maybe around 1-2 min)")
    
    first_step = time_steps_data[0]
    point_fields = first_step["deviations"]["point_data"]
    
    if not point_fields:
        print("No point data fields found for animation.")
        return

    number_of_fields = len(point_fields)
    fig, axes = plt.subplots(
        2, number_of_fields,
        figsize=(6 * number_of_fields, 10),
        squeeze=False
    )

    extent = first_step["extent"]
    images_abs = []
    images_rel = []

    # Initialize artists ONCE for high performance (avoiding recreation overhead)
    for i, (field_name, diff_data) in enumerate(point_fields.items()):
        abs_data = diff_data["absolute"]
        rel_data = diff_data["relative"]

        # Absolute deviation subplot
        img_abs = axes[0, i].imshow(
            abs_data,
            origin="lower",
            extent=extent,
            aspect="equal",
            cmap="viridis"
        )
        axes[0, i].set_title(f"{field_name} - Absolute")
        axes[0, i].set_xlabel("x")
        axes[0, i].set_ylabel("y")
        fig.colorbar(img_abs, ax=axes[0, i], label="Absolute deviation")
        images_abs.append(img_abs)

        # Relative deviation subplot
        img_rel = axes[1, i].imshow(
            rel_data,
            origin="lower",
            extent=extent,
            aspect="equal",
            cmap="viridis"
        )
        axes[1, i].set_title(f"{field_name} - Relative")
        axes[1, i].set_xlabel("x")
        axes[1, i].set_ylabel("y")
        fig.colorbar(img_rel, ax=axes[1, i], label="Relative deviation")
        images_rel.append(img_rel)

    fig.suptitle(f"Deviations: {first_step['filename']} (Time Step 1/{len(time_steps_data)})", fontsize=14)
    fig.tight_layout()

    def update(frame_idx):
        step = time_steps_data[frame_idx]
        filename = step["filename"]
        point_fields = step["deviations"]["point_data"]
        
        fig.suptitle(f"Deviations: {filename} (Time Step {frame_idx + 1}/{len(time_steps_data)})", fontsize=14)
        
        for i, (field_name, diff_data) in enumerate(point_fields.items()):
            abs_arr = diff_data["absolute"]
            rel_arr = diff_data["relative"]
            
            images_abs[i].set_data(abs_arr)
            images_rel[i].set_data(rel_arr)
            
            # Update color limits dynamically per frame if needed
            images_abs[i].set_clim(vmin=abs_arr.min(), vmax=max(abs_arr.max(), 1e-12))
            images_rel[i].set_clim(vmin=rel_arr.min(), vmax=max(rel_arr.max(), 1e-12))
            
        return images_abs + images_rel

    ani = animation.FuncAnimation(
        fig,
        update,
        frames=len(time_steps_data),
        interval=100,  # ~10 fps
        blit=False
    )

    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, "vti_deviations_animation.mp4")
    
    # Save as MP4 using FFMpegWriter, with fallback to GIF if ffmpeg is unavailable
    try:
        writer = animation.FFMpegWriter(fps=10, metadata=dict(artist='VTI Comparator'), bitrate=2000)
        ani.save(output_path, writer=writer, dpi=150)
        print(f"Animation saved successfully to {output_path}")
    except Exception as e:
        print(f"FFmpeg writer failed ({e}), saving as GIF instead...")
        gif_path = os.path.join(output_dir, "vti_deviations_animation.gif")
        ani.save(gif_path, writer='pillow', dpi=100)
        print(f"Animation saved successfully to {gif_path}")

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
    time_steps_data = []

    print("Analyzing VTI directories and collecting time step data...")
    for f1, f2 in zip(files1, files2):
        grid1 = pv.read(f1)
        grid2 = pv.read(f2)

        filename = os.path.basename(f1)
        print(f"Processing {filename}...")

        if grid1.dimensions != grid2.dimensions:
            print(f"  Grid dimension mismatch: {grid1.dimensions} vs {grid2.dimensions}")
            failed = True
            continue

        nx, ny, nz = grid1.dimensions
        if nz != 1:
            print(f"  Skipping 3D grid for 2D plotting: {filename}")
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
                    print(f"  Shape mismatch for {data_type} '{field_name}': {arr1.shape} vs {arr2.shape}")
                    failed = True
                    continue

                absolute_diff = np.abs(arr1 - arr2)
                relative_diff = np.zeros_like(absolute_diff, dtype=float)

                np.divide(
                    absolute_diff,
                    np.abs(arr2),
                    out=relative_diff,
                    where=(arr2 != 0)
                )

                relative_diff[(arr2 == 0) & (absolute_diff != 0)] = np.inf

                max_diff = np.max(absolute_diff) if absolute_diff.size else 0.0
                status = "MATCH" if max_diff <= tolerance else "DIFFERENT"

                print(f"  [{data_type}: {field_name}] Max Diff: {max_diff:.3e} -> {status}")

                if max_diff > tolerance:
                    failed = True

                # Process point data shape for 2D spatial plotting
                abs_plot = absolute_diff
                rel_plot = relative_diff
                if data_type == "point_data":
                    if abs_plot.ndim > 1:
                        abs_plot = np.linalg.norm(abs_plot, axis=1)
                        rel_plot = np.linalg.norm(rel_plot, axis=1)
                    abs_plot = abs_plot.reshape((ny, nx))
                    rel_plot = rel_plot.reshape((ny, nx))

                deviations[data_type][field_name] = {
                    "absolute": abs_plot,
                    "relative": rel_plot
                }

        extent = (
            grid1.origin[0],
            grid1.origin[0] + (nx - 1) * grid1.spacing[0],
            grid1.origin[1],
            grid1.origin[1] + (ny - 1) * grid1.spacing[1]
        )

        time_steps_data.append({
            "filename": filename,
            "extent": extent,
            "deviations": deviations
        })

    if generatePlot and time_steps_data:
        create_vti_animation(time_steps_data, plot_dir)

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