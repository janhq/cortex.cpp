#!/bin/sh

npm install -g cortexso
# Run cortex
cortex  -a 0.0.0.0

cortex engines llamacpp init
cortex engines tensorrt-llm init

# Keep the container running by tailing the log file
tail -f /root/cortex/cortex.log
