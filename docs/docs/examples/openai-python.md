---
title: Nitro with openai-python
description: Nitro intergration guide for Python.
keywords: [Nitro, Jan, fast inference, inference server, local AI, large language model, OpenAI compatible, open source, llama]
---


You can migrate from OAI API or Azure OpenAI to Nitro using your existing Python code quickly
> The **ONLY** thing you need to do is to override `baseURL` in `openai` init with `Nitro` URL
- Python OpenAI SDK: https://pypi.org/project/openai/

## Chat Completion
<table>
<tr>
<td> Engine </td> <td> Python Code </td>
</tr>
<tr>
<td> Nitro </td>
<td>

```python
import asyncio

from openai import AsyncOpenAI

# gets API Key from environment variable OPENAI_API_KEY
client = AsyncOpenAI(
    base_url="http://localhost:3928/v1/",
    api_key="sk-xxx"
)


async def main() -> None:
    stream = await client.chat.completions.create(
        model="gpt-4",
        messages=[{"role": "user", "content": "Say this is a test"}],
        stream=True,
    )
    async for completion in stream:
        print(completion.choices[0].delta.content, end="")
    print()

asyncio.run(main())
```
</td>
</tr>
<tr>
<td> OAI </td>
<td>

```python
import asyncio

from openai import AsyncOpenAI

# gets API Key from environment variable OPENAI_API_KEY
client = AsyncOpenAI(api_key="sk-xxx")


async def main() -> None:
    stream = await client.chat.completions.create(
        model="gpt-4",
        messages=[{"role": "user", "content": "Say this is a test"}],
        stream=True,
    )
    async for completion in stream:
        print(completion.choices[0].delta.content, end="")
    print()

asyncio.run(main())
```

</td>
</tr>
<tr>
<td> Azure OAI </td>
<td>

```python
from openai import AzureOpenAI

openai.api_key = '...' # Default is AZURE_OPENAI_API_KEY

stream = AzureOpenAI(
    api_version=api_version,
    azure_endpoint="https://example-endpoint.openai.azure.com",
)

completion = client.chat.completions.create(
    model="deployment-name",  # e.g. gpt-35-instant
    messages=[{"role": "user", "content": "Say this is a test"}],
    stream=True,
)
for part in stream:
    print(part.choices[0].delta.content or "")
```

</td>
</tr>
</table>

## Embedding
<table>
<tr>
<td> Engine </td> <td> Embedding </td>
</tr>
<tr>
<td> Nitro </td>
<td>

```python
import asyncio

from openai import AsyncOpenAI

# gets API Key from environment variable OPENAI_API_KEY
client = AsyncOpenAI(base_url="http://localhost:3928/v1/",
                     api_key="sk-xxx")


async def main() -> None:
    embedding = await client.embeddings.create(
        input='Hello How are you?', 
        model='text-embedding-ada-002'
    )
    print(embedding)

asyncio.run(main())
```
</td>
</tr>
<tr>
<td> OAI </td>
<td>

```python
import asyncio

from openai import AsyncOpenAI

# gets API Key from environment variable OPENAI_API_KEY
client = AsyncOpenAI(api_key="sk-xxx")


async def main() -> None:
    embedding = await client.embeddings.create(
        input='Hello How are you?',          
        model='text-embedding-ada-002'
    )
    print(embedding)

asyncio.run(main())
```

</td>
</tr>
<tr>
<td> Azure OAI </td>
<td>

```python
import openai

openai.api_type = "azure"
openai.api_key = YOUR_API_KEY
openai.api_base = "https://YOUR_RESOURCE_NAME.openai.azure.com"
openai.api_version = "2023-05-15"

response = openai.embedding.create(
    input="Your text string goes here",
    engine="YOUR_DEPLOYMENT_NAME"
)
embeddings = response['data'][0]['embedding']
print(embeddings)
```

</td>
</tr>
</table>

## Audio

:::info Coming soon
:::

## How to reproduce
**Step 1:** Dependencies installation.

```bash title="Install OpenAI"
pip install openai
```

**Step 2:** Fill `index.py` file with code.

**Step 3:** Run the code with `python index.py`.