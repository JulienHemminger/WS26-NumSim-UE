import glob
import os
import numpy as np
import pyvista as pv
import matplotlib.pyplot as plt

def analyze_runs(dir1, dir2):
    files1 = sorted(glob.glob(os.path.join(dir1, "output_*.vti")))
    files2 = sorted(glob.glob(os.path.join(dir2, "output_*.vti")))
    
    if not files1 or not files2:
        print("Error: No .vti files found in one or both directories.")
        return

    if len(files1) != len(files2):
        print(f"Warning: File count mismatch ({len(files1)} vs {len(files2)})")

    limit = min(len(files1), len(files2))
    
    times1 = []
    times2 = []
    p_max_diffs = []
    v_max_diffs = []
    
    for i in range(limit):
        grid1 = pv.read(files1[i])
        grid2 = pv.read(files2[i])
        
        t1 = float(grid1.field_data['TIME'][0]) if 'TIME' in grid1.field_data else np.nan
        t2 = float(grid2.field_data['TIME'][0]) if 'TIME' in grid2.field_data else np.nan
        times1.append(t1)
        times2.append(t2)
        
        filename = os.path.basename(files1[i])
        
        p_diff = 0.0
        if 'pressure' in grid1.point_data and 'pressure' in grid2.point_data:
            p_diff = np.max(np.abs(grid1.point_data['pressure'] - grid2.point_data['pressure']))
            p_max_diffs.append(p_diff)
        else:
            p_max_diffs.append(np.nan)
            
        v_diff = 0.0
        if 'velocity' in grid1.point_data and 'velocity' in grid2.point_data:
            v_diff = np.max(np.abs(grid1.point_data['velocity'] - grid2.point_data['velocity']))
            v_max_diffs.append(v_diff)
        else:
            v_max_diffs.append(np.nan)
            
        print(f"{filename} | Pressure Max Diff: {p_diff:.3e} | Velocity Max Diff: {v_diff:.3e}")

    times1 = np.array(times1)
    times2 = np.array(times2)
    
    print("\n--- Time Series Analysis ---")
    dt1 = np.diff(times1)
    dt2 = np.diff(times2)
    
    fixed1 = np.allclose(dt1, dt1[0], atol=1e-7) if len(dt1) > 0 else False
    fixed2 = np.allclose(dt2, dt2[0], atol=1e-7) if len(dt2) > 0 else False
    
    print(f"Run 1 - Fixed time step: {fixed1} (dt approx: {dt1[0] if len(dt1)>0 else 'N/A'})")
    print(f"Run 2 - Fixed time step: {fixed2} (dt approx: {dt2[0] if len(dt2)>0 else 'N/A'})")
    
    if not np.allclose(times1, times2, atol=1e-7):
        print("Time series status: MISMATCH")
        if np.allclose(times1 - times1[0], times2 - times2[0], atol=1e-7):
            print("Diagnosis: Time offset detected (same progression, different start time/shift).")
        elif fixed1 and fixed2 and len(dt1) > 0 and len(dt2) > 0:
            print(f"Diagnosis: Different fixed step sizes ({dt1[0]:.4e} vs {dt2[0]:.4e}).")
        else:
            print("Diagnosis: Time steps are irregular or diverging progressively.")
    else:
        print("Time series status: IDENTICAL")

    plt.figure(figsize=(10, 5))
    plt.plot(range(limit), p_max_diffs, label='Pressure Max Diff', marker='o', markersize=2)
    plt.plot(range(limit), v_max_diffs, label='Velocity Max Diff', marker='x', markersize=2)
    plt.yscale('log')
    plt.xlabel('File Index')
    plt.ylabel('Max Absolute Difference')
    plt.title('Error Growth Over Time (First 100 Files)')
    plt.legend()
    plt.grid(True, which="both", ls="--")
    plt.savefig('comparison_plot.png', dpi=300)
    print("\nPlot generated and saved successfully as 'comparison_plot.png'.")

if __name__ == "__main__":
    import sys
    if len(sys.argv) < 3:
        print("Usage: python compare_runs.py <dir_run1> <dir_run2>")
        sys.exit(1)
        
    analyze_runs(sys.argv[1], sys.argv[2])

"""
Usage:
python3 Exercise1/Submission/test/tools/compare_vti_outputs.py path/to/first/out path/to/second/out
"""
