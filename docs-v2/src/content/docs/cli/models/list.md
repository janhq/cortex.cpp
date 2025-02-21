---
title: Cortex Models List
description: Cortex models subcommands.
---

:::warning
🚧 Cortex.cpp is currently under development. Our documentation outlines the intended behavior of Cortex, which may not yet be fully implemented in the codebase.
:::

# `cortex models list`

This command lists all local and remote models.



## Usage

```bash
cortex models list [options]
```
For example, it returns the following:
```bash
+---------+----------------+----------------+-----------------+---------+
| (Index) |       ID       |   model alias  |      engine     | version |
+---------+----------------+----------------+-----------------+---------+
|    1    | llama3:gguf    | llama3:gguf    | llama-cpp       |    1    |
+---------+----------------+----------------+-----------------+---------+
|    2    | tinyllama:gguf | tinyllama:gguf | llama-cpp       |    1    |
+---------+----------------+----------------+-----------------+---------+

```

## Options

| Option                    | Description                                        | Required | Default value | Example              |
|---------------------------|----------------------------------------------------|----------|---------------|----------------------|
| `-f`, `--format <format>` | Specify output format for the models list.         | No       | `json`        | `-f json`       |
| `-h`, `--help`            | Display help for command.                          | No       | -             | `-h`             |


