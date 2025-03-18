## Cortex Docker Setup

This guide will help you set up and run Cortex using Docker.

For more information on how to use Cortex, please refer to the [Cortex Documentation](https://cortex.so/docs/) and [Cortex Docker](https://cortex.so/docs/installation/docker).

### Prerequisites
- Docker / Docker Desktop
- nvidia-container-toolkit (for GPU support)

### Instructions

**Build Cortex Docker Image from source or Pull from Docker Hub**

- Pull Cortex Docker Image from Docker Hub

```bash
# Pull the latest image
docker pull menloltd/cortex:latest

# Pull a specific version
docker pull menloltd/cortex:nightly-1.0.1-224
```

- Build and Run Cortex Docker Container from Dockerfile

```bash
git clone https://github.com/menloresearch/cortex.cpp.git
cd cortex.cpp
git submodule update --init

# Default always uses the latest cortex.cpp and cortex.llamacpp
docker build -t menloltd/cortex --build-arg CORTEX_CPP_VERSION=$(git rev-parse HEAD) -f docker/Dockerfile .

# Use specific version of cortex.cpp and cortex.llamacpp
docker build --build-arg CORTEX_LLAMACPP_VERSION=0.1.34 --build-arg CORTEX_CPP_VERSION=$(git rev-parse HEAD) -t menloltd/cortex -f docker/Dockerfile .
```

**Run Cortex Docker Container**

```bash
# Create Volume to store models and data
docker volume create cortex_data

# GPU mode - nvidia-docker required, it will automatically use all available GPUs
docker run --gpus all -it -d --name cortex -v cortex_data:/root/cortexcpp -p 39281:39281 menloltd/cortex

# CPU mode
docker run -it -d --name cortex -v cortex_data:/root/cortexcpp -p 39281:39281 menloltd/cortex
```

**Check logs (Optional)**

```bash
docker logs cortex
```

**Access to http://localhost:39281 to check the cortex docs API.**

**Execute to container and try out cortex cli**

```bash
docker exec -it cortex bash
cortex --help
```
