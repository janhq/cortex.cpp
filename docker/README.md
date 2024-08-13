# Docker with cortex

We offer two methods for deploying the Cortex environment on Docker.

## Method 1: Use the default Dockerfile with Cortex.

To use this method, you need to follow these steps:
```bash
git clone https://github.com/janhq/cortex.git
cd cortex/docker
docker build -t cortex:latest .

# Run the container with GPU support
docker run -it --gpus all -d -p 1337:1337 cortex:latest

# Run the container with CPU support
docker run -it -d -p 1337:1337 cortex:latest

# After starting, you can access Swagger at http://localhost:1337/api and the API server at http://localhost:1337.
# Additionally, you can exec into the container and use cortex-cli to perform other operations.
```

## Method 2: Use Dockerfile.firewall with the feature to block outbound connections by domain and block inbound connections by API path.

The use case for this method is when you want to host the Cortex API 100% offline, preventing access to remote models like the OpenAI API. Alternatively, you might want to block inbound connections by restricting clients from calling the API to load models `/v1/models/start`.

To use this method, you need to follow these steps:

- Step 1: Edit the contents of the [blocked-domains.txt](./docker/common/blocked-domains.txt) file according to your requirements. Refer to the provided examples in the file. The goal is to block outbound connections to the domains you do not want to allow.
- Step 2: Edit the contents of the [blocked-paths.txt](./docker/common/blocked-paths.txt) file according to your requirements. Refer to the provided examples in the file. The goal is to block inbound connections to the paths you do not want to allow.
- Step 3: Build the image with Dockerfile.firewall following the instructions below:

    ```bash
    git clone https://github.com/janhq/cortex.git
    cd cortex/docker
    docker build -f Dockerfile.firewall -t cortex-with-firewall:latest .

    # Run the container with GPU support
    docker run -it --gpus all -d -p 1337:1337 cortex:latest

    # Run the container with CPU support
    docker run -it -d -p 1337:1337 cortex:latest

    # After starting, you can access Swagger at http://localhost:1337/api and the API server at http://localhost:1337.
    # Additionally, you can exec into the container and use cortex-cli to perform other operations.
    ```
