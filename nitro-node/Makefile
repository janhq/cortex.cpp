# Makefile for Nitro-node - Build and Clean

# Default target, build all
.PHONY: all

all: publish

# Installs npm dependencies
#install: build-core
install:
ifeq ($(OS),Windows_NT)
	npm config set fetch-timeout 300000
endif
	npm install

# Build
build: install
	npm run build

# Download Nitro
download-nitro: install
	npm run downloadnitro

test-ci: install
	npm run test:ci

# Note, this make target is just for testing on *NIX systems
test: install
	@test -e test/test_assets/*.gguf &&  echo "test/test_assets/*.gguf is already downloaded" || (mkdir -p test/test_assets && cd test/test_assets/ && curl -JLO "https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf")
	npm test

# Builds and pack
pack: build
	npm run build:publish

# Test that installation will also download nitro binaries
test-e2e-installation: pack
ifeq ($(OS),Windows_NT)
	$$env:NITRO_NODE_VERSION=(npm version --json | jq '.["@janhq/nitro-node"]' | foreach {$$_.replace('"','')}) $$env:NITRO_NODE_PKG=(Resolve-Path -Path "janhq-nitro-node$$NITRO_NODE_VERSION.tgz") node ..\.github\scripts\e2e-test-install-nitro-node.js
else
	NITRO_NODE_VERSION=$$(npm version --json | jq '.["@janhq/nitro-node"]' | tr -d '"') NITRO_NODE_PKG=$$(realpath "janhq-nitro-node-$${NITRO_NODE_VERSION}.tgz") node ../.github/scripts/e2e-test-install-nitro-node.js
endif

clean:
ifeq ($(OS),Windows_NT)
	powershell -Command "Get-ChildItem -Path . -Include *.tgz, package-lock.json -Recurse | Remove-Item -Recurse -Force"
	powershell -Command "Get-ChildItem -Path . -Include node_modules, bin, dist -Recurse -Directory | Remove-Item -Recurse -Force"
else
	rm -rf *.tgz package-lock.json bin dist
	find . -name "node_modules" -type d -prune -exec rm -rf '{}' +
endif
