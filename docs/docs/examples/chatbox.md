---
title: Nitro with Chatbox
---

This guide demonstrates how to integrate Nitro with Chatbox, showcasing the compatibility of Nitro with various platforms.

## What is Chatbox?
Chatbox is a versatile desktop client that supports multiple cutting-edge Large Language Models (LLMs). It is available for Windows, Mac, and Linux operating systems. 

For more information, please visit the [Chatbox official GitHub page](https://github.com/Bin-Huang/chatbox).


## Downloading and Installing Chatbox

To download and install Chatbox, follow the instructions available at this [link](https://github.com/Bin-Huang/chatbox#download).

## Using Nitro as a Backend

1. Start Nitro server

Open your command line tool and enter:
```
nitro
```

> Ensure you are using the latest version of [Nitro](new/install.md)

2. Run the Model

To load the model, use the following command:

```
curl http://localhost:3928/inferences/llamacpp/loadmodel \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "model/llama-2-7b-chat.Q5_K_M.gguf",
    "ctx_len": 512,
    "ngl": 100,
  }'
```

3. Config chatbox
Adjust the `settings` in Chatbox to connect with Nitro. Change your settings to match the configuration shown in the image below:

![Settings](img/chatbox.PNG)

4. Chat with the Model

Once the setup is complete, you can start chatting with the model using Chatbox. All functions of Chatbox are now enabled with Nitro as the backend.

## Video demo