# Compilation, Build, and Execution

### 1. Build the Project

Bash

```
rm -rf build
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make install
```

### 2. Run the Program

Bash

```
./numsim ../test/0_driven_cavity/input_parameters.txt
```

### 3. Run the reference tests

The reference tests compare the generated VTI files with the data in
`test/0_driven_cavity/reference_output` and
`test/3_varying_params/reference_output`. Configure and build the project, then run:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Runs without a `uID` setting write to `build/out`. Each CTest case clears that
directory before running; the two scenarios run sequentially because they share it.
The comparator aligns frames by their recorded physical time, checks the mean time
step within 5%, and allows a maximum absolute pressure or velocity difference of
`1e-2`. The CI workflow downloads the reference archive from the `v1.0.0` release;
locally, keep the reference directories in the locations above.

### Troubleshooting

If you run into any errors during compilation or execution, feel free to ask an LLM for help—you might not need this, but for me, adding the Java library path to my shell configuration fixed it:

Bash

```
echo 'export LD_LIBRARY_PATH=/usr/lib/jvm/java-11-openjdk-amd64/lib:/usr/lib/jvm/java-11-openjdk-amd64/lib/server:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```
