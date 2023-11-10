#!/bin/bash

## Example run command
# ./linux-and-mac.sh './jan/plugins/@janhq/inference-plugin/dist/nitro/nitro_mac_arm64' https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v0.3-GGUF/resolve/main/tinyllama-1.1b-chat-v0.3.Q2_K.gguf

# Check for required arguments
if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <path_to_binary> <url_to_download>"
    exit 1
fi

rm /tmp/response1.log /tmp/response2.log /tmp/nitro.log

BINARY_PATH=$1
DOWNLOAD_URL=$2

# Start the binary file
"$BINARY_PATH" > /tmp/nitro.log 2>&1 &

# Get the process id of the binary file
pid=$!

if ! ps -p $pid > /dev/null; then
    echo "nitro failed to start. Logs:"
    cat /tmp/nitro.log
    exit 1
fi

# Wait for a few seconds to let the server start
sleep 5



# Check if /tmp/testmodel exists, if not, download it
if [[ ! -f "/tmp/testmodel" ]]; then
    wget $DOWNLOAD_URL -O /tmp/testmodel
fi

# Run the curl commands
response1=$(curl -o /tmp/response1.log -s -w "%{http_code}" --location 'http://localhost:3928/inferences/llamacpp/loadModel' \
--header 'Content-Type: application/json' \
--data '{
    "llama_model_path": "/tmp/testmodel",
    "ctx_len": 2048,
    "ngl": 32,
    "embedding": false
}' 2>&1)

response2=$(curl -o /tmp/response2.log -s -w "%{http_code}" --location 'http://localhost:3928/inferences/llamacpp/chat_completion' \
--header 'Content-Type: application/json' \
--header 'Accept: text/event-stream' \
--header 'Access-Control-Allow-Origin: *' \
--data '{
        "messages": [
            {"content": "Hello there", "role": "assistant"},
            {"content": "Write a long and sad story for me", "role": "user"}
        ],
        "stream": true,
        "model": "gpt-3.5-turbo",
        "max_tokens": 100,
        "stop": ["hello"],
        "frequency_penalty": 0,
        "presence_penalty": 0,
        "temperature": 0.7
     }' 2>&1
)

error_occurred=0
if [[ "$response1" -ne 200 ]]; then
    echo "The first curl command failed with status code: $response1"
    cat /tmp/response1.log
    error_occurred=1
fi

if [[ "$response2" -ne 200 ]]; then
    echo "The second curl command failed with status code: $response2"
    cat /tmp/response2.log
    error_occurred=1
fi

if [[ "$error_occurred" -eq 1 ]]; then
    echo "Nitro test run failed!!!!!!!!!!!!!!!!!!!!!!"
    echo "Nitro Error Logs:"
    cat /tmp/nitro.log
    kill $pid
    exit 1
fi

echo "----------------------"
echo "Log load model:"
cat /tmp/response1.log

echo "----------------------"
echo "Log run test:"
cat /tmp/response2.log


echo "Nitro test run successfully!"

# Kill the server process
kill $pid
