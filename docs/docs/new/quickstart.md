---
title: Quickstart
---
## Step 1: Install Nitro

### For Linux and MacOS
Open your terminal and enter the following command. This will download and install Nitro on your system.
  ```bash
  curl -sfL https://raw.githubusercontent.com/janhq/nitro/main/install.sh -o /tmp/install.sh && chmod +x /tmp/install.sh && sudo bash /tmp/install.sh --gpu && rm /tmp/install.sh
  ```

### For Windows
Open PowerShell and execute the following command. This will perform the same actions as for Linux and MacOS but is tailored for Windows.
  ```bash
  powershell -Command "& { Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/janhq/nitro/main/install.bat' -OutFile 'install.bat'; .\install.bat --gpu; Remove-Item -Path 'install.bat' }"
  ```

> **NOTE:**Installing Nitro will add new files and configurations to your system to enable it to run.

For a manual installation process, see: [Install from Source](install.md)

## Step 2: Downloading a Model

Next, we need to download a model. For this example, we'll use the [Llama2 7B chat model](https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/tree/main).

- Create a `/model` and navigate into it:
```bash
mkdir model && cd model
wget -O llama-2-7b-model.gguf https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/resolve/main/llama-2-7b-chat.Q5_K_M.gguf?download=true
```

## Step 3: Run Nitro server

To start using Nitro, you need to run its server.

```bash title="Run Nitro server"
nitro
```

To check if the Nitro server is running:

```bash title="Nitro Health Status"
curl http://localhost:3928/healthz
```

## Step 4: Load model

To load the model to Nitro server, you need to

```bash title="Load model"
curl http://localhost:3928/inferences/llamacpp/loadmodel \
  -H 'Content-Type: application/json' \
  -d '{
    "llama_model_path": "/model/llama-2-7b-model.gguf",
    "ctx_len": 512,
    "ngl": 100,
  }'
```

## Step 5: Making an Inference

Finally, let's make an actual inference call using Nitro.

- In your terminal, execute:

```bash title="Nitro Inference"
curl http://localhost:3928/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "messages": [
      {
        "role": "user",
        "content": "Who won the world series in 2020?"
      },
    ]
  }'
```

This command sends a request to Nitro, asking it about the 2020 World Series winner.

- As you can see, A key benefit of Nitro is its alignment with [OpenAI's API structure](https://platform.openai.com/docs/guides/text-generation?lang=curl). Its inference call syntax closely mirrors that of OpenAI's API, facilitating an easier shift for those accustomed to OpenAI's framework.