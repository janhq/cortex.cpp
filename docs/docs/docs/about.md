---
title: About Nitro
slug: /docs
---

Nitro is a fast, lightweight (3mb) inference server that can be embedded in apps to run local AI. Nitro can be used to run a variety of popular open source AI models, and provides an OpenAI-compatible API. 

Nitro is used to power [Jan](https://jan.ai), a open source alternative to OpenAI's platform that can be run on your own computer or server. 


Nitro is a fast, lightweight, and embeddable inference engine, powering [Jan](https://jan.ai/). Developed in C++, it's specially optimized for use in edge computing and is ready for deployment in products.

⚡ Discover more about Nitro on [GitHub](https://github.com/janhq/nitro)

## Why Nitro?

### Lightweight & Fast

- Old materials
  - At a mere 3MB, Nitro is a testament to efficiency. This stark difference in size makes Nitro an ideal choice for applications.
  - Nitro is designed to blend seamlessly into your application without restricting the use of other tools. This flexibility is a crucial advantage.
- **Quick Setup:**
Nitro can be up and running in about 10 seconds. This rapid deployment means you can focus more on development and less on installation processes.

- Old material
  - Nitro uses the `drogon` C++17/20 HTTP application framework, which makes a significant difference. This framework is known for its speed, ensuring that Nitro processes data swiftly. This means your applications can make quick decisions based on complex data, a crucial factor in today's fast-paced digital environment.
  - Nitro elevates its game with drogon cpp, a C++ production-ready web framework. Its non-blocking socket IO ensures that your web services are efficient, robust, and reliable.
  - [Batching Inference](features/batch)
  - Non-blocking Socket IO

### OpenAI-compatible API

- [ ] OpenAI-compatible
- [ ] Given examples
- [ ] What is not covered? (e.g. Assistants, Tools -> See Jan)

- Extends OpenAI's API with helpful model methods
- e.g. Load/Unload model
- e.g. Checking model status
- [Unload model](features/load-unload)
- With Nitro, you gain more control over `llama.cpp` features. You can now stop background slot processing and unload models as needed. This level of control optimizes resource usage and enhances application performance.

### Cross-Platform

- [ ] Cross-platform

### Multi-modal

- [ ] Hint at what's coming

## Architecture

 - [ ] Link to Specifications

## Support

- [ ] File a Github Issue
- [ ] Go to Discord

## Contributing

- [ ] Link to Github

## Acknowledgements

- [drogon](https://github.com/drogonframework/drogon): The fast C++ web framework supporting either C++17 or C++14
- [llama.cpp](https://github.com/ggerganov/llama.cpp): Inference of LLaMA model in pure C/C++

