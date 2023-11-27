---
title: Continuous Batching
description: Nitro's continuous batching combines multiple requests, enhancing throughput.
---

## What is continous batching?

Continuous batching is a powerful technique that significantly boosts throughput in large language model (LLM) inference while minimizing latency. This process dynamically groups multiple inference requests, allowing for more efficient GPU utilization.

## Why Continuous Batching?

Traditional static batching methods can lead to underutilization of GPU resources, as they wait for all sequences in a batch to complete before moving on. Continuous batching overcomes this by allowing new sequences to start processing as soon as others finish, ensuring more consistent and efficient GPU usage.

## Benefits of Continuous Batching

- **Increased Throughput:** Improvement over traditional batching methods.
- **Reduced Latency:** Lower p50 latency, leading to faster response times.
- **Efficient Resource Utilization:** Maximizes GPU memory and computational capabilities.

## How to use continous batching
Nitro's `continuous batching` feature allows you to combine multiple requests for the same model execution, enhancing throughput and efficiency.

```bash title="Enable Batching" {6,7}
curl http://localhost:3928/inferences/llamacpp/loadmodel \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/path/to/your_model.gguf",
    "ctx_len": 512,
    "cont_batching": true,
    "n_parallel": 4,
  }'
```

For optimal performance, ensure that the `n_parallel` value is set to match the `thread_num`, as detailed in the [Multithreading](features/multi-thread.md) documentation.

### Benchmark and Compare

To understand the impact of continuous batching on your system, perform benchmarks comparing it with traditional batching methods. This [article](https://www.anyscale.com/blog/continuous-batching-llm-inference) will help you quantify improvements in throughput and latency.