---
title: Cortex.cpp
description: Cortex.cpp Architecture
slug: "cortex-cpp"
---

:::warning
🚧 Cortex.cpp is currently under development. Our documentation outlines the intended behavior of Cortex, which may not yet be fully implemented in the codebase.
:::

Cortex.cpp is a Local AI engine that is used to run and customize LLMs. Cortex can be deployed as a standalone server, or integrated into apps like [Jan.ai](https://jan.ai/)

Cortex's roadmap is to eventually support full OpenAI API-equivalence.

It includes a Drogon server, with request queues, model orchestration logic, and hardware telemetry, and more, for prod environments.

This guide walks you through how Cortex.cpp is designed, the codebase structure, and future plans.

## Usage

See [Quickstart](/docs/quickstart)

## Interface

## Architecture

## Code Structure

```md
├── app/
│ │ ├── controllers/
│ │ ├── models/
│ │ ├── services/
│ │ ├── ?engines/
│ │ │ ├── llama.cpp
│ │ │ ├── tensorrt-llm
│ │ │ └── ...
│ │ └── ...
│ ├── CMakeLists.txt
│ ├── config.json
│ ├── Dockerfile
│ ├── docker-compose.yml
│ ├── README.md
│ └── ...
```

`cortex-cpp` folder contains stateless implementations, most of which call into `llamacpp` and `tensorrt-llm`, depending on the engine at runtime.

Here you will find the implementations for stateless endpoints:

- `/chat/completion`
- `/audio`
- `/fine_tuning`
- `/embeddings`
- `/load_model`
- `/unload_model`

And core hardware and model management logic like CPU instruction set detection, and multiple model loading logic.

## Runtime

## Roadmap
