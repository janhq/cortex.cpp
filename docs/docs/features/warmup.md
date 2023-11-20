---
title: Warming Up Model 
---

## What is Model Warming Up?

Model warming up is the process of running pre-requests through a model to optimize its components for production use. This step is crucial for reducing initialization and optimization delays during the first few inference requests.

## What are the Benefits?

Warming up an AI model offers several key benefits:

- **Enhanced Initial Performance:** Unlike in `llama.cpp`, where the first inference can be very slow, warming up reduces initial latency, ensuring quicker response times from the start.
- **Consistent Response Times:** Especially beneficial for systems updating models frequently, like those with real-time training, to avoid performance lags with new snapshots.

## How to Enable Model Warming Up?

On the Nitro server, model warming up is automatically enabled whenever a new model is loaded. This means that the server handles the warm-up process behind the scenes, ensuring that the model is ready for efficient and effective performance from the first inference request.
