# Makefile for Nitro-node - Build and Clean

# Default target, build all
.PHONY: all

all: publish

# Installs yarn dependencies
#install: build-core
install:
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

test-ci: install
	yarn test

# Note, this make target is just for testing on *NIX systems
test: install
	@test -e test/test_assets/*.gguf &&  echo "test/test_assets/*.gguf is already downloaded" || (mkdir -p test/test_assets && cd test/test_assets/ && curl -JLO "https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf")
	yarn test

# Builds and publishes the extension
publish: build
	yarn run build:publish

clean:
ifeq ($(OS),Windows_NT)
	powershell -Command "Remove-Item -Recurse -Force -Path *.tgz, .yarn, yarn.lock, package-lock.json, bin"
	powershell -Command "Get-ChildItem -Path . -Include node_modules, dist -Recurse -Directory | Remove-Item -Recurse -Force"
else
	rm -rf *.tgz .yarn yarn.lock package-lock.json bin
	find . -name "node_modules" -type d -prune -exec rm -rf '{}' +
	find . -name "dist" -type d -exec rm -rf '{}' +
endif
