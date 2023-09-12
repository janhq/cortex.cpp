FROM debian:bookworm-slim AS clang16_image

# Install dependencies and LLVM
RUN apt-get update -qq \
 && apt-get install -qqy --no-install-recommends \
    gnupg2 wget ca-certificates apt-transport-https \
    autoconf automake cmake dpkg-dev file make patch libc6-dev \
    git curl libcurl4-openssl-dev \
    libopencv-dev libyaml-cpp-dev \
    postgresql-server-dev-all libmariadb-dev libsqlite3-dev libhiredis-dev \
    openssl libssl-dev libjsoncpp-dev uuid-dev zlib1g-dev libc-ares-dev \
 && echo "deb https://apt.llvm.org/bookworm llvm-toolchain-bookworm-16 main" > /etc/apt/sources.list.d/llvm.list \
 && wget -qO /etc/apt/trusted.gpg.d/llvm.asc https://apt.llvm.org/llvm-snapshot.gpg.key \
 && apt-get update -qq \
 && apt-get install -qqy -t llvm-toolchain-bookworm-16 clang-16 clang-tidy-16 lld-16 libc++-16-dev libc++abi-16-dev \
 && for f in /usr/lib/llvm-16/bin/*; do ln -sf "$f" /usr/bin; done \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

# Set clang as the default compiler
RUN ln -sf /usr/bin/clang /usr/bin/cc \
 && ln -sf /usr/bin/clang++ /usr/bin/c++ \
 && cc --version \
 && c++ --version

WORKDIR /workspace

# Install Drogon
RUN git clone https://github.com/drogonframework/drogon /workspace/drogon \
 && cd /workspace/drogon \
 && ./build.sh \
 && rm -rf /workspace/drogon

# Install AWS SDK
RUN git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp /workspace/aws-sdk-cpp \
 && cd /workspace/aws-sdk-cpp \
 && git checkout tags/1.11.160 \
 && mkdir build && cd build \
 && cmake .. -DBUILD_ONLY=s3 -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_SHARED_LIBS=ON -DAUTORUN_UNIT_TESTS=OFF \
 && make -j $(nproc) \
 && make install \
 && rm -rf /workspace/aws-sdk-cpp

# Install valijson
RUN git clone --recurse-submodules --depth=1 https://github.com/tristanpenman/valijson.git /workspace/valijson \
 && mkdir /workspace/valijson/build \
 && cd /workspace/valijson/build \
 && cmake .. \
 && cmake --install . \
 && rm -rf /workspace/valijson

# Install Triton client
RUN git clone https://github.com/triton-inference-server/client /workspace/client \
 && mkdir /workspace/client/build \
 && cd /workspace/client/build \
 && cmake -DCMAKE_INSTALL_PREFIX=`pwd`/install -DTRITON_ENABLE_CC_GRPC=ON .. \
 && make -j $(nproc) cc-clients \
 && mv /workspace/client/build/install /workspace/install \
 && rm -rf /workspace/client

# Install sentence piece
RUN apt update \
 && apt install -y cmake pkg-config libgoogle-perftools-dev git \
 && rm -rf /var/lib/apt/lists/* \
 && git clone https://github.com/google/sentencepiece.git /workspace/sentencepiece \
 && mkdir /workspace/sentencepiece/build \
 && cd /workspace/sentencepiece/build \
 && cmake .. \
 && make -j $(nproc) \
 && make install \
 && ldconfig -v \
 && rm -rf /workspace/sentencepiece

COPY . /workspace/workdir

WORKDIR /workspace/workdir/inference_backend/build
RUN cmake -DBUILD_SINGLE_BINARY=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. && make -j $(nproc)

ENTRYPOINT [ "/workspace/workdir/inference_backend/build/bin/janinfer_backend_single_binary" ]
