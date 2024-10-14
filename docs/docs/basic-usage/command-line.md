---
title: Command Line Interface
description: Cortex CLI Overview.
slug: "command-line"
---

:::warning
🚧 Cortex.cpp is currently under development. Our documentation outlines the intended behavior of Cortex, which may not yet be fully implemented in the codebase.
:::

Cortex has a [Docker](https://docs.docker.com/engine/reference/commandline/cli/) and [Ollama](https://ollama.com/)-inspired [CLI syntax](/docs/cli) for running model operations. 

## How It Works
Cortex’s CLI invokes the Cortex Engine’s API, which runs in the background on port `39281`. 


## Basic Usage
### [Start Cortex Server](/docs/cli)
```bash
# By default the server will be started on port `39281`
cortex
```
### [Run Model](/docs/cli/run)
Cortex supports these [Built-in Models](/models)
```bash
# Pull and start a model
cortex run <model_id>
```
### [Chat with Model](/docs/cli/chat)
```bash
# chat with a model
cortex chat <model_id>
```
### [Show the Model State](/docs/cli/ps) 
```bash
# Show a model and cortex system status
cortex ps
```
### [Stop Model](/docs/cli/stop)
```bash
# Stop a model
cortex stop
```
### [Pull Model](/docs/cli/pull)
```bash
# Pull a model
cortex pull <model_id>
```
