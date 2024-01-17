# Makefile for Nitro-node - Build and Clean

# Default target, build all
.PHONY: all

all: publish

# Build jan/core
build-core:
	git submodule update --init
ifeq ($(OS),Windows_NT)
	type NUL > jan/yarn.lock
else
	touch jan/yarn.lock
endif
	cd jan && yarn install
	cd jan/core && yarn build

# Installs yarn dependencies
install: build-core
ifeq ($(OS),Windows_NT)
	yarn config set network-timeout 300000
endif
	yarn install

# Build
build: install
	yarn run build

# Download Nitro
download-nitro: install
	yarn run downloadnitro

# Builds and publishes the extension
publish: build download-nitro
	yarn run publish

clean:
ifeq ($(OS),Windows_NT)
	del /S *.tgz
	powershell -Command "Get-ChildItem -Path . -Include node_modules, dist -Recurse -Directory | Remove-Item -Recurse -Force"
else
	rm -f *.tgz
	find . -name "node_modules" -type d -prune -exec rm -rf '{}' +
	find . -name "dist" -type d -exec rm -rf '{}' +
endif
