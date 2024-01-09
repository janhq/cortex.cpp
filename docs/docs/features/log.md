---
title: Logging
description: Enabling logging in Nitro.
keywords: [Nitro, log, Jan, fast inference, inference server, local AI, large language model, OpenAI compatible, open source, llama]
---


## Enabling Logging

Nitro's logging feature can be activated by specifying a log folder. This is crucial for monitoring and troubleshooting.

## Setting Up Logging

To configure logging, you need to specify the path to the log folder. Use the following command to set it up:

```bash title="Config logging" {5}
curl http://localhost:3928/inferences/llamacpp/loadmodel \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/path/to/your_model.gguf",
    "llama_log_folder": "/path/to/log/folder/"
  }'
```

> **Note:** Ensure the log folder exists before running this command. If the specified folder does not exist, logs will default to your current directory.