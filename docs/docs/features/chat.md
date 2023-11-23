---
title: Chat Completion
---

The Chat Completion feature in Nitro provides a flexible way to interact with any local Large Language Model (LLM).

## Single Request Example

To send a single query to your chosen LLM, follow these steps:

<div style={{ width: '50%', float: 'left', clear: 'left' }}>

```bash title="Nitro"
curl http://localhost:3928/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "model": "",
    "messages": [
      {
        "role": "user",
        "content": "Hello"
      },
    ]
  }'

```

</div>

<div style={{ width: '50%', float: 'right', clear: 'right' }}>

```bash title="OpenAI"
curl https://api.openai.com/v1/chat/completions \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $OPENAI_API_KEY" \
  -d '{
    "model": "gpt-3.5-turbo",
    "messages": [
      {
        "role": "user",
        "content": "Hello"
      }
    ]
  }'
```

</div>

This command sends a request to your local LLM, querying about the winner of the 2020 World Series.

### Dialog Request Example

For ongoing conversations or multiple queries, the dialog request feature is ideal. Here’s how to structure a multi-turn conversation:

<div style={{ width: '50%', float: 'left', clear: 'left' }}>

```bash title="Nitro"
curl http://localhost:3928/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "messages": [
      {
        "role": "system",
        "content": "You are a helpful assistant."
      },
      {
        "role": "user",
        "content": "Who won the world series in 2020?"
      },
      {
        "role": "assistant",
        "content": "The Los Angeles Dodgers won the World Series in 2020."
      },
      {
        "role": "user",
        "content": "Where was it played?"
      }
    ]
  }'

```

</div>

<div style={{ width: '50%', float: 'right', clear: 'right' }}>

```bash title="OpenAI"
curl https://api.openai.com/v1/chat/completions \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $OPENAI_API_KEY" \
  -d '{
    "messages": [
      {
        "role": "system",
        "content": "You are a helpful assistant."
      },
      {
        "role": "user",
        "content": "Who won the world series in 2020?"
      },
      {
        "role": "assistant",
        "content": "The Los Angeles Dodgers won the World Series in 2020."
      },
      {
        "role": "user",
        "content": "Where was it played?"
      }
    ]
  }'
```

</div>

### Chat Completion Response

Below are examples of responses from both the Nitro server and OpenAI:

<div style={{ width: '50%', float: 'left', clear: 'left' }}>

```js title="Nitro"
{
  "choices": [
    {
      "finish_reason": "stop",
      "index": 0,
      "message": {
        "content": "Hello, how may I assist you this evening?",
        "role": "assistant"
      }
    }
  ],
  "created": 1700215278,
  "id": "sofpJrnBGUnchO8QhA0s",
  "model": "_",
  "object": "chat.completion",
  "system_fingerprint": "_",
  "usage": {
    "completion_tokens": 13,
    "prompt_tokens": 90,
    "total_tokens": 103
  }
}
```

</div>

<div style={{ width: '50%', float: 'right', clear: 'right' }}>

```js title="OpenAI"
{
  "choices": [
    {
      "finish_reason": "stop",
      "index": 0,
      "message": {
        "role": "assistant",
        "content": "Hello there, how may I assist you today?",
      }
    }
  ],
  "created": 1677652288,
  "id": "chatcmpl-123",
  "model": "gpt-3.5-turbo-0613",
  "object": "chat.completion",
  "system_fingerprint": "fp_44709d6fcb",
  "usage": {
    "completion_tokens": 12,
    "prompt_tokens": 9,
    "total_tokens": 21
  }
}
```

</div>

The chat completion feature in Nitro showcases compatibility with OpenAI, making the transition between using OpenAI and local AI models more straightforward. For further details and advanced usage, please refer to the [API reference](https://nitro.jan.ai/api-reference).
