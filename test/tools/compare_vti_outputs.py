import argparse
import glob
import os
import sys

import numpy as np
import vtk
from vtk.util.numpy_support import vtk_to_numpy


def read_series(directory):
    files = sorted(glob.glob(os.path.join(directory, "output_*.vti")))
    if not files:
        raise ValueError(f"No VTI files found in {directory}")

    times = []
    frames = []
    geometry = None
    expected_names = None
    for path in files:
        reader = vtk.vtkXMLImageDataReader()
        reader.SetFileName(path)
        reader.Update()
        if reader.GetErrorCode() != vtk.vtkErrorCode.NoError:
            raise ValueError(f"VTK could not read {path}")

        image = reader.GetOutput()
        time_array = image.GetFieldData().GetArray("TIME")
        if time_array is None or time_array.GetNumberOfTuples() != 1:
            raise ValueError(f"Missing scalar TIME field in {path}")
        time = float(time_array.GetValue(0))
        if not np.isfinite(time):
            raise ValueError(f"Non-finite TIME value in {path}")
        times.append(time)

        current_geometry = (
            image.GetExtent(),
            image.GetOrigin(),
            image.GetSpacing(),
        )
        if geometry is None:
            geometry = current_geometry
        elif current_geometry != geometry:
            raise ValueError(f"Mesh geometry changes within {directory}: {path}")

        point_data = image.GetPointData()
        arrays = {}
        for index in range(point_data.GetNumberOfArrays()):
            array = point_data.GetArray(index)
            name = array.GetName()
            if name is None:
                raise ValueError(f"Unnamed point-data array in {path}")
            arrays[name] = vtk_to_numpy(array).copy()
        if not {"pressure", "velocity"}.issubset(arrays):
            raise ValueError(f"Missing pressure or velocity array in {path}")
        if expected_names is None:
            expected_names = set(arrays)
        elif set(arrays) != expected_names:
            raise ValueError(f"Point-data arrays change within {directory}: {path}")
        frames.append(arrays)

    times = np.asarray(times)
    if np.any(np.diff(times) <= 0):
        raise ValueError(f"TIME values must be strictly increasing in {directory}")
    return files, times, frames, geometry


def interpolate_frame(times, frames, time):
    upper = int(np.searchsorted(times, time, side="left"))
    if upper == 0:
        if not np.isclose(times[0], time, atol=1e-10, rtol=0):
            raise ValueError(f"Generated output starts after reference time {time:g}")
        return frames[0]
    if upper == len(times):
        if not np.isclose(times[-1], time, atol=1e-10, rtol=0):
            raise ValueError(f"Generated output ends before reference time {time:g}")
        return frames[-1]
    if np.isclose(times[upper], time, atol=1e-10, rtol=0):
        return frames[upper]

    lower = upper - 1
    weight = (time - times[lower]) / (times[upper] - times[lower])
    return {
        name: frames[lower][name]
        + weight * (frames[upper][name] - frames[lower][name])
        for name in frames[lower]
    }


def maximum_difference(reference, generated):
    if reference.shape != generated.shape:
        raise ValueError(f"Array shape mismatch: {reference.shape} vs {generated.shape}")
    if not np.array_equal(np.isnan(reference), np.isnan(generated)):
        return float("inf")
    if not np.array_equal(np.isposinf(reference), np.isposinf(generated)):
        return float("inf")
    if not np.array_equal(np.isneginf(reference), np.isneginf(generated)):
        return float("inf")

    finite = np.isfinite(reference)
    if not np.any(finite):
        return 0.0
    return float(np.max(np.abs(reference[finite] - generated[finite])))


def compare_directories(
    reference_directory,
    generated_directory,
    value_tolerance,
    mean_dt_relative_tolerance,
    endpoint_tolerance,
):
    print(f"Reference: {reference_directory}")
    print(f"Generated: {generated_directory}")
    ref_files, ref_times, ref_frames, ref_geometry = read_series(reference_directory)
    gen_files, gen_times, gen_frames, gen_geometry = read_series(generated_directory)

    if ref_geometry[0] != gen_geometry[0]:
        raise ValueError(f"Mesh extents differ: {ref_geometry[0]} vs {gen_geometry[0]}")
    for name, ref_values, gen_values in zip(
        ("origin", "spacing"), ref_geometry[1:], gen_geometry[1:]
    ):
        if not np.allclose(ref_values, gen_values, atol=1e-12, rtol=0):
            raise ValueError(f"Mesh {name} differs: {ref_values} vs {gen_values}")

    if not np.allclose(
        (ref_times[0], ref_times[-1]),
        (gen_times[0], gen_times[-1]),
        atol=endpoint_tolerance,
        rtol=0,
    ):
        raise ValueError(
            "Simulation time endpoints differ: "
            f"reference=({ref_times[0]:.12g}, {ref_times[-1]:.12g}), "
            f"generated=({gen_times[0]:.12g}, {gen_times[-1]:.12g})"
        )

    ref_mean_dt = float(np.mean(np.diff(ref_times)))
    gen_mean_dt = float(np.mean(np.diff(gen_times)))
    dt_relative_difference = abs(ref_mean_dt - gen_mean_dt) / ref_mean_dt
    print(
        f"Frames: reference={len(ref_files)}, generated={len(gen_files)}; "
        f"mean dt: reference={ref_mean_dt:.8g}, generated={gen_mean_dt:.8g} "
        f"(relative difference {dt_relative_difference:.2%})"
    )
    if dt_relative_difference > mean_dt_relative_tolerance:
        raise ValueError(
            f"Mean time-step difference {dt_relative_difference:.2%} exceeds "
            f"{mean_dt_relative_tolerance:.2%}"
        )

    max_differences = {"pressure": 0.0, "velocity": 0.0}
    for time, ref_arrays in zip(ref_times, ref_frames):
        gen_arrays = interpolate_frame(gen_times, gen_frames, time)
        if ref_arrays.keys() != gen_arrays.keys():
            raise ValueError("Reference and generated point-data arrays differ")
        for name in max_differences:
            difference = maximum_difference(ref_arrays[name], gen_arrays[name])
            max_differences[name] = max(max_differences[name], difference)

    success = True
    for name, difference in max_differences.items():
        status = "PASS" if difference <= value_tolerance else "FAIL"
        print(
            f"{status}: {name} maximum absolute difference "
            f"{difference:.6e} (tolerance {value_tolerance:.6e})"
        )
        success &= difference <= value_tolerance

    if success:
        print("SUCCESS: Reference comparison passed.")
    else:
        print("FAILURE: Field differences exceed tolerance.")
    return success


def main():
    parser = argparse.ArgumentParser(
        description="Compare VTI outputs against reference data at matching physical times."
    )
    parser.add_argument("reference_directory")
    parser.add_argument("generated_directory")
    parser.add_argument("--value-tolerance", type=float, default=1e-2)
    parser.add_argument("--mean-dt-relative-tolerance", type=float, default=0.05)
    parser.add_argument("--endpoint-tolerance", type=float, default=1e-8)
    args = parser.parse_args()

    if args.value_tolerance < 0:
        parser.error("--value-tolerance must be non-negative")
    if args.mean_dt_relative_tolerance < 0:
        parser.error("--mean-dt-relative-tolerance must be non-negative")
    if args.endpoint_tolerance < 0:
        parser.error("--endpoint-tolerance must be non-negative")

    try:
        return 0 if compare_directories(
            args.reference_directory,
            args.generated_directory,
            args.value_tolerance,
            args.mean_dt_relative_tolerance,
            args.endpoint_tolerance,
        ) else 1
    except (OSError, RuntimeError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
