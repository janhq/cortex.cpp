cmake -S ./cortex-cpp-deps -B ./build-deps/cortex-cpp-deps
make -C ./build-deps/cortex-cpp-deps -j 10
rm -rf ./build-deps/cortex-cpp-deps
