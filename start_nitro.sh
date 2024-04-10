#!/bin/bash
# Determine the directory where this script is located.
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
# Set DYLD_LIBRARY_PATH to include the 'lib' directory inside the script's directory.
export DYLD_LIBRARY_PATH="$DIR/lib:$DYLD_LIBRARY_PATH"
# Execute the binary, assuming it's located in the same directory as this script.
# Replace 'your_binary' with the actual name of your binary.
exec "$DIR/nitro" "$@"
