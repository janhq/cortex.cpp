---
title: About Nitro
slug: /docs
description: Efficient LLM inference engine for edge computing
keywords: [Nitro, Jan, fast inference, inference server, local AI, large language model, OpenAI compatible, open source, llama]
---

Nitro is a high-efficiency C++ inference engine for edge computing, powering [Jan](https://jan.ai/). It is lightweight and embeddable, ideal for product integration.

Learn more on [GitHub](https://github.com/janhq/nitro).

## Why Nitro?

- **Fast Inference:** Built on top of the cutting-edge inference library `llama.cpp`, modified to be production ready.
- **Lightweight:** Only 3MB, ideal for resource-sensitive environments.
- **Easily Embeddable:** Simple integration into existing applications, offering flexibility.
- **Quick Setup:** Approximately 10-second initialization.
- **Enhanced Web Framework:** Incorporates `drogon cpp` to boost web service efficiency.

### OpenAI-compatible API

Nitro's compatibility with OpenAI's API structure is a notable advantage. Its command format for inference calls closely mirrors that of OpenAI, facilitating an easy transition for users.

For instance, compare the Nitro inference call:

<div class="code-snippet-left">

```bash title="Nitro chat completion"
curl http://localhost:3928/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "model": "gpt-3.5-turbo",
    "messages": [
      {
        "role": "system",
        "content": "You are a helpful assistant."
      },
      {
        "role": "user",
        "content": "Who won the world series in 2020?"
      },
    ]
  }'

```

</div>

<div class="code-snippet-right">

```bash title="OpenAI API chat completion"
curl https://api.openai.com/v1/chat/completions \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $OPENAI_API_KEY" \
  -d '{
    "model": "gpt-3.5-turbo",
    "messages": [
      {
        "role": "system",
        "content": "You are a helpful assistant."
      },
      {
        "role": "user",
        "content": "Who won the world series in 2020?"
      },
    ]
  }'
```

</div>

- **Extends OpenAI's API with helpful model methods:**
  - [Unload model](features/load-unload#unload-model)
  - [Checking model status](features/load-unload/#status)

### Cross-Platform

- **Operating Systems**: Nitro Supports Windows, Linux, and MacOS.
- **Hardware Compatibility**:
  - CPUs: ARM, x86.
  - GPUs: Nvidia, AMD.
- **Detailed Resources**: [Windows Installation Guide](install/#windows), [Linux and MacOS Installation Guide](install/#linux-and-macos).

### Multi-modal Capabilities

- **Coming Soon**: Expansion to multi-modal functionalities - enabling Nitro to process and generate images, and audio.
- **Features to Expect**:
  - Large Language-and-Vision Assistant.
  - Speech recognition and transcription.

## Architecture

- **Overview**: Nitro's architecture is designed for scalability and efficiency, utilizing a modular framework that supports diverse AI functionalities.
- **Detailed Specifications**: For an in-depth understanding of Nitro's internal workings, components, and design philosophy, refer to our [Architecture Specifications](architecture.md).

## Support

### GitHub Issue Tracking

- **Report Problems**: Encounter an issue with Nitro? File a [GitHub issue](https://github.com/janhq/nitro). Please include detailed error logs and steps to reproduce the problem.

### Discord Community

- **Join the Conversation**: Discuss Nitro development and seek peer support in our [#nitro-dev](https://discord.gg/FTk2MvZwJH) channel on Discord.

## Contributing

### How to Contribute

Nitro welcomes contributions in various forms, not just coding. Here are some ways you can get involved:

- **Understand Nitro**: Start with the [Getting Started](/new/quickstart) guide. Found an issue or have a suggestion? [Open an issue](https://github.com/janhq/nitro/issues) to let us know.

- **Feature Development**: Engage with community feature requests. Bring ideas to life by opening a [pull request](https://github.com/janhq/nitro/pulls) for features that interest you.

### Links

- [Nitro GitHub Repository](https://github.com/janhq/nitro)

## Acknowledgements

- [drogon](https://github.com/drogonframework/drogon): The fast C++ web framework
- [llama.cpp](https://github.com/ggerganov/llama.cpp): Inference of LLaMA model in pure C/C++
