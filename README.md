# Compilation, Build, and Execution

### 1. Build the Project

Bash

```
rm -rf build
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make install
```

### 2. Run scenario0 (driven cavity) and compare outputs to reference

Bash

```
./numsim ../test/0_driven_cavity/input_parameters.txt

python3 test/tools/compare_vti_outputs.py build/out test/0_driven_cavity/reference_output
python3 test/tools/compare_txt_outputs.py build/out test/0_driven_cavity/reference_output
```

### Troubleshooting

If you run into any errors during compilation or execution, feel free to ask an LLM for help—you might not need this, but for me, adding the Java library path to my shell configuration fixed it:

Bash

```
echo 'export LD_LIBRARY_PATH=/usr/lib/jvm/java-11-openjdk-amd64/lib:/usr/lib/jvm/java-11-openjdk-amd64/lib/server:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

