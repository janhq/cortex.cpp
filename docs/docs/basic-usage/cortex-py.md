---
title: cortex.py
description: How to integrate cortex.py with a Python application.
---


:::warning
🚧 Cortex.py is currently under development, and this page is a stub for future development. 
:::


<!-- 
:::warning
🚧 Cortex.cpp is currently under development. Our documentation outlines the intended behavior of Cortex, which may not yet be fully implemented in the codebase.
:::
Cortex.cpp can be used in a Python application with the `cortex.py` library. Cortex.cpp provides a Python client library as a **fork of OpenAI's [Python library](https://github.com/openai/openai-python)** with additional methods for Local AI.
## Installation

```py
pip install @janhq/cortex-python
```

## Usage

1. Replace the OpenAI import with Cortex.cpp in your application:

```diff
- from openai import OpenAI
+ from @janhq/cortex-python import Cortex
```

2. Modify the initialization of the client to use Cortex.cpp:

```diff
- client = OpenAI(api_key='your-api-key')
+ client = Cortex(base_url="BASE_URL", api_key="API_KEY")  # This can be omitted if using the default

```

### Example Usage

```py
from @janhq/cortex-python import Cortex

client = OpenAI(base_url="http://localhost:3928", api_key="cortex")

model = "TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF"
client.models.start(model=model)

completion = client.chat.completions.create(
    model=model,
    messages=[
        {
            "role": "user",
            "content": "Say this is a test",
        },
    ],
)
print(completion.choices[0].message.content)
``` -->
