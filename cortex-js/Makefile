CODE_SIGN ?= false
AZURE_KEY_VAULT_URI ?= xxxx
AZURE_CLIENT_ID ?= xxxx
AZURE_TENANT_ID ?= xxxx
AZURE_CLIENT_SECRET ?= xxxx
AZURE_CERT_NAME ?= xxxx
DEVELOPER_ID ?= xxxx
CORTEX_EXE_IN ?= "dist/cortexso-win.exe"
CORTEX_EXE_OUT ?= "cortex.exe"
CORTEX_VERSION ?= "0.0.0.1"

update-app-info:
ifeq ($(OS),Windows_NT)
	@powershell -Command 'npx resedit --in $(CORTEX_EXE_IN) --out $(CORTEX_EXE_OUT) --icon "1,cortex.ico" --no-grow --company-name "Homebrew Computer Pte Ltd" --file-description "cortex cli" --file-version "$(CORTEX_VERSION)" --internal-name "cortex" --product-name "cortex" --product-version "$(CORTEX_VERSION)"'
else ifeq ($(shell uname -s),Linux)
	@cp ./dist/cortexso-linux ./cortex
endif

codesign-binary:
ifeq ($(CODE_SIGN),false)
	@echo "Skipping Code Sign"
	@exit 0
endif

ifeq ($(OS),Windows_NT)
	@powershell -Command "dotnet tool install --global AzureSignTool;"
	@powershell -Command 'azuresigntool.exe sign -kvu "$(AZURE_KEY_VAULT_URI)" -kvi "$(AZURE_CLIENT_ID)" -kvt "$(AZURE_TENANT_ID)" -kvs "$(AZURE_CLIENT_SECRET)" -kvc "$(AZURE_CERT_NAME)" -tr http://timestamp.globalsign.com/tsa/r6advanced1 -v ".\cortex.exe";'
else ifeq ($(shell uname -s),Linux)
	@echo "Skipping Code Sign for linux"
	@exit 0
else
	codesign --force -s "$(DEVELOPER_ID)" --options=runtime --entitlements="./entitlements.plist" ./cortex;
endif

codesign-installer:
ifeq ($(CODE_SIGN),false)
	@echo "Skipping Code Sign"
	@exit 0
endif

ifeq ($(OS),Windows_NT)
	@powershell -Command "dotnet tool install --global AzureSignTool;"
	@powershell -Command 'azuresigntool.exe sign -kvu "$(AZURE_KEY_VAULT_URI)" -kvi "$(AZURE_CLIENT_ID)" -kvt "$(AZURE_TENANT_ID)" -kvs "$(AZURE_CLIENT_SECRET)" -kvc "$(AZURE_CERT_NAME)" -tr http://timestamp.globalsign.com/tsa/r6advanced1 -v ".\setup.exe";'
else ifeq ($(shell uname -s),Linux)
	@echo "Skipping Code Sign for linux"
	@exit 0
else
	productsign --sign "Developer ID Installer: $(DEVELOPER_ID)" cortex-installer.pkg cortex-installer-signed.pkg;
	rm cortex-installer.pkg;
	mv cortex-installer-signed.pkg cortex-installer.pkg;
endif

postbundle:
ifeq ($(RUN_TESTS),false)
	@echo "Skipping tests"
	@exit 0
endif
ifeq ($(OS),Windows_NT)
	@powershell -Command "7z a -ttar temp.tar cortex.exe; 7z a -tgzip cortex.tar.gz temp.tar;"
	@powershell -Command "7z a -ttar temp2.tar setup.exe; 7z a -tgzip cortex-installer.tar.gz temp2.tar;"
else ifeq ($(shell uname -s),Linux)
	@chmod +x cortex; \
	tar -czvf cortex.tar.gz cortex;
else
	@chmod +x cortex; \
	tar -czvf cortex.tar.gz cortex; \
	tar -czvf cortex-installer.tar.gz cortex-installer.pkg;
endif