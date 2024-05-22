# Makefile for Cortex llamacpp engine - Build, Lint, Test, and Clean
.PHONY: all build package run-e2e-test


CMAKE_EXTRA_FLAGS ?= ""
RUN_TESTS ?= false
LLM_MODEL_URL ?= "https://delta.jan.ai/tinyllama-1.1b-chat-v0.3.Q2_K.gguf"
EMBEDDING_MODEL_URL ?= "https://catalog.jan.ai/dist/models/embeds/nomic-embed-text-v1.5.f16.gguf"
PYTHON_FILE_EXECUTION_PATH ?= "python-file-to-test.py"
CODE_SIGN ?= false
AZURE_KEY_VAULT_URI ?= xxxx
AZURE_CLIENT_ID ?= xxxx
AZURE_TENANT_ID ?= xxxx
AZURE_CLIENT_SECRET ?= xxxx
AZURE_CERT_NAME ?= xxxx
DEVELOPER_ID ?= xxxx

# Default target, does nothing
all:
	@echo "Specify a target to run"

# Build the Cortex engine
build:
ifeq ($(OS),Windows_NT)
	@powershell -Command "cmake -S ./cortex-cpp-deps -B ./build-deps/cortex-cpp-deps;"
	@powershell -Command "cmake --build ./build-deps/cortex-cpp-deps --config Release -j4;"
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

pre-package:
ifeq ($(OS),Windows_NT)
	@powershell -Command "mkdir -p cortex-cpp\engines\cortex.llamacpp\; cp -r build\engines\cortex.llamacpp\engine.dll cortex-cpp\engines\cortex.llamacpp\;"
	@powershell -Command "cp -r build\Release\cortex-cpp.exe .\cortex-cpp\;"
	@powershell -Command "cp -r build-deps\_install\bin\zlib.dll .\cortex-cpp\;"
	@powershell -Command "cp -r ..\.github\patches\windows\msvcp140.dll .\cortex-cpp\;"
	@powershell -Command "cp -r ..\.github\patches\windows\vcruntime140_1.dll .\cortex-cpp\;"
	@powershell -Command "cp -r ..\.github\patches\windows\vcruntime140.dll .\cortex-cpp\;"
else ifeq ($(shell uname -s),Linux)
	@mkdir -p cortex-cpp/engines/cortex.llamacpp; \
	cp build/engines/cortex.llamacpp/libengine.so cortex-cpp/engines/cortex.llamacpp/; \
	cp build/cortex-cpp cortex-cpp/;
else
	@mkdir -p cortex-cpp/engines/cortex.llamacpp; \
	cp build/engines/cortex.llamacpp/libengine.dylib cortex-cpp/engines/cortex.llamacpp/; \
	cp build/cortex-cpp cortex-cpp/;
endif

codesign:
ifeq ($(CODE_SIGN),false)
	@echo "Skipping Code Sign"
	@exit 0
endif

ifeq ($(OS),Windows_NT)
	@powershell -Command "dotnet tool install --global AzureSignTool;"
	@powershell -Command 'azuresigntool.exe sign -kvu "$(AZURE_KEY_VAULT_URI)" -kvi "$(AZURE_CLIENT_ID)" -kvt "$(AZURE_TENANT_ID)" -kvs "$(AZURE_CLIENT_SECRET)" -kvc "$(AZURE_CERT_NAME)" -tr http://timestamp.globalsign.com/tsa/r6advanced1 -v ".\cortex-cpp\cortex-cpp.exe";'
	@powershell -Command 'azuresigntool.exe sign -kvu "$(AZURE_KEY_VAULT_URI)" -kvi "$(AZURE_CLIENT_ID)" -kvt "$(AZURE_TENANT_ID)" -kvs "$(AZURE_CLIENT_SECRET)" -kvc "$(AZURE_CERT_NAME)" -tr http://timestamp.globalsign.com/tsa/r6advanced1 -v ".\cortex-cpp\engines\cortex.llamacpp\engine.dll";'
else ifeq ($(shell uname -s),Linux)
	@echo "Skipping Code Sign for linux"
	@exit 0
else
	find "cortex-cpp" -type f -exec codesign --force -s "$(DEVELOPER_ID)" --options=runtime {} \;
endif

package:
ifeq ($(OS),Windows_NT)
	@powershell -Command "7z a -ttar temp.tar cortex-cpp\*; 7z a -tgzip cortex-cpp.tar.gz temp.tar;"
else ifeq ($(shell uname -s),Linux)
	tar -czvf cortex-cpp.tar.gz cortex-cpp;
else
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

run-python-e2e-test:
ifeq ($(RUN_TESTS),false)
	@echo "Skipping tests"
	@exit 0
endif
ifeq ($(OS),Windows_NT)
	@powershell -Command "cp -r build\engines\cortex.python cortex-cpp\engines\;"
	@powershell -Command "cd cortex-cpp;..\..\.github\scripts\e2e-test-python-windows.bat cortex-cpp.exe ..\..\.github\scripts\$(PYTHON_FILE_EXECUTION_PATH);"
	@powershell -Command "rm -r .\engines\cortex.python;"
else ifeq ($(shell uname -s),Linux)
	@cp -rf build/engines/cortex.python cortex-cpp/engines/; \
	cd cortex-cpp; \
	chmod +x ../../.github/scripts/e2e-test-python-linux-and-mac.sh && ../../.github/scripts/e2e-test-python-linux-and-mac.sh ./cortex-cpp ../../.github/scripts/$(PYTHON_FILE_EXECUTION_PATH); \
	rm -rf uploads/; \
	rm -rf ./engines/cortex.python;
else
	@cp -rf build/engines/cortex.python cortex-cpp/engines/; \
	cd cortex-cpp; \
	chmod +x ../../.github/scripts/e2e-test-python-linux-and-mac.sh && ../../.github/scripts/e2e-test-python-linux-and-mac.sh ./cortex-cpp ../../.github/scripts/$(PYTHON_FILE_EXECUTION_PATH); \
	rm -rf uploads/; \
	rm -rf ./engines/cortex.python;
endif

clean:
ifeq ($(OS),Windows_NT)
	@powershell -Command "rm -rf build; rm -rf build-deps; rm -rf cortex-cpp; rm -rf cortex-cpp.tar.gz;"
else ifeq ($(shell uname -s),Linux)
	@rm -rf build; rm -rf build-deps; rm -rf cortex-cpp; rm -rf cortex-cpp.tar.gz;
else
	@rm -rf build; rm -rf build-deps; rm -rf cortex-cpp; rm -rf cortex-cpp.tar.gz;
endif