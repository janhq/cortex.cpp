---
title: Cortex Models Start
description: Cortex models subcommands.
---

:::warning
🚧 Cortex.cpp is currently under development. Our documentation outlines the intended behavior of Cortex, which may not yet be fully implemented in the codebase.
:::

# `cortex models start`

This command starts a model defined by a `model_id`.


## Usage

```bash
# Start a model
cortex models start [model_id]

# Start with a specified engine
cortex models start [model_id]:[engine] [options]
```


:::info
- This command uses a `model_id` from the model that you have downloaded or available in your file system.
:::

## Options

| Option                    | Description                                              | Required | Default value                                | Example           |
|---------------------------|----------------------------------------------------------|----------|----------------------------------------------|-------------------|
| `model_id`                | The identifier of the model you want to start.           | No       | `Prompt to select from the available models` | `mistral`         |
| `--gpus`                  | List of GPUs to use.                                     | No       | -                                            | `[0,1]`           |
| `--ctx_len`               | Maximum context length for inference.                    | No       | `min(8192, max_model_context_length)`        | `1024`            |
| `-h`, `--help`            | Display help information for the command.                | No       | -                                            | `-h`              |



