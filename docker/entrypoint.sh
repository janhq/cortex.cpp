#!/bin/sh

# Install cortex.llamacpp engine

echo "apiServerHost: 0.0.0.0" > /root/.cortexrc
echo "enableCors: true" >> /root/.cortexrc

cortex engines install llama-cpp -s /opt/cortex.llamacpp

# Start the cortex server

cortex start

# Keep the container running by tailing the log files
tail -f /root/cortexcpp/logs/cortex.log &
tail -f /root/cortexcpp/logs/cortex-cli.log &
wait