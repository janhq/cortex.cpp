---
title: Load and Unload models
description: Nitro loads and unloads local AI models (local LLMs).
---

## Load model

The `loadmodel` in Nitro lets you load a local model into the server. It's an upgrade from `llama.cpp`, offering more features and customization options.

You can load the model using:

```bash title="Load Model" {1}
curl http://localhost:3928/inferences/llamacpp/loadmodel \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/path/to/your_model.gguf",
    "ctx_len": 512,
  }'
```

For more detail on the loading model, please refer to [Table of parameters].(#table-of-parameters).

### Enabling GPU Inference

To enable GPU inference in Nitro, a simple POST request is used. This request will instruct Nitro to load the specified model into the GPU, significantly boosting the inference throughput.

```bash title="GPU enable" {5}
curl http://localhost:3928/inferences/llamacpp/loadmodel \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/path/to/your_model.gguf",
    "ctx_len": 512,
    "ngl": 100,
  }'
```

You can adjust the `ngl` parameter based on your requirements and GPU capabilities.

## Unload model
To unload a model, you can use a similar `curl` command as loading the model, adjusting the endpoint to `/unloadmodel.`

```bash title="Unload the model" {1}
curl http://localhost:3928/inferences/llamacpp/unloadmodel
```

## Status
The `modelStatus` function provides the current status of the model, including whether it is loaded and its properties. This function offers improved monitoring capabilities compared to `llama.cpp`.

```bash title="Check Model Status" {1}
curl http://localhost:3928/inferences/llamacpp/modelstatus
```

If you load the model correctly, the response would be

```js title="Load Model Sucessfully"
{"message":"Model loaded successfully", "code": "ModelloadedSuccessfully"}
```

In case you got error while loading models. Please check for the correct model path.
```js title="Load Model Failed"
{"message":"No model loaded", "code": "NoModelLoaded"}
```

### Table of parameters

| Parameter        | Type    | Description                                                  |
|------------------|---------|--------------------------------------------------------------|
| `llama_model_path` | String  | The file path to the LLaMA model.                            |
| `ngl`              | Integer | The number of GPU layers to use.                             |
| `ctx_len`          | Integer | The context length for the model operations.                 |
| `embedding`        | Boolean | Whether to use embedding in the model.                       |
| `n_parallel`       | Integer | The number of parallel operations.|
|`cpu_threads`|Integer|The number of threads for CPU inference.|
| `cont_batching`    | Boolean | Whether to use continuous batching.                          |
| `user_prompt`      | String  | The prompt to use for the user.                              |
| `ai_prompt`        | String  | The prompt to use for the AI assistant.                      |
| `system_prompt`    | String  | The prompt for system rules.                          |
| `pre_prompt`    | String  | The prompt to use for internal configuration.                          |