## Cortex Docker Setup

This guide will help you set up and run Cortex using Docker.

### Prerequisites
- Docker / Docker Desktop
- nvidia-container-toolkit (for GPU support)

### Instructions
1. Clone the Cortex repository
    ```bash
    git clone https://github.com/janhq/cortex.cpp.git

    cd cortex.cpp/docker
    ```
2. Build the Docker image
    ```bash
    # Default always uses the latest cortex.cpp and cortex.llamacpp
    docker build -t cortex .

    # Use specific version of cortex.cpp and cortex.llamacpp
    docker build --build-arg CORTEX_LLAMACPP_VERSION=0.1.34 -t cortex .
    ```

3. Run the Docker container
    ```bash
    # Create Volume to store models and data
    docker volume create cortex_data

    # CPU mode
    docker run -it -d --name cortex -v cortex_data:/root/cortexcpp -p 39281:39281 cortex

    # GPU mode - nvidia-docker required, it will automatically use all available GPUs
    docker run --gpus all -it -d --name cortex -v cortex_data:/root/cortexcpp -p 39281:39281 cortex
    ```

4. Check logs (Optional)
    ```bash
    docker logs cortex
    ```

5. Access to http://localhost:39281 to check the cortex docs API.

6. Execute to container and try out cortex cli
    ```bash
    docker exec -it cortex bash
    cortex --help
    ```