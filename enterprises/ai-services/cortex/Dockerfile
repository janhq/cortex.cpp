FROM nvidia/cuda:12.1.0-runtime-ubuntu22.04

WORKDIR /app

RUN apt-get update && apt-get install -y wget openmpi-bin libopenmpi-dev
RUN wget https://github.com/janhq/cortex.cpp/releases/download/v1.0.1/cortex-1.0.1-linux-amd64-local-installer.deb
RUN dpkg -i cortex-1.0.1-linux-amd64-local-installer.deb
RUN rm -rf cortex-1.0.1-linux-amd64-local-installer.deb
RUN cortex enigines install llama-cpp
RUN cortex pull ichigo:3b-gguf-q8-0
