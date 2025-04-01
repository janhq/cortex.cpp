---
title: Cortex Models Update
description: Cortex models subcommands.
---

:::warning
🚧 Cortex.cpp is currently under development. Our documentation outlines the intended behavior of Cortex, which may not yet be fully implemented in the codebase.
:::

# `cortex models update`

This command updates a model configuration defined by a `model_id`.



## Usage

```bash
cortex models update [options] <model_id>
```
:::info
This command uses a `model_id` from the model that you have downloaded or available in your file system.
:::
## Options

| Option                      | Description                                                                                           | Required | Default value        | Example                                                   |
|-----------------------------|-------------------------------------------------------------------------------------------------------|----------|----------------------|-----------------------------------------------------------|
| `model_id`                  | The identifier of the model you want to update.                                                       | Yes      | -                    | `mistral`                                          |
| `-c`, `--options <options...>` | Specify the options to update the model. Syntax: `-c option1=value1 option2=value2`.  | Yes      | -                    | `-c max_tokens=100 temperature=0.5`                        |
| `-h`, `--help`              | Display help information for the command.                                                             | No       | -                    | `-h`                                                  |



