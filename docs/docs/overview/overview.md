---
title: Introduction 
---

## What is Nitro?
Nitro is a fast, lightweight, and embeddable inference engine, powering [Jan](https://jan.ai/). Developed in C++, it's specially optimized for use in edge computing and is ready for deployment in products.

⚡ Discover more about Nitro on [GitHub](https://github.com/janhq/nitro)

## Why Choose Nitro?

- **Fast Inference:**

Nitro uses the `drogon` C++17/20 HTTP application framework, which makes a significant difference. This framework is known for its speed, ensuring that Nitro processes data swiftly. This means your applications can make quick decisions based on complex data, a crucial factor in today's fast-paced digital environment.

- **Lightweight:**

At a mere 3MB, Nitro is a testament to efficiency. This stark difference in size makes Nitro an ideal choice for applications.

- **Easily Embeddable:**
Nitro is designed to blend seamlessly into your application without restricting the use of other tools. This flexibility is a crucial advantage.

### Feature-Rich

Nitro doesn't just match the capabilities of llama.cpp; it takes them a step further. It has features essential for modern applications:
- [Batching Inference](features/batch)
- [Unload model](features/load-unload)

- **Quick Setup:**
Nitro can be up and running in about 10 seconds. This rapid deployment means you can focus more on development and less on installation processes.

- **Broad Platform Support:**

Nitro is compatible with a wide range of platforms.

## Advancements Over llama.cpp
- **Enhanced Web Framework:**

Nitro elevates its game with drogon cpp, a C++ production-ready web framework. Its non-blocking socket IO ensures that your web services are efficient, robust, and reliable.
- **Refined Control Over llama.cpp Features:**

With Nitro, you gain more control over `llama.cpp` features. You can now stop background slot processing and unload models as needed. This level of control optimizes resource usage and enhances application performance.

## Acknowledgements:
- [llama.cpp](https://github.com/ggerganov/llama.cpp): Inference of LLaMA model in pure C/C++
- [drogon](https://github.com/drogonframework/drogon): The fast C++ web framework supporting either C++17 or C++14
