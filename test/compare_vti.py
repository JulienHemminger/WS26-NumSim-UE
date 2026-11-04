import sys
import os
import glob
import subprocess
import numpy as np
import xml.etree.ElementTree as ET

def print_dir_tree(path):
    print(f"\n--- Directory Tree for: {path} ---")
    try:
        subprocess.run(["tree", path], check=True)
    except (subprocess.SubprocessError, FileNotFoundError):
        for root, dirs, files in os.walk(path):
            level = root.replace(path, '').count(os.sep)
            indent = ' ' * 4 * level
            print(f"{indent}{os.path.basename(root)}/")
            subindent = ' ' * 4 * (level + 1)
            for f in files:
                print(f"{subindent}{f}")
    print("----------------------------------------\n")

def parse_vti_arrays(filepath):
    try:
        tree = ET.parse(filepath)
        root = tree.getroot()
    except Exception as e:
        print(f"Error parsing XML for {filepath}: {e}")
        return None

    arrays = {}
    for elem in root.iter():
        if elem.tag.endswith('DataArray'):
            name = elem.attrib.get('Name', 'unnamed')
            if elem.text:
                vals = np.fromstring(elem.text, sep=' ')
                if vals.size > 0:
                    arrays[name] = vals
    return arrays

def compare_directories(ref_dir, gen_dir, tol=1e-5):
    print(f"\nComparing reference directory: {ref_dir}")
    print(f"Against generated directory:  {gen_dir}")
    
    if not os.path.exists(ref_dir):
        print(f"ERROR: Reference directory does not exist: {ref_dir}")
        print_dir_tree(os.path.dirname(os.path.abspath(ref_dir)) or '.')
        sys.exit(1)
    if not os.path.exists(gen_dir):
        print(f"ERROR: Generated output directory does not exist: {gen_dir}")
        print("Hint: Did the simulation executable fail to run or write output?")
        print_dir_tree(os.path.dirname(os.path.abspath(gen_dir)) or '.')
        sys.exit(1)

    ref_files = sorted(glob.glob(os.path.join(ref_dir, "*.vti")))
    gen_files = sorted(glob.glob(os.path.join(gen_dir, "*.vti")))
    
    if not ref_files:
        print(f"ERROR: No .vti reference files found in {ref_dir}")
        print_dir_tree(ref_dir)
        sys.exit(1)
    if not gen_files:
        print(f"ERROR: No .vti generated output files found in {gen_dir}")
        print_dir_tree(gen_dir)
        sys.exit(1)
    if len(ref_files) != len(gen_files):
        print(f"WARNING / ERROR: File count mismatch! Reference has {len(ref_files)} files, generated has {len(gen_files)} files.")
        print_dir_tree(ref_dir)
        print_dir_tree(gen_dir)
        sys.exit(1)
    overall_success = True
    for i, (ref_f, gen_f) in enumerate(zip(ref_files, gen_files)):
        filename = os.path.basename(ref_f)
        ref_data = parse_vti_arrays(ref_f)
        gen_data = parse_vti_arrays(gen_f)
        
        if ref_data is None or gen_data is None:
            overall_success = False
            break
            
        ref_keys = set(ref_data.keys())
        gen_keys = set(gen_data.keys())
        if ref_keys != gen_keys:
            print(f"[{filename}] MISMATCH: Data arrays differ. Ref: {ref_keys}, Gen: {gen_keys}")
            overall_success = False
            continue
            
        file_passed = True
        diff_str_parts = []
        
        for key in ref_keys:
            r_arr = ref_data[key]
            g_arr = gen_data[key]
            
            if r_arr.shape != g_arr.shape:
                print(f"[{filename}] Shape mismatch for array '{key}': {r_arr.shape} vs {g_arr.shape}")
                file_passed = False
                continue
                
            max_diff = np.max(np.abs(r_arr - g_arr))
            diff_str_parts.append(f"{key} Max Diff: {max_diff:.3e}")
            
            if not np.allclose(r_arr, g_arr, atol=tol, rtol=tol):
                file_passed = False
                
        diff_summary = " | ".join(diff_str_parts)
        status = "PASSED" if file_passed else "FAILED (Exceeds tolerance)"
        print(f"File {i+1}/{len(ref_files)} ({filename}) -> {status} | {diff_summary}")
        
        if not file_passed:
            overall_success = False

    if overall_success:
        print("\nSUCCESS: All VTI files match within tolerance.")
        sys.exit(0)
    else:
        print("\nFAILURE: One or more VTI files failed validation.")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 compare_vti.py <ref_dir> <gen_dir>")
        sys.exit(1)
        
    compare_directories(sys.argv[1], sys.argv[2])