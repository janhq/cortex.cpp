---
title: Multithreading 
description: Nitro utilizes multithreading to optimize hardware usage.
---

## What is Multithreading?

Multithreading is a programming concept where a process executes multiple threads simultaneously, improving efficiency and performance. It allows concurrent execution of tasks, such as data processing or user interface updates. This technique is crucial for optimizing hardware usage and enhancing application responsiveness.

## Drogon's Threading Model

Nitro powered by Drogon, a high-speed C++ web application framework, utilizes a thread pool where each thread possesses its own event loop. These event loops are central to Drogon's functionality:

- **Main Loop**: Runs on the main thread, responsible for starting worker loops.
- **Worker Loops**: Handle tasks and network events, ensuring efficient task execution without blocking.

## Why it's important

Understanding and effectively using multithreading in Drogon is crucial for several reasons:

1. **Optimized Performance**:  Multithreading enhances application efficiency by enabling simultaneous task execution for faster response times.

2. **Non-blocking IO Operations**: Utilizing multiple threads prevents long-running tasks from blocking the entire application, ensuring high responsiveness.

3. **Deadlock Avoidance**: Event loops and threads helps prevent deadlocks, ensuring smoother and uninterrupted application operation.

4. **Effective Resource Utilization**: Distributing tasks across multiple threads leads to more efficient use of server resources, improving overall performance.

5. **Async Programming**

6. **Scalability**

## Enabling More Threads on Nitro

To increase the number of threads used by Nitro, use the following command syntax:

```js
nitro [thread_num] [host] [port]
```

- **thread_num:** Specifies the number of threads for the Nitro server.
- **host:** The host address normally `127.0.0.1` (localhost) or `0.0.0.0` (all interfaces).
- **port:** The port number where Nitro is to be deployed.

To launch Nitro with 4 threads, enter this command in the terminal:
```js
nitro 4 127.0.0.1 5000
```

> After enabling multithreading, monitor your system's performance. Adjust the `thread_num` as needed to optimize throughput and latency based on your workload.

## Acknowledgements
For more information on Drogon's threading, visit [Drogon's Documentation](https://github.com/drogonframework/drogon/wiki/ENG-FAQ-1-Understanding-drogon-threading-model).