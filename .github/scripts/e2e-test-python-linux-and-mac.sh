#!/bin/bash

## Example run command
# ./e2e-test-python-linux-and-mac.sh '../../examples/build/server' './e2e-test.py'

# Check for required arguments
if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <path_to_binary> <path_to_python_file>"
    exit 1
fi

BINARY_PATH=$1
PYTHON_FILE_EXECUTION_PATH=$2

rm /tmp/python-file-execution-res.log /tmp/server.log

# Random port to ensure it's not used
min=10000
max=11000
range=$((max - min + 1))
PORT=$((RANDOM % range + min))

# Install numpy for Python
export PYTHONHOME=$(pwd)/engines/cortex.python/python/
export LD_LIBRARY_PATH="$PYTHONHOME:$LD_LIBRARY_PATH"
export DYLD_FALLBACK_LIBRARY_PATH="$PYTHONHOME:$DYLD_FALLBACK_LIBRARY_PATH"
echo "Set Python HOME to $PYTHONHOME"
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
./engines/cortex.python/python/bin/python3 -m ensurepip
./engines/cortex.python/python/bin/python3 -m pip install --upgrade pip
./engines/cortex.python/python/bin/python3 -m pip install numpy --target=$PYTHONHOME/lib/python/site-packages/

# Start the binary file
"$BINARY_PATH" 1 127.0.0.1 $PORT >/tmp/server.log &

pid=$!

if ! ps -p $pid >/dev/null; then
    echo "server failed to start. Logs:"
    cat /tmp/server.log
    exit 1
fi

# Wait for a few seconds to let the server start
sleep 3

# Run the curl commands
response1=$(curl --connect-timeout 60 -o /tmp/python-file-execution-res.log -s -w "%{http_code}" --location "http://127.0.0.1:$PORT/v1/fine_tuning/job" \
    --header 'Content-Type: application/json' \
    --data '{
        "file_execution_path": "'$PYTHON_FILE_EXECUTION_PATH'"
    }')

error_occurred=0

# Verify the response
if [[ "$response1" -ne 200 ]]; then
    echo "The python file execution curl command failed with status code: $response1"
    cat /tmp/python-file-execution-res.log
    error_occurred=1
fi

# Verify the output of the Python file in output.txt
OUTPUT_FILE="./output.txt"
EXPECTED_OUTPUT="1 2 3"  # Replace with the expected content

if [[ -f "$OUTPUT_FILE" ]]; then
    actual_output=$(cat "$OUTPUT_FILE")
    if [[ "$actual_output" != "$EXPECTED_OUTPUT" ]]; then
        echo "The output of the Python file does not match the expected output."
        echo "Expected: $EXPECTED_OUTPUT"
        echo "Actual: $actual_output"
        error_occurred=1
    else
        echo "The output of the Python file matches the expected output."
    fi
else
    echo "Output file $OUTPUT_FILE does not exist."
    error_occurred=1
fi


if [[ "$error_occurred" -eq 1 ]]; then
    echo "Server test run failed!!!!!!!!!!!!!!!!!!!!!!"
    echo "Server Error Logs:"
    cat /tmp/server.log
    kill $pid
    echo "An error occurred while running the server."
    exit 1
fi

echo "----------------------"
echo "Log server:"
cat /tmp/server.log

echo "Server test run successfully!"

# Kill the server process
kill $pid