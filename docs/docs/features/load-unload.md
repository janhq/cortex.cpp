---
title: Load and Unload models 
---

## Load model

The loadModel function in Nitro enables the loading of a model into the system. This function is an advanced feature over llama.cpp, providing enhanced capabilities and options for customization.

You can simply load the model using

```zsh
curl -X POST 'http://localhost:3928/inferences/llamacpp/loadmodel' \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/path/to/your_model.gguf",
  }'
```

For more detail in loading model, please refer to [First Inference Request](nitro/first-call)

:::info
OPTIONAL: You can run Nitro on a different port like 5000 instead of 3928 by running it manually in terminal

./nitro 1 127.0.0.1 5000 ([thread_num] [host] [port])
thread_num : the number of thread that nitro webserver needs to have
host : host value normally 127.0.0.1 or 0.0.0.0
port : the port that nitro got deployed onto
:::


## Unload model
To unload a model, you can use a similar `curl` command as loading the model, adjusting the endpoint to `/unloadmodel.`

```zsh
curl -X POST 'http://localhost:3928/inferences/llamacpp/unloadmodel' \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/path/to/your_model.gguf",
  }'
```

### Stop background process
:::danger TODO
updating
:::

## Status
The `modelStatus` function provides the current status of the model, including whether it is loaded and its properties. This function offers improved monitoring capabilities compared to `llama.cpp`.

```zsh
curl -X POST 'http://localhost:3928/inferences/llamacpp/modelstatus' \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/path/to/your_model.gguf",
  }'
```

