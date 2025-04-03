# Stage 1: Base dependencies (common stage)
FROM ubuntu:22.04 as common

ENV DEBIAN_FRONTEND=noninteractive

# Install common dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    software-properties-common \
    curl \
    wget \
    jq \
    tar \
    openmpi-bin \
    libopenmpi-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Stage 2: Build dependencies and compilation
FROM common as build

# Install Dependencies
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null && \
    apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    make \
    git \
    uuid-dev \
    lsb-release \
    gpg \
    zip \
    unzip \
    gcc \
    g++ \
    ninja-build \
    pkg-config \
    python3-pip \
    openssl \
    libssl-dev && \
    pip3 install awscli && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Download and install CMake 3.22.6
RUN wget https://github.com/Kitware/CMake/releases/download/v3.22.6/cmake-3.22.6.tar.gz -q -O /tmp/cmake.tar.gz && \
    tar -xzf /tmp/cmake.tar.gz -C /tmp && \
    cd /tmp/cmake-3.22.6 && \
    ./bootstrap && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/cmake.tar.gz /tmp/cmake-3.22.6

ARG CORTEX_CPP_VERSION=latest
ARG CMAKE_EXTRA_FLAGS=""

WORKDIR /app

# Copy source code
COPY ./engine /app/engine
COPY ./docs/static/openapi/cortex.json /app/docs/static/openapi/cortex.json

# Build project
# Configure vcpkg binary sources
RUN cd engine && make configure-vcpkg && make build CMAKE_EXTRA_FLAGS="-DCORTEX_CPP_VERSION=${CORTEX_CPP_VERSION} -DCMAKE_BUILD_TEST=OFF -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake ${CMAKE_EXTRA_FLAGS}"

# Stage 3: Runtime
FROM common as runtime
WORKDIR /app
COPY --from=build /app/engine/build/cortex /usr/local/bin/cortex
COPY --from=build /app/engine/build/cortex-server /usr/local/bin/cortex-server

COPY ./docker/download-cortex.llamacpp.sh /tmp/download-cortex.llamacpp.sh
COPY ./docker/entrypoint.sh /usr/local/bin/entrypoint.sh

# Get the latest version of Cortex Llama
ARG CORTEX_LLAMACPP_VERSION=latest
RUN chmod +x /tmp/download-cortex.llamacpp.sh && /bin/bash /tmp/download-cortex.llamacpp.sh ${CORTEX_LLAMACPP_VERSION}

# Configure entrypoint
RUN chmod +x /usr/local/bin/entrypoint.sh

EXPOSE 39281

# Healthcheck
HEALTHCHECK --interval=300s --timeout=30s --start-period=10s --retries=3 \
    CMD curl -f http://127.0.0.1:39281/healthz || exit 1

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
