# Makefile for Cortex llamacpp engine - Build, Lint, Test, and Clean
.PHONY: all build package

BUILD_DEPS_CMAKE_EXTRA_FLAGS ?= ""
CMAKE_EXTRA_FLAGS ?= ""
RUN_TESTS ?= false
LLM_MODEL_URL ?= "https://delta.jan.ai/tinyllama-1.1b-chat-v0.3.Q2_K.gguf"
EMBEDDING_MODEL_URL ?= "https://catalog.jan.ai/dist/models/embeds/nomic-embed-text-v1.5.f16.gguf"
CODE_SIGN ?= false
AZURE_KEY_VAULT_URI ?= xxxx
AZURE_CLIENT_ID ?= xxxx
AZURE_TENANT_ID ?= xxxx
AZURE_CLIENT_SECRET ?= xxxx
AZURE_CERT_NAME ?= xxxx
DEVELOPER_ID ?= xxxx

# Arguments for build installer
PACKAGE_NAME ?= cortexcpp
VERSION ?= 0.1.1
SOURCE_BINARY_PATH ?= ../../cortex/cortex
SOURCE_BINARY_SERVER_PATH ?= ../../cortex/cortex-server
DESTINATION_BINARY_NAME ?= cortex
DESTINATION_BINARY_SERVER_NAME ?= cortex-server
DATA_FOLDER_NAME ?= .cortex
CONFIGURATION_FILE_NAME ?= .cortexrc
UNINSTALLER_FILE_NAME ?= cortex-uninstall.sh
ARCH ?= amd64

# Default target, does nothing
all:
	@echo "Specify a target to run"

configure-vcpkg:
ifeq ($(OS),Windows_NT)
	@cd vcpkg && cmd /c bootstrap-vcpkg.bat;
	@cd vcpkg && cmd /c vcpkg install
else ifeq ($(shell uname -s),Linux) 
	@cd vcpkg && ./bootstrap-vcpkg.sh;
	@cd vcpkg && ./vcpkg install;
else
	@cd vcpkg && ./bootstrap-vcpkg.sh;
	@cd vcpkg && ./vcpkg install;
endif

# Build the Cortex engine
build:
ifeq ($(OS),Windows_NT)
	@powershell -Command "mkdir -p build; cd build; cmake .. $(CMAKE_EXTRA_FLAGS); cmake --build . --config Release -j4;"
else ifeq ($(shell uname -s),Linux)
	@mkdir -p build && cd build; \
	cmake .. $(CMAKE_EXTRA_FLAGS); \
	make -j4;
else
	@mkdir -p build && cd build; \
	cmake .. $(CMAKE_EXTRA_FLAGS); \
	make -j4;
endif

run-unit-tests:
ifeq ($(OS),Windows_NT)
	@powershell -Command "cd build\test\components\; .\test-components.exe;"
else ifeq ($(shell uname -s),Linux)
	@cd build/test/components/;\
	./test-components
else
	@cd build/test/components/;\
	./test-components
endif


pre-package:
ifeq ($(OS),Windows_NT)
	@powershell -Command "mkdir -p cortex;"
	@powershell -Command "cp build\cortex-server.exe .\cortex\$(DESTINATION_BINARY_SERVER_NAME).exe;"
	@powershell -Command "cp build\cortex.exe .\cortex\$(DESTINATION_BINARY_NAME).exe;"
	@powershell -Command "cp ..\engine\deps\windows\vulkan-1.dll .\cortex\;"
	@powershell -Command "cp ..\.github\patches\windows\msvcp140.dll .\cortex\;"
	@powershell -Command "cp ..\.github\patches\windows\vcruntime140_1.dll .\cortex\;"
	@powershell -Command "cp ..\.github\patches\windows\vcruntime140.dll .\cortex\;"
else ifeq ($(shell uname -s),Linux)
	@mkdir -p cortex; \
	cp build/cortex-server cortex/$(DESTINATION_BINARY_SERVER_NAME); \
	cp build/cortex cortex/$(DESTINATION_BINARY_NAME); \
	cp ../engine/deps/linux/libvulkan.so cortex/libvulkan.so;
else
	@mkdir -p cortex; \
	cp build/cortex-server cortex/$(DESTINATION_BINARY_SERVER_NAME); \
	cp build/cortex cortex/$(DESTINATION_BINARY_NAME);
endif

codesign-binary:
ifeq ($(CODE_SIGN),false)
	@echo "Skipping Code Sign"
	@exit 0
endif

ifeq ($(OS),Windows_NT)
	@powershell -Command "dotnet tool install --global AzureSignTool;"
	@powershell -Command '~\.dotnet\tools\azuresigntool.exe sign -kvu "$(AZURE_KEY_VAULT_URI)" -kvi "$(AZURE_CLIENT_ID)" -kvt "$(AZURE_TENANT_ID)" -kvs "$(AZURE_CLIENT_SECRET)" -kvc "$(AZURE_CERT_NAME)" -tr http://timestamp.globalsign.com/tsa/r6advanced1 -v ".\cortex\$(DESTINATION_BINARY_NAME).exe";'
	@powershell -Command '~\.dotnet\tools\azuresigntool.exe sign -kvu "$(AZURE_KEY_VAULT_URI)" -kvi "$(AZURE_CLIENT_ID)" -kvt "$(AZURE_TENANT_ID)" -kvs "$(AZURE_CLIENT_SECRET)" -kvc "$(AZURE_CERT_NAME)" -tr http://timestamp.globalsign.com/tsa/r6advanced1 -v ".\cortex\$(DESTINATION_BINARY_SERVER_NAME).exe";'
else ifeq ($(shell uname -s),Linux)
	@echo "Skipping Code Sign for linux"
	@exit 0
else
	codesign --force -s "$(DEVELOPER_ID)" --options=runtime --entitlements="./templates/macos/entitlements.plist" ./cortex/$(DESTINATION_BINARY_NAME);
	codesign --force -s "$(DEVELOPER_ID)" --options=runtime --entitlements="./templates/macos/entitlements.plist" ./cortex/$(DESTINATION_BINARY_SERVER_NAME);
endif

package:
ifeq ($(OS),Windows_NT)
	@powershell -Command "7z a -ttar temp.tar cortex\*; 7z a -tgzip cortex.tar.gz temp.tar;"
else ifeq ($(shell uname -s),Linux)
	tar -czvf cortex.tar.gz cortex;
else
	tar -czvf cortex.tar.gz cortex;
endif

build-installer:
ifeq ($(OS),Windows_NT)
	@echo "Building installer for Windows"; \
	cp cortex/* ../;
else ifeq ($(shell uname -s),Linux)
	@echo "Building installer for linux"; \
	cd templates/linux; \
	chmod +x create_deb.sh; \
	./create_deb.sh $(PACKAGE_NAME) $(VERSION) $(SOURCE_BINARY_PATH) $(SOURCE_BINARY_SERVER_PATH) $(DESTINATION_BINARY_NAME) $(DESTINATION_BINARY_SERVER_NAME) $(DATA_FOLDER_NAME) $(CONFIGURATION_FILE_NAME) $(ARCH); \
	cp $(PACKAGE_NAME).deb ../../
else
	@echo "Building installer for Macos"; \
	cd templates/macos; \
	chmod +x create_pkg.sh; \
	./create_pkg.sh $(PACKAGE_NAME) $(VERSION) $(SOURCE_BINARY_PATH) $(SOURCE_BINARY_SERVER_PATH) $(DESTINATION_BINARY_NAME) $(DESTINATION_BINARY_SERVER_NAME) $(DATA_FOLDER_NAME) $(CONFIGURATION_FILE_NAME) $(UNINSTALLER_FILE_NAME); \
	cp $(PACKAGE_NAME).pkg ../../
endif

clean:
ifeq ($(OS),Windows_NT)
	@powershell -Command "rm -rf build; rm -rf build-deps; rm -rf cortex; rm -rf cortex.tar.gz;"
else ifeq ($(shell uname -s),Linux)
	@rm -rf build; rm -rf build-deps; rm -rf cortex; rm -rf cortex.tar.gz;
else
	@rm -rf build; rm -rf build-deps; rm -rf cortex; rm -rf cortex.tar.gz;
endif