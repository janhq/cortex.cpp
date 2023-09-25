# How to install

Step 1:
Install dependencies static files
```zsh
./install_deps
```
This will create a build_deps folder, just ignore it

Step2:
Build the app from source
```zsh
mkdir build
cmake ..
make -j (your CPU core number here)
```

run ./nitro to test
