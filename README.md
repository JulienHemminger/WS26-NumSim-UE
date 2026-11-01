# Compilation, Build, and Execution

### 1. Build the Project

Bash

```
cd ../Exercise1/Submission/
rm -rf build
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make install
```

### 2. Run the Program

Bash

```
./numsim ../test/0_lid_driven_cavity/lid_driven_cavity.txt
```

### Troubleshooting

If you run into any errors during compilation or execution, feel free to ask an LLM for help—you might not need this, but for me, adding the Java library path to my shell configuration fixed it:

Bash

```
echo 'export LD_LIBRARY_PATH=/usr/lib/jvm/java-11-openjdk-amd64/lib:/usr/lib/jvm/java-11-openjdk-amd64/lib/server:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

