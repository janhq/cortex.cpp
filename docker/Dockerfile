# Please change the base image to the appropriate CUDA version base on NVIDIA Driver Compatibility
# Run nvidia-smi to check the CUDA version and the corresponding driver version
# Then update the base image to the appropriate CUDA version refer https://catalog.ngc.nvidia.com/orgs/nvidia/containers/cuda/tags

FROM nvidia/cuda:12.4.1-runtime-ubuntu22.04 AS base 

# 1. Install dependencies only when needed
FROM base AS devel

# Install g++ 11
RUN apt update && apt install -y gcc-11 g++-11 cpp-11 jq xsel curl gnupg make python3-dev && curl -sL https://deb.nodesource.com/setup_20.x | bash - && apt install nodejs -y && rm -rf /var/lib/apt/lists/*

# Update alternatives for GCC and related tools
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 110 \
                         --slave /usr/bin/g++ g++ /usr/bin/g++-11 \
                         --slave /usr/bin/gcov gcov /usr/bin/gcov-11 \
                         --slave /usr/bin/gcc-ar gcc-ar /usr/bin/gcc-ar-11 \
                         --slave /usr/bin/gcc-ranlib gcc-ranlib /usr/bin/gcc-ranlib-11 && \
    update-alternatives --install /usr/bin/cpp cpp /usr/bin/cpp-11 110

RUN npm install -g yarn

WORKDIR /app

FROM devel AS release

EXPOSE 1337

COPY ./common/entrypoint.sh /usr/local/bin/entrypoint.sh

RUN chmod +x /usr/local/bin/entrypoint.sh

HEALTHCHECK --interval=300s --timeout=30s --start-period=10s --retries=3 \
  CMD curl -f http://127.0.0.1:1337/api/system || exit 1

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
