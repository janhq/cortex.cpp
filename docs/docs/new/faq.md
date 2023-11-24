---
title: FAQs
slug: /faq
---

<details>
  <summary>1. Is Nitro the same as Llama.cpp with an API server?</summary>

Yes, that's correct. However, Nitro isn't limited to just Llama.cpp; it will soon integrate multiple other models like Whisper, Bark, and Stable Diffusion, all in a single binary. This eliminates the need for you to develop a separate API server on top of AI models. Nitro is a comprehensive solution, designed for ease of use and efficiency.

</details>

<details>
  <summary>2. Is Nitro simply Llama-cpp-python?</summary>

Indeed, Nitro isn't bound to Python, which allows you to leverage high-performance software that fully utilizes your system's capabilities. With Nitro, learning how to deploy a Python web server or use FastAPI isn't necessary. The Nitro web server is already fully optimized.

</details>

<details>
  <summary>3. Why should I switch to Nitro over Ollama?</summary>

While Ollama does provide similar functionalities, its design serves a different purpose. Ollama has a larger size (around 200MB) compared to Nitro's 3MB distribution. Nitro's compact size allows for easy embedding into subprocesses, ensuring minimal concerns about package size for your application. This makes Nitro a more suitable choice for applications where efficiency and minimal resource usage are key.

</details>

<details>
  <summary>4. Why is the model named "chat-gpt-3.5"?</summary>

Many applications implement the OpenAI ChatGPT API, and we want Nitro to be versatile for any AI client. While you can use any model name, we've ensured that if you're already using the chatgpt API, switching to Nitro is seamless. Just replace api.openai.com with localhost:3928 in your client settings (like Chatbox, Sillytavern, Oobaboga, etc.), and it will work smoothly with Nitro.

</details>
