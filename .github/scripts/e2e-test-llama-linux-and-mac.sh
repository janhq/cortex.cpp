#!/bin/bash

## Example run command
# ./linux-and-mac.sh './jan/plugins/@janhq/inference-plugin/dist/nitro/nitro_mac_arm64' https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v0.3-GGUF/resolve/main/tinyllama-1.1b-chat-v0.3.Q2_K.gguf

# Check for required arguments
if [[ $# -ne 3 ]]; then
    echo "Usage: $0 <path_to_binary> <url_to_download_llm> <url_to_download_embedding>"
    exit 1
fi

rm /tmp/load-llm-model-res.log /tmp/completion-res.log /tmp/unload-model-res.log /tmp/load-embedding-model-res.log /tmp/embedding-res.log /tmp/nitro.log

BINARY_PATH=$1
DOWNLOAD_LLM_URL=$2
DOWNLOAD_EMBEDDING_URL=$3

# Random port to ensure it's not used
min=10000
max=11000
range=$((max - min + 1))
PORT=$((RANDOM % range + min))

# Start the binary file
"$BINARY_PATH" 1 127.0.0.1 $PORT >/tmp/nitro.log &

# Get the process id of the binary file
pid=$!

if ! ps -p $pid >/dev/null; then
    echo "nitro failed to start. Logs:"
    cat /tmp/nitro.log
    exit 1
fi

# Wait for a few seconds to let the server start
sleep 5

# Check if /tmp/testllm exists, if not, download it
if [[ ! -f "/tmp/testllm" ]]; then
    curl --connect-timeout 300 $DOWNLOAD_LLM_URL --output /tmp/testllm
fi

# Check if /tmp/test-embedding exists, if not, download it
if [[ ! -f "/tmp/test-embedding" ]]; then
    curl --connect-timeout 300 $DOWNLOAD_EMBEDDING_URL --output /tmp/test-embedding
fi

# Run the curl commands
response1=$(curl --connect-timeout 60 -o /tmp/load-llm-model-res.log -s -w "%{http_code}" --location "http://127.0.0.1:$PORT/inferences/llamacpp/loadModel" \
    --header 'Content-Type: application/json' \
    --data '{
    "llama_model_path": "/tmp/testllm",
    "ctx_len": 50,
    "ngl": 32,
    "embedding": false
}')

if ! ps -p $pid >/dev/null; then
    echo "nitro failed to load model. Logs:"
    cat /tmp/nitro.log
    exit 1
fi

response2=$(
    curl --connect-timeout 60 -o /tmp/completion-res.log -s -w "%{http_code}" --location "http://127.0.0.1:$PORT/v1/chat/completions" \
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
        "max_tokens": 50,
        "stop": ["hello"],
        "frequency_penalty": 0,
        "presence_penalty": 0,
        "temperature": 0.1
     }'
)

# unload model
response3=$(curl --connect-timeout 60 -o /tmp/unload-model-res.log --request GET -s -w "%{http_code}" --location "http://127.0.0.1:$PORT/inferences/llamacpp/unloadModel" \
    --header 'Content-Type: application/json' \
    --data '{
    "llama_model_path": "/tmp/testllm"
}')

# load embedding model
response4=$(curl --connect-timeout 60 -o /tmp/load-embedding-model-res.log -s -w "%{http_code}" --location "http://127.0.0.1:$PORT/inferences/llamacpp/loadModel" \
    --header 'Content-Type: application/json' \
    --data '{
    "llama_model_path": "/tmp/test-embedding",
    "ctx_len": 50,
    "ngl": 32,
    "embedding": true,
    "model_type": "embedding"
}')

# request embedding
response5=$(
    curl --connect-timeout 60 -o /tmp/embedding-res.log -s -w "%{http_code}" --location "http://127.0.0.1:$PORT/v1/embeddings" \
        --header 'Content-Type: application/json' \
        --header 'Accept: text/event-stream' \
        --header 'Access-Control-Allow-Origin: *' \
        --data '{
          "input": "Hello",
          "model": "test-embedding",
          "encoding_format": "float"       
     }'
)

error_occurred=0
if [[ "$response1" -ne 200 ]]; then
    echo "The load llm model curl command failed with status code: $response1"
    cat /tmp/load-llm-model-res.log
    error_occurred=1
fi

if [[ "$response2" -ne 200 ]]; then
    echo "The completion curl command failed with status code: $response2"
    cat /tmp/completion-res.log
    error_occurred=1
fi

if [[ "$response3" -ne 200 ]]; then
    echo "The unload model curl command failed with status code: $response3"
    cat /tmp/unload-model-res.log
    error_occurred=1
fi

if [[ "$response4" -ne 200 ]]; then
    echo "The load embedding model curl command failed with status code: $response4"
    cat /tmp/load-embedding-model-res.log
    error_occurred=1
fi

if [[ "$response5" -ne 200 ]]; then
    echo "The embedding curl command failed with status code: $response5"
    cat /tmp/embedding-res.log
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
cat /tmp/load-llm-model-res.log

echo "----------------------"
echo "Log run test:"
cat /tmp/completion-res.log

echo "----------------------"
echo "Log run test:"
cat /tmp/unload-model-res.log

echo "----------------------"
echo "Log run test:"
cat /tmp/load-embedding-model-res.log

echo "----------------------"
echo "Log run test:"
cat /tmp/embedding-res.log

echo "Nitro test run successfully!"

# Kill the server process
kill $pid
