---
title: Cortex Models Remove
description: Cortex models subcommands.
---

:::warning
🚧 Cortex.cpp is currently under development. Our documentation outlines the intended behavior of Cortex, which may not yet be fully implemented in the codebase.
:::

# `cortex models remove`

This command deletes a local model defined by a `model_id`.



## Usage

```bash
cortex models remove <model_id>
```
:::info
This command uses a `model_id` from the model that you have downloaded or available in your file system.
:::
## Options
| Option                    | Description                                                                 | Required | Default value        | Example                |
|---------------------------|-----------------------------------------------------------------------------|----------|----------------------|------------------------|
| `model_id`                | The identifier of the model you want to remove.                             | Yes      | -                    | `mistral`       |
| `-h`, `--help`            | Display help for command.                                                   | No       | -                    | `-h`               |



