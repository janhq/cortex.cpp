# Use alpine latest as the base image
FROM alpine:latest

# Build variables
ARG NITRO_VERSION=0.3.14

# Create build directory
WORKDIR /work

# Install build dependencies
RUN apk add --no-cache git cmake build-base util-linux-dev zlib-dev

# Clone code
RUN git clone --recurse-submodules -j2 --depth 1 --branch v${NITRO_VERSION} --single-branch https://github.com/janhq/nitro.git

# Build
RUN cd nitro && \
    cmake -S nitro_deps/ -B ./build_deps/nitro_deps && \
    cmake --build ./build_deps/nitro_deps/ --config Release && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j $(nproc)

# Install
WORKDIR /app
RUN cp /work/nitro/build/nitro /app/nitro && \
    chmod +x /app/nitro

# Cleanup
RUN apk del git cmake util-linux-dev zlib-dev && \
    rm -rf /work

# Expose port
EXPOSE 3928

# Set the command to run nitro
ENTRYPOINT [ "/app/nitro" ]
CMD [ "1", "0.0.0.0", "3928" ]
