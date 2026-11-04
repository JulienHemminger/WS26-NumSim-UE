import glob
import os
import numpy as np
import pyvista as pv

def compare_vti_directories(dir1, dir2, tolerance=1e-6):
    files1 = sorted(glob.glob(os.path.join(dir1, "output_*.vti")))
    files2 = sorted(glob.glob(os.path.join(dir2, "output_*.vti")))
    
    if not files1 or not files2:
        print("Error: No .vti files found in one or both directories.")
        return

    if len(files1) != len(files2):
        print(f"Warning: File count mismatch ({len(files1)} vs {len(files2)})")

    for f1, f2 in zip(files1, files2):
        grid1 = pv.read(f1)
        grid2 = pv.read(f2)
        
        filename = os.path.basename(f1)
        print(f"Analyzing {filename}...")
        
        for field_name in grid1.point_data.keys():
            arr1 = grid1.point_data[field_name]
            arr2 = grid2.point_data[field_name]
            
            if arr1.shape != arr2.shape:
                print(f"  Shape mismatch for point data '{field_name}': {arr1.shape} vs {arr2.shape}")
                continue
                
            max_diff = np.max(np.abs(arr1 - arr2))
            status = "MATCH" if max_diff <= tolerance else "DIFFERENT"
            print(f"  [Point Data: {field_name}] Max Diff: {max_diff:.3e} -> {status}")

        for field_name in grid1.cell_data.keys():
            arr1 = grid1.cell_data[field_name]
            arr2 = grid2.cell_data[field_name]
            
            if arr1.shape != arr2.shape:
                print(f"  Shape mismatch for cell data '{field_name}': {arr1.shape} vs {arr2.shape}")
                continue
                
            max_diff = np.max(np.abs(arr1 - arr2))
            status = "MATCH" if max_diff <= tolerance else "DIFFERENT"
            print(f"  [Cell Data: {field_name}] Max Diff: {max_diff:.3e} -> {status}")

        for field_name in grid1.field_data.keys():
            arr1 = grid1.field_data[field_name]
            arr2 = grid2.field_data[field_name]
            
            if arr1.shape != arr2.shape:
                print(f"  Shape mismatch for field data '{field_name}': {arr1.shape} vs {arr2.shape}")
                continue
                
            max_diff = np.max(np.abs(arr1 - arr2))
            status = "MATCH" if max_diff <= tolerance else "DIFFERENT"
            print(f"  [Field Data: {field_name}] Max Diff: {max_diff:.3e} -> {status}")

if __name__ == "__main__":
    import sys
    if len(sys.argv) < 3:
        print("Usage: python compare_runs.py <dir_run1> <dir_run2>")
        sys.exit(1)
        
    compare_vti_directories(sys.argv[1], sys.argv[2])

"""
Usage:
python3 Exercise1/Submission/test/tools/compare_raw_outputs.py path/to/first/out path/to/second/out
"""
