#!/bin/sh

# Install cortex.llamacpp engine

mkdir -p /root/.config/cortexcpp
echo "apiServerHost: 0.0.0.0" > /root/.config/cortexcpp/.cortexrc
echo "enableCors: true" >> /root/.config/cortexcpp/.cortexrc

# Start the cortex server
cortex start

# Install the engine
cortex engines install llama-cpp -s /opt/cortex.llamacpp

cortex engines list


# Keep the container running by tailing the log files
tail -f /root/.local/share/cortexcpp/logs/cortex.log &
tail -f /root/.local/share/cortexcpp/logs/cortex-cli.log &
wait