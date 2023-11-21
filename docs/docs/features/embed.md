---
title: Embedding
---

## What are embeddings?

Embeddings are lists of numbers (floats). To find how similar two embeddings are, we measure the [distance](https://en.wikipedia.org/wiki/Cosine_similarity) between them. Shorter distances mean they're more similar; longer distances mean less similarity.

## Activating Embedding Feature

To utilize the embedding feature, include the JSON parameter `"embedding": true` in your [load model request](features/load-unload.md). This action enables Nitro to process inferences with embedding capabilities.

### Embedding Request

Here’s an example showing how to get the embedding result from the model:

<div style={{ width: '50%', float: 'left', clear: 'left' }}>

```bash title="Nitro" {1}
curl http://localhost:3928/v1/embeddings \
    -H 'Content-Type: application/json' \
    -d '{
        "input": "Hello",
        "model":"Llama-2-7B-Chat-GGUF",
        "encoding_format": "float"
    }'

```

</div>
<div style={{ width: '50%', float: 'right', clear: 'right' }}>

```bash title="OpenAI request" {1}
curl https://api.openai.com/v1/embeddings \
  -H "Authorization: Bearer $OPENAI_API_KEY" \
  -H "Content-Type: application/json" \
  -d '{
    "input": "Hello",
    "model": "text-embedding-ada-002",
    "encoding_format": "float"
  }'
```

</div>

## Embedding Reponse

The example response used the output from model [llama2 Chat 7B Q5 (GGUF)](https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/tree/main) loaded to Nitro server.

<div style={{ width: '50%', float: 'left', clear: 'left' }}>

```js title="Nitro"
{
  "data": [
    {
      "embedding": [
        -0.9874749,
        0.2965493,
        ...
        -0.253227
      ],
      "index": 0,
      "object": "embedding"
    }
  ]
}
```

</div>

<div style={{ width: '50%', float: 'right', clear: 'right' }}>

```js title="OpenAI"
{
  "embedding": [
    0.0023064255,
    -0.009327292,
    .... (1536 floats total for ada-002)
    -0.0028842222,
  ],
  "index": 0,
  "object": "embedding"
}




```

</div>

The embedding feature in Nitro demonstrates a high level of compatibility with OpenAI, simplifying the transition between using OpenAI and local AI models. For more detailed information and advanced use cases, refer to the comprehensive [API Reference](https://nitro.jan.ai/api-reference)).
