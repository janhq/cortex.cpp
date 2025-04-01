---
title: Cortex Models Pull
description: Cortex models subcommands.
---

:::warning
🚧 Cortex.cpp is currently under development. Our documentation outlines the intended behavior of Cortex, which may not yet be fully implemented in the codebase.
:::

# `cortex models pull`

This command downloads a model. You can use a HuggingFace `model_id` to download a model.



## Usage

```bash
cortex models pull <model_id>
```
### `model_id`
You can find the `model_id` for your desired model from:
- [Cortex Model Hub](https://huggingface.co/cortexso)
- [HuggingFace](https://huggingface.co/models)
- [Models](/models)
:::info
Currently Cortex only supports the following model format: **GGUF**, **ONNX**, and **TensorRT-LLM**.
:::
## Alias

The following alias is also available for downloading models:

- `cortex models download _`

## Options

| Option                    | Description                              | Required | Default value | Example                    |
|---------------------------|------------------------------------------|----------|---------------|----------------------------|
| `model_id`                | The identifier of the model you want to download. | Yes      | -             | `mistral`           |
| `-h`, `--help`              | Display help for command.                | No       | -             | `-h`                   |



