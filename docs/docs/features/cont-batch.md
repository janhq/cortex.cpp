---
title: Continuous Batching
description: Nitro's continuous batching combines multiple requests, enhancing throughput.
keywords: [Nitro, Jan, fast inference, inference server, local AI, large language model, OpenAI compatible, open source, llama]
---

Continuous batching boosts throughput and minimizes latency in large language model (LLM) inference. This technique groups multiple inference requests, significantly improving GPU utilization.

**Key Advantages:**

- Increased Throughput.
- Reduced Latency.
- Efficient GPU Use.

**Implementation Insight:**

To evaluate its effectiveness, compare continuous batching with traditional methods. For more details on benchmarking, refer to this [article](https://www.anyscale.com/blog/continuous-batching-llm-inference).

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