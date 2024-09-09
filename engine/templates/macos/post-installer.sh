#!/bin/bash
DESTINATION_BINARY_NAME=cortex
echo "Download cortex.llamacpp engines by default"
USER_TO_RUN_AS=${SUDO_USER:-$(whoami)}
sudo -u "$USER_TO_RUN_AS" /usr/local/bin/$DESTINATION_BINARY_NAME engines cortex.llamacpp install
