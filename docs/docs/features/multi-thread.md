---
title: Multithreading 
description: Nitro utilizes multithreading to optimize hardware usage.
keywords: [Nitro, Jan, fast inference, inference server, local AI, large language model, OpenAI compatible, open source, llama]
---

Multithreading in programming allows concurrent task execution, improving efficiency and responsiveness. It's key for optimizing hardware and application performance.

**Effective multithreading offers:**

- Faster Performance.
- Responsive IO.
- Deadlock Prevention.
- Resource Optimization.
- Asynchronous Programming Support.
- Scalability Enhancement.

For more information on threading, visit [Drogon's Documentation](https://github.com/drogonframework/drogon/wiki/ENG-FAQ-1-Understanding-drogon-threading-model).

## Enabling Multi-Threads on Nitro

To increase the number of threads used by Nitro, use the following command syntax:

```bash title="Nitro deploy server format"
nitro [thread_num] [host] [port] [uploads_folder_path]
```

- **thread_num:** Specifies the number of threads for the Nitro server.
- **host:** The host address normally `127.0.0.1` (localhost) or `0.0.0.0` (all interfaces).
- **port:** The port number where Nitro is to be deployed.
- **uploads_folder_path:** To set a custom path for file uploads in Drogon. Otherwise, it uses the current folder as the default location. 

To launch Nitro with 4 threads, enter this command in the terminal:
```bash title="Example"
nitro 4 127.0.0.1 5000
```

> After enabling multithreading, monitor your system's performance. Adjust the `thread_num` as needed to optimize throughput and latency based on your workload.