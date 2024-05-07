# Makefile for Cortex llamacpp engine - Build, Lint, Test, and Clean
.PHONY: all build package run-e2e-test


CMAKE_EXTRA_FLAGS ?= ""
RUN_TESTS ?= false
LLM_MODEL_URL ?= "https://delta.jan.ai/tinyllama-1.1b-chat-v0.3.Q2_K.gguf"
EMBEDDING_MODEL_URL ?= "https://catalog.jan.ai/dist/models/embeds/nomic-embed-text-v1.5.f16.gguf"

# Default target, does nothing
all:
	@echo "Specify a target to run"

# Build the Cortex engine
build:
ifeq ($(OS),Windows_NT)
	@powershell -Command "cmake -S ./nitro_deps -B ./build_deps/nitro_deps;"
	@powershell -Command "cmake --build ./build_deps/nitro_deps --config Release -j4;"
	@powershell -Command "mkdir -p build; cd build; cmake .. $(CMAKE_EXTRA_FLAGS); cmake --build . --config Release -j4;"
else ifeq ($(shell uname -s),Linux)
	@./install_deps.sh;
	@mkdir -p build && cd build; \
	cmake .. $(CMAKE_EXTRA_FLAGS); \
	make -j4;
else
	@./install_deps.sh;
	@mkdir -p build && cd build; \
	cmake .. $(CMAKE_EXTRA_FLAGS); \
	make -j4;
endif

package:
ifeq ($(OS),Windows_NT)
	@powershell -Command "mkdir -p cortex-cpp\engines\cortex.llamacpp\; cp build\engines\cortex.llamacpp\engine.dll cortex\engines\cortex.llamacpp\;"
	@powershell -Command "cp build\Release\nitro.exe .\cortex-cpp\;"
	@powershell -Command "cp build_deps\_install\bin\zlib.dll .\cortex-cpp\;"
	@powershell -Command "cp ..\.github\patches\windows\msvcp140.dll .\cortex-cpp\;"
	@powershell -Command "cp ..\.github\patches\windows\vcruntime140_1.dll .\cortex-cpp\;"
	@powershell -Command "cp ..\.github\patches\windows\vcruntime140.dll .\cortex-cpp\;"
	@powershell -Command "7z a -ttar temp.tar cortex-cpp\\*; 7z a -tgzip cortex-cpp.tar.gz temp.tar;"
else ifeq ($(shell uname -s),Linux)
	@mkdir -p cortex-cpp/engines/cortex.llamacpp; \
	cp build/engines/cortex.llamacpp/libengine.so cortex-cpp/engines/cortex.llamacpp/; \
	cp build/cortex-cpp cortex-cpp/; \
	tar -czvf cortex-cpp.tar.gz cortex-cpp;
else
	@mkdir -p cortex-cpp/engines/cortex.llamacpp; \
	cp build/engines/cortex.llamacpp/libengine.dylib cortex-cpp/engines/cortex.llamacpp/; \
	cp build/cortex-cpp cortex-cpp/; \
	tar -czvf cortex-cpp.tar.gz cortex-cpp;
endif

run-e2e-test:
ifeq ($(RUN_TESTS),false)
	@echo "Skipping tests"
	@exit 0
endif
ifeq ($(OS),Windows_NT)
	@powershell -Command "cd cortex-cpp; ..\..\.github\scripts\e2e-test-llama-windows.bat cortex-cpp.exe $(LLM_MODEL_URL) $(EMBEDDING_MODEL_URL);"
else ifeq ($(shell uname -s),Linux)
	@cd cortex-cpp; \
	chmod +x ../../.github/scripts/e2e-test-llama-linux-and-mac.sh && ../../.github/scripts/e2e-test-llama-linux-and-mac.sh ./cortex-cpp $(LLM_MODEL_URL) $(EMBEDDING_MODEL_URL); \
	rm -rf uploads/;
else
	@cd cortex-cpp; \
	chmod +x ../../.github/scripts/e2e-test-llama-linux-and-mac.sh && ../../.github/scripts/e2e-test-llama-linux-and-mac.sh ./cortex-cpp $(LLM_MODEL_URL) $(EMBEDDING_MODEL_URL); \
	rm -rf uploads/;
endif