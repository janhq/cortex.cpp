---
title: Embeddings
---
:::warning
🚧 Cortex is currently under active development. Our documentation outlines the intended behavior of
Cortex, which may not yet be fully implemented in the codebase.
:::

Cortex now support an embeddings endpoint that is fully compatible with OpenAI's one.
This tutorial show you how to create embeddings in cortex using the OpenAI python SDK.

## Embedding with openai compatible

Start server and run model in detached mode.

```sh
cortex run -d llama3.1:8b-gguf-q4-km
```

Create a directory and a python environment, and start a python or IPython shell.

```sh
mkdir test-embeddings
cd test-embeddings
```
```sh
python -m venv .venv
source .venv/bin/activate
pip install ipython openai
```
```sh
ipython
```
Import the necessary modules and create a client.

```py
from datetime import datetime
from openai import OpenAI
from pydantic import BaseModel
```
```py
client = OpenAI(
    base_url="http://localhost:39281/v1",
    api_key="not-needed"
)
```

### 3. Create embeddings

```py
output_embs = client.embeddings.create(
    input="Roses are red, violets are blue, Cortex is great, and so is Jan too!",
    model="llama3.1:8b-gguf-q4-km",
    # encoding_format="base64"
)
```
```py
print(output_embs)
```
```
CreateEmbeddingResponse(
    data=[
        Embedding(
            embedding=[-0.017303412780165672, -0.014513173140585423, ...],
            index=0,
            object='embedding'
        )
    ],
    model='llama3.1:8b-gguf-q4-km',
    object='list',
    usage=Usage(
        prompt_tokens=22,
        total_tokens=22
    )
)
```

Cortex also supports the same input types as [OpenAI](https://platform.openai.com/docs/api-reference/embeddings/create).

```py
# input as string
response = client.embeddings.create(input = "single prompt or article or other", model=MODEL)
```
```py
# input as array of string
response = client.embeddings.create(input = ["list", "of", "prompts"], model=MODEL)
```
```py
# input as array of tokens
response = client.embeddings.create(input = [12, 44, 123], model=MODEL)
```
```py
# input as array of arrays contain tokens
response = client.embeddings.create(input = [[912,312,54],[12,433,1241]], model=MODEL)
```
