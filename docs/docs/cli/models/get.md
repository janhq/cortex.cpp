---
title: Cortex Models Get
description: Cortex models subcommands.
---

:::warning
🚧 Cortex.cpp is currently under development. Our documentation outlines the intended behavior of Cortex, which may not yet be fully implemented in the codebase.
:::

# `cortex models get`

This command returns a model detail defined by a `model_id`.



## Usage

```bash
cortex models get <model_id>
```
For example, it returns the following:

```bash
{
  name: 'tinyllama',
  model: 'tinyllama',
  version: 1,
  files: [ 'C:\\Users\\ACER\\cortex\\models\\tinyllama\\model.gguf' ],
  stop: [ '</s>' ],
  top_p: 0.95,
  temperature: 0.7,
  frequency_penalty: 0,
  presence_penalty: 0,
  max_tokens: 4096,
  stream: true,
  ngl: 33,
  ctx_len: 4096,
  engine: 'llamacpp',
  prompt_template: '<|system|>\n{system_message}<|user|>\n{prompt}<|assistant|>',
  id: 'tinyllama',
  created: 1720659351720,
  object: 'model',
  owned_by: ''
}
```
:::info
This command uses a `model_id` from the model that you have downloaded or available in your file system.
:::

## Options

| Option            | Description                                           | Required | Default value | Example         |
|-------------------|-------------------------------------------------------|----------|---------------|-----------------|
| `model_id`        | The identifier of the model you want to retrieve.     | Yes      | -             | `mistral`|
| `-h`, `--help`    | Display help information for the command.             | No       | -             | `-h`        |

