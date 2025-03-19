# Use Debian stable slim as the base image
FROM debian:stable-slim

# Set working directory
WORKDIR /app

# Install curl, unzip, and numactl, download the file, unzip it, then remove unnecessary packages
RUN apt-get update && \
    apt-get install -y curl tar gzip numactl && \
    curl -L "https://github.com/menloresearch/nitro/releases/download/v0.1.17/nitro-0.1.17-linux-amd64.tar.gz" -o nitro.tar.gz && \
    tar -xzvf nitro.tar.gz && \
    rm nitro.tar.gz && \
    apt-get remove --purge -y curl tar gzip && \
    apt-get autoremove -y && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Expose the port
EXPOSE 3928

# Change the permissions of the nitro binary to make it executable
RUN chmod +x /app/nitro/nitro

# Set the command to run the nitro binary with numactl limiting to cores 0-7
ENTRYPOINT ["numactl", "--physcpubind=0-7", "/app/nitro/nitro"]
CMD ["1", "0.0.0.0", "3928"]

