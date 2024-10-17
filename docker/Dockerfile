FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    wget \
    jq \
    tar \
    openmpi-bin \
    libopenmpi-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ARG CORTEX_LLAMACPP_VERSION=latest
ARG CORTEX_CPP_VERSION=latest

COPY ./download-cortex.llamacpp.sh /tmp/download-cortex.llamacpp.sh

# Get the latest version of the Cortex Llama
RUN chmod +x /tmp/download-cortex.llamacpp.sh && /bin/bash /tmp/download-cortex.llamacpp.sh ${CORTEX_LLAMACPP_VERSION}

COPY ./download-cortex.cpp.sh /tmp/download-cortex.cpp.sh

# Get the latest version of the Cortex cpp
RUN chmod +x /tmp/download-cortex.cpp.sh && /bin/bash /tmp/download-cortex.cpp.sh ${CORTEX_CPP_VERSION}

# Copy the entrypoint script
COPY entrypoint.sh /usr/local/bin/entrypoint.sh

EXPOSE 39281

HEALTHCHECK --interval=300s --timeout=30s --start-period=10s --retries=3 \
    CMD curl -f http://127.0.0.1:39281/healthz || exit 1
  
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
