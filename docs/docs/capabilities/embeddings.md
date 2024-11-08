---
title: Embeddings
---
:::info
🚧 Cortex is currently under development, and this page is a stub for future development.
:::

cortex.cpp now support embeddings endpoint with fully OpenAI compatible.

For embeddings API usage please refer to [API references](/api-reference#tag/chat/POST/v1/embeddings). This tutorial show you how to use embeddings in cortex with openai python SDK.

## Embedding with openai compatible

### 1. Start server and run model

```
cortex run llama3.1:8b-gguf-q4-km
```

### 2. Create script `embeddings.py` with this content

```
from datetime import datetime
from openai import OpenAI
from pydantic import BaseModel
ENDPOINT = "http://localhost:39281/v1"
MODEL = "llama3.1:8bb-gguf-q4-km"
client = OpenAI(
    base_url=ENDPOINT,
    api_key="not-needed"
)
```

### 3. Create embeddings

```
response = client.embeddings.create(input = "embedding", model=MODEL, encoding_format="base64")
print(response)
```

The reponse will be like this

```
CreateEmbeddingResponse(
    data=[
        Embedding(
            embedding='hjuAPOD8TryuPU8...',
            index=0,
            object='embedding'
        )
    ],
    model='meta-llama3.1-8b-instruct',
    object='list',
    usage=Usage(
        prompt_tokens=2,
        total_tokens=2
    )
)
```


The output embeddings is encoded as base64 string. Default the model will output the embeddings in float mode.

```
response = client.embeddings.create(input = "embedding", model=MODEL)
print(response)
```

Result will be

```
CreateEmbeddingResponse(
    data=[
        Embedding(
            embedding=[0.1, 0.3, 0.4 ....],
            index=0,
            object='embedding'
        )
    ],
    model='meta-llama3.1-8b-instruct',
    object='list',
    usage=Usage(
        prompt_tokens=2,
        total_tokens=2
    )
)
```

Cortex also supports all input types as [OpenAI](https://platform.openai.com/docs/api-reference/embeddings/create#embeddings-create-input).

```sh
# input as string
response = client.embeddings.create(input = "embedding", model=MODEL)

# input as array of string
response = client.embeddings.create(input = ["embedding"], model=MODEL)

# input as array of tokens
response = client.embeddings.create(input = [12,44,123], model=MODEL)

# input as array of arrays contain tokens
response = client.embeddings.create(input = [[912,312,54],[12,433,1241]], model=MODEL)
```
