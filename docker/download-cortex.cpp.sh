#!/bin/bash
set -x

VERSION=${1:-latest}

# Get the latest version of the cortex.llamacpp
if [ "$VERSION" = "latest" ]; then
  VERSION=$(curl -s https://api.github.com/repos/janhq/cortex.cpp/releases/latest | jq -r '.tag_name' | sed 's/v//');
fi

cd /tmp
wget https://github.com/janhq/cortex.cpp/releases/download/v$VERSION/cortex-$VERSION-linux-amd64.tar.gz
tar -xvf cortex-$VERSION-linux-amd64.tar.gz
mv ./cortex/cortex /usr/local/bin/cortex
chmod +x /usr/local/bin/cortex
rm -rf /tmp/*