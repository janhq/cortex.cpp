# How to install

### Step 1:

Install dependencies static files

```zsh
./install_deps.sh
```

This will create a build_deps folder, just ignore it

### Step 2:

Generate build file
- On MacOS with Apple silicon:

    ```zsh
    mkdir build && cd build
    cmake ..
    ```

- On MacOS with Intel processors:
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

### Step 3:

Run ./nitro to start the process.

To see if the build was successful, visit `localhost:8080/test`
