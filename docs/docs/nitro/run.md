---
title: Build simple Nitro chatbot
---

This guide provides instructions to create a chatbot powered by Nitro using the GGUF model.

## Step 1: Download the Model

First, you'll need to download the chatbot model.

1. **Navigate to the Models Folder**
   - Open your project directory.
   - Locate and open the `models` folder within the directory.

2. **Select a GGUF Model**
   - Visit the Hugging Face repository at [TheBloke's Models](https://huggingface.co/TheBloke).
   - Browse through the available models.
   - Choose the model that best fits your needs.

3. **Download the Model**
   - Once you've selected a model, download it using a command like the one below. Replace `<llama_model_path>` with the path of your chosen model.


```zsh title="Downloading Zephyr 7B Model"
wget https://huggingface.co/TheBloke/zephyr-7B-beta-GGUF/resolve/main/zephyr-7b-beta.Q5_K_M.gguf?download=true
```

## Step 2: Load model
Now, you'll set up the model in your application.

1. **Open `app.py` File**

    - In your project directory, find and open the app.py file.

2. **Configure the Model Path**

    - Modify the model path in app.py to point to your downloaded model.
    - Update the configuration parameters as necessary.

```zsh title="Example Configuration" {2}
dat = {
    "llama_model_path": "nitro/interface/models/zephyr-7b-beta.Q5_K_M.gguf",
    "ctx_len": 2048,
    "ngl": 100,
    "embedding": True,
    "n_parallel": 4,
    "pre_prompt": "A chat between a curious user and an artificial intelligence",
    "user_prompt": "USER: ",
    "ai_prompt": "ASSISTANT: "}
```

Congratulations! Your Nitro chatbot is now set up. Feel free to experiment with different configuration parameters to tailor the chatbot to your needs.

For more information on parameter settings and their effects, please refer to Run Nitro(using-nitro) for a comprehensive parameters table.