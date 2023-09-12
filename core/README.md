# Inference Backend

This repository is intended to be used with the [accelerated_ai](https://github.com/janhq/accelerated_ai) repository. It serves as an HTTP server that abstracts away Triton for ease of use. Written in C++, it is inherently very performant and uses minimal resources.

## Directory Structure

```
.
├── docs (Documentation)
├── inference_backend (Inference Backend)
│   ├── controllers (Endpoints Controllers)
│   │   ├── llm_models
│   │   └── txt2img
│   ├── include (Inference Logic & Utils)
│   ├── schemas (JSON Schemas for Request Validation)
│   └── test
└── models
```


### Notes:
- **inference_backend**: A Drogon C++ web server using the janinfer library for inference and user service.
- **controllers**: Controllers in Drogon C++, each handling the logic for a deployed model from the [accelerated_ai](https://github.com/janhq/accelerated_ai) repository.

Disclaimer: This is currently an experimental project!

## Available Endpoints

### Endpoints

```zsh
- /inferences/llm_models OPENAI_COMPATIBLE (STREAMING)
- /inferences/txt2img POST - JSON
```

### Supported features
- Simple http webserver to do inference on triton (without triton client)
- Upload inference result to s3 (txt2img)

## Getting Started

### Using Docker (Recommended)

1. **Prerequisites**: Ensure you have a base Docker image with Triton Client installed.
    - Currently, only compatible with `nvcr.io/nvidia/tritonserver:23.06-py3-sdk`.

2. **Build Docker Image**: 
    ```zsh
    docker build . -t jan_infer
    ```

3. **Configuration**: 
    - Download and modify the example config file from [here](example.config.yaml).
    - Make sure to rename it by removing "example." from the filename.

    ```yaml
    custom_config:
      s3_public_endpoint:  <your s3 endpoint>
      triton_endpoint: <your triton ip:port>
      s3_bucket: <your s3 bucket name>
      drogon_port: <backend deployment port>
    ```

4. **Run Docker Container**: 
    - Replace the placeholders with your specific configurations.
  
    ```zsh
    docker run \
      -v /path/to/your/config.yaml:/workspace/workdir/janinfer_backend/config.yaml \
      -p 3000:3000 \
      -e AWS_ACCESS_KEY_ID=<your_access_key> \
      -e AWS_SECRET_ACCESS_KEY=<your_secret_key> \
      -e AWS_DEFAULT_REGION=<your_region> \
      jan_infer
    ```

Note: **/path/to/your/config.yaml** -> This is the config file that you need to make in step 3, you can place it anywhere as long as mount it properly like above.

That's it! You should now have the inference backend up and running.

ation about how some parts of the backend is implemented can be found at [Developer Documentation](core/docs/development)
