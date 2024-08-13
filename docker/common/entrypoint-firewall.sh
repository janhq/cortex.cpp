#!/bin/sh

# Setup DNS resolution with dnsmasq
echo "nameserver 127.0.0.1" > /etc/resolv.conf
dnsmasq -k &

# Generate Nginx configuration from routes.txt
/usr/local/bin/generate_nginx_conf.sh

# Install cortex
npm install -g cortexso

# Start cortex
cortex -a 127.0.0.1

cortex engines llamacpp init
cortex engines tensorrt-llm init

# Start nginx
nginx -g 'daemon off;' &

# Keep the container running by tailing the log file
tail -f /root/cortex/cortex.log
