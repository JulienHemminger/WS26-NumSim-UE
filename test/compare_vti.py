import argparse
import glob
import os
import sys

import numpy as np
import vtk
from vtk.util.numpy_support import vtk_to_numpy


def read_vti(path):
    reader = vtk.vtkXMLImageDataReader()
    reader.SetFileName(path)
    reader.Update()

    if reader.GetErrorCode() != vtk.vtkErrorCode.NoError:
        raise ValueError(f"VTK could not read {path}")

    image = reader.GetOutput()
    if image.GetNumberOfPoints() == 0:
        raise ValueError(f"No points found in {path}")
    return image


def arrays_by_name(attributes):
    arrays = {}
    for index in range(attributes.GetNumberOfArrays()):
        array = attributes.GetAbstractArray(index)
        name = array.GetName()
        if name is None:
            raise ValueError("Found an unnamed data array")
        arrays[name] = vtk_to_numpy(array)
    return arrays


def compare_geometry(reference, generated, filename, tolerance):
    checks = (
        ("extent", reference.GetExtent(), generated.GetExtent()),
        ("origin", reference.GetOrigin(), generated.GetOrigin()),
        ("spacing", reference.GetSpacing(), generated.GetSpacing()),
    )
    passed = True
    for label, ref_values, gen_values in checks:
        if not np.allclose(ref_values, gen_values, atol=tolerance, rtol=tolerance):
            print(f"[{filename}] Geometry {label} mismatch: {ref_values} vs {gen_values}")
            passed = False
    return passed


def compare_arrays(reference, generated, filename, tolerance):
    passed = True
    data_sets = (
        ("point", reference.GetPointData(), generated.GetPointData()),
        ("cell", reference.GetCellData(), generated.GetCellData()),
        ("field", reference.GetFieldData(), generated.GetFieldData()),
    )

    ref_point_arrays = arrays_by_name(data_sets[0][1])
    ref_field_arrays = arrays_by_name(data_sets[2][1])
    if not {"pressure", "velocity"}.issubset(ref_point_arrays):
        print(f"[{filename}] Reference point data must contain pressure and velocity arrays")
        passed = False
    if "TIME" not in ref_field_arrays:
        print(f"[{filename}] Reference field data must contain a TIME array")
        passed = False

    for label, ref_attributes, gen_attributes in data_sets:
        ref_arrays = arrays_by_name(ref_attributes)
        gen_arrays = arrays_by_name(gen_attributes)
        if ref_arrays.keys() != gen_arrays.keys():
            print(
                f"[{filename}] {label} arrays differ: "
                f"reference={sorted(ref_arrays)}, generated={sorted(gen_arrays)}"
            )
            passed = False
            continue

        for name, ref_values in ref_arrays.items():
            gen_values = gen_arrays[name]
            if ref_values.shape != gen_values.shape:
                print(
                    f"[{filename}] Shape mismatch for {label} array '{name}': "
                    f"{ref_values.shape} vs {gen_values.shape}"
                )
                passed = False
                continue

            if not np.issubdtype(ref_values.dtype, np.number) or not np.issubdtype(
                gen_values.dtype, np.number
            ):
                if not np.array_equal(ref_values, gen_values):
                    print(f"[{filename}] Non-numeric {label} array '{name}' differs")
                    passed = False
                continue

            max_diff = (
                float(np.max(np.abs(ref_values - gen_values))) if ref_values.size else 0.0
            )
            if not np.allclose(
                ref_values, gen_values, atol=tolerance, rtol=tolerance, equal_nan=False
            ):
                print(
                    f"[{filename}] {label} array '{name}' differs "
                    f"(max absolute difference {max_diff:.3e})"
                )
                passed = False
    return passed


def compare_directories(ref_dir, gen_dir, tolerance):
    print(f"Comparing reference directory: {ref_dir}")
    print(f"Against generated directory:  {gen_dir}")

    for label, path in (("Reference", ref_dir), ("Generated output", gen_dir)):
        if not os.path.isdir(path):
            print(f"ERROR: {label} directory does not exist: {path}")
            return False

    ref_files = sorted(glob.glob(os.path.join(ref_dir, "*.vti")))
    gen_files = sorted(glob.glob(os.path.join(gen_dir, "*.vti")))
    if not ref_files:
        print(f"ERROR: No .vti reference files found in {ref_dir}")
        return False
    if not gen_files:
        print(f"ERROR: No .vti output files found in {gen_dir}")
        return False
    if len(ref_files) != len(gen_files):
        print(
            f"ERROR: File count mismatch: reference has {len(ref_files)} files, "
            f"generated has {len(gen_files)} files"
        )
        return False

    overall_success = True
    for ref_file, gen_file in zip(ref_files, gen_files):
        filename = os.path.basename(ref_file)
        if os.path.basename(gen_file) != filename:
            print(
                f"ERROR: File sequence mismatch: expected {filename}, "
                f"found {os.path.basename(gen_file)}"
            )
            overall_success = False
            continue

        try:
            reference = read_vti(ref_file)
            generated = read_vti(gen_file)
            passed = compare_geometry(reference, generated, filename, tolerance)
            passed &= compare_arrays(reference, generated, filename, tolerance)
        except (RuntimeError, ValueError) as error:
            print(f"[{filename}] ERROR: {error}")
            passed = False

        if not passed:
            overall_success = False

    if overall_success:
        print(f"SUCCESS: All {len(ref_files)} VTI files match within tolerance {tolerance:g}.")
    else:
        print("FAILURE: One or more VTI files failed validation.")
    return overall_success


def main():
    parser = argparse.ArgumentParser(description="Compare two VTK ImageData directories.")
    parser.add_argument("reference_directory")
    parser.add_argument("generated_directory")
    parser.add_argument("--tolerance", type=float, default=1e-5)
    args = parser.parse_args()

    if args.tolerance < 0:
        parser.error("--tolerance must be non-negative")
    return 0 if compare_directories(
        args.reference_directory, args.generated_directory, args.tolerance
    ) else 1


if __name__ == "__main__":
    sys.exit(main())
