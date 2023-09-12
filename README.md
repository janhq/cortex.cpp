# Nitro - Accelerated AI Inference Engine

<p align="center">
  <img alt="nitrologo" src="https://user-images.githubusercontent.com/69952136/266939567-4a7d24f0-9338-4ab5-9261-cb3c71effe35.png">
</p>

<p align="center">
  <!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
  <img alt="GitHub commit activity" src="https://img.shields.io/github/commit-activity/m/janhq/nitro"/>
  <img alt="Github Last Commit" src="https://img.shields.io/github/last-commit/janhq/nitro"/>
  <img alt="Github Contributors" src="https://img.shields.io/github/contributors/janhq/nitro"/>
  <img alt="GitHub closed issues" src="https://img.shields.io/github/issues-closed/janhq/nitro"/>
  <img alt="Discord" src="https://img.shields.io/discord/1107178041848909847?label=discord"/>
</p>

<p align="center">
  <a href="https://docs.jan.ai/">Getting Started</a> - <a href="https://docs.jan.ai">Docs</a> 
  - <a href="https://docs.jan.ai/changelog/">Changelog</a> - <a href="https://github.com/janhq/nitro/issues">Bug reports</a> - <a href="https://discord.gg/AsJ8krTT3N">Discord</a>
</p>

> ⚠️ **Nitro is currently in Development**: Expect breaking changes and bugs!


## Features

### Supported features
- Simple http webserver to do inference on triton (without triton client)
- Upload inference result to s3 (txt2img)
## Available Endpoints

### Endpoints

```zsh
- /inferences/llm_models OPENAI_COMPATIBLE (STREAMING)
- /inferences/txt2img POST - JSON
- /inferences/img2img POST - MULTIPART
```

## Documentation

## Installation

### Using Docker (Recommended)

1. **Prerequisites**: Ensure you have a base Docker image with Triton Client installed.
    - Currently, only compatible with `nvcr.io/nvidia/tritonserver:23.06-py3-sdk`.

2. **Build Docker Image**: 
    ```zsh
    docker build . -t jan_infer
    ```

3. **Configuration**: 
    - Download and modify the example config file from [here](core/example.config.yaml).
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

## About Nitro

### Repo Structure
```zsh
.
|-- core
|   |-- inference_backend
|   |   |-- controllers
|   |   |   |-- img2img
|   |   |   |-- llm_models
|   |   |   `-- txt2img
|   |   |-- include
|   |   |-- schemas
|   |   `-- test
|   |-- models
|   `-- scripts
`-- docs
    |-- development
    `-- openapi


```

### Architecture
![Current architecture](docs/architecture.png)

### Contributing

Contributions are welcome! Please read the [CONTRIBUTING.md](CONTRIBUTING.md) file for guidelines on how to contribute to this project.

Please note that Jan intends to build a sustainable business that can provide high quality jobs to its contributors. If you are excited about our mission and vision, please contact us to explore opportunities. 

### Contact

- For support: please file a Github ticket
- For questions: join our Discord [here](https://discord.gg/FTk2MvZwJH)
- For long form inquiries: please email hello@jan.ai
