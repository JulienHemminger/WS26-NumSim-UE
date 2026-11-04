import sys
import numpy as np
import xml.etree.ElementTree as ET
import glob
import os

def parse_vti(filepath):
    tree = ET.parse(filepath)
    root = tree.getroot()
    data = []
    for elem in root.iter():
        if elem.tag.endswith('DataArray'):
            if elem.text:
                vals = np.fromstring(elem.text, sep=' ')
                if vals.size > 0:
                    data.append(vals)
    return np.concatenate(data) if data else np.array([])

def compare_directories(ref_dir, gen_dir, tol=1e-5):
    ref_files = sorted(glob.glob(os.path.join(ref_dir, "*.vti")))
    gen_files = sorted(glob.glob(os.path.join(gen_dir, "*.vti")))
    
    if len(ref_files) != len(gen_files):
        sys.exit(1)
        
    for ref_f, gen_f in zip(ref_files, gen_files):
        ref_data = parse_vti(ref_f)
        gen_data = parse_vti(gen_f)
        
        if ref_data.shape != gen_data.shape:
            sys.exit(1)
            
        if not np.allclose(ref_data, gen_data, atol=tol, rtol=tol):
            sys.exit(1)
            
    sys.exit(0)

if __name__ == "__main__":
    compare_directories(sys.argv[1], sys.argv[2])