# How to install

### Step 1:

Install dependencies static files
- On MacOS with Apple silicon
    ```zsh
    ./install_deps.sh
    ```
- On Windows
    ```
    cmake -S ./nitro_deps -B ./build_deps/nitro_deps
    cmake --build ./build_deps/nitro_deps --config Release
    ```
This will create a build_deps folder, just ignore it

### Step 2:

Generate build file
- On MacOS, Linux, and Windows

    ```zsh
    mkdir build && cd build
    cmake ..
    ```

- On MacOS with Intel processors
    ```zsh
    mkdir build && cd build
    cmake -DLLAMA_METAL=OFF .. 
    ```

- On Linux with CUDA:
    ```zsh
    mkdir build && cd build
    cmake -DLLAMA_CUBLAS=ON .. 
    ```


Build the app
- On MacOS and Linux
    ```
    # MacOS
    make -j $(sysctl -n hw.physicalcpu)
    # Linux
    make -j $(%NUMBER_OF_PROCESSORS%)
    ```

- On Windows
    ```
    cmake --build . --config Release
    ```

### Step 3:
- On MacOS and Linux run ./nitro to start the process.
- On Windows:
    ```
    cd Release
    # add zlib dynamic lib
    copy ..\..\build_deps\_install\bin\zlib.dll .
    nitro.exe
    ```

To see if the build was successful, visit `localhost:8080/test`
