# Use alpine latest as the base image
FROM alpine:latest as builder

# Build variables
ARG NITRO_VERSION=0.3.14

# Create build directory
WORKDIR /work

# Install build dependencies
RUN apk add --no-cache git cmake g++ make util-linux-dev zlib-dev

# Clone code
RUN git clone --recurse-submodules -j2 --depth 1 --branch v${NITRO_VERSION} --single-branch https://github.com/menloresearch/nitro.git

# Build
RUN cd nitro && \
    cmake -S nitro_deps/ -B ./build_deps/nitro_deps && \
    cmake --build ./build_deps/nitro_deps/ --config Release && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j $(nproc) && \
    chmod +x ./nitro

# Create the final image
FROM alpine:latest

# Install runtime dependencies from stripped down g++ and util-linux-dev (without dev packages)
RUN apk add --no-cache libstdc++ gmp isl25 mpc1 mpfr4 zlib libuuid

# Create working directory
WORKDIR /app

# Copy the binary from the builder image
COPY --from=builder /work/nitro/build/nitro /app/nitro

# Expose port
EXPOSE 3928

# Set the command to run nitro
ENTRYPOINT [ "/app/nitro" ]
CMD [ "1", "0.0.0.0", "3928" ]
