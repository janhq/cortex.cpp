#!/bin/bash
DESTINATION_BINARY_NAME=cortex
USER_TO_RUN_AS=${SUDO_USER:-$(whoami)}
echo "Download cortex.llamacpp engines by default"
sudo -u $USER_TO_RUN_AS /usr/local/bin/$DESTINATION_BINARY_NAME engines install cortex.llamacpp
