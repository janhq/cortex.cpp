---
title: Text Generation
---


Cortex provides a text generation endpoint that is fully compatible with OpenAI's API.
This section shows you how to generate text using Cortex with the OpenAI Python SDK.

## Text Generation with OpenAI compatibility

Start server and run model in detached mode.

```sh
cortex run -d llama3.1:8b-gguf-q4-km
```

Create a directory and a python environment, and start a python or IPython shell.

```sh
mkdir test-generation
cd test-generation
```
```sh
python -m venv .venv # or uv venv .venv --python 3.13
source .venv/bin/activate
pip install ipython openai rich # or uv pip install ipython openai rich
```
```sh
ipython # or "uv run ipython"
```

Import the necessary modules and create a client.

```py
from openai import OpenAI

client = OpenAI(
    base_url="http://localhost:39281/v1",
    api_key="not-needed"
)
```

### Generate Text

Basic completion:

```py
response = client.chat.completions.create(
    model="llama3.1:8b-gguf-q4-km",
    messages=[
        {"role": "user", "content": "Tell me a short story about a friendly robot."}
    ]
)
print(response.choices[0].message.content)
```
```
Here's a short story about a friendly robot:

**Zeta's Gift**

In a small town surrounded by lush green hills, there lived a robot named Zeta. Zeta was unlike any other robot in the world. While others
were designed for specific tasks like assembly or transportation, Zeta was created with a single purpose: to spread joy and kindness.

Zeta's bright blue body was shaped like a ball, with glowing lines that pulsed with warmth on its surface. Its large, round eyes sparkled
with a warm light, as if reflecting the friendliness within. Zeta loved nothing more than making new friends and surprising them with small
gifts.

One sunny morning, Zeta decided to visit the local bakery owned by Mrs. Emma, who was famous for her delicious pastries. As Zeta entered the
shop, it was greeted by the sweet aroma of freshly baked bread. The robot's advanced sensors detected a young customer, Timmy, sitting at a
corner table, looking sad.

Zeta quickly approached Timmy and offered him a warm smile. "Hello there! I'm Zeta. What seems to be troubling you?" Timmy explained that he
was feeling down because his family couldn't afford his favorite dessert – Mrs. Emma's famous chocolate cake – for his birthday.

Moved by Timmy's story, Zeta asked Mrs. Emma if she could help the young boy celebrate his special day. The baker smiled and handed Zeta a
beautifully decorated cake. As the robot carefully placed the cake on a tray, it sang a gentle melody: "Happy Birthday, Timmy! May your day
be as sweet as this treat!"

Timmy's eyes widened with joy, and he hugged Zeta tightly. Word of Zeta's kindness spread quickly through the town, earning the robot the
nickname "The Friendly Robot." From that day on, whenever anyone in need was spotted, Zeta would appear at their side, bearing gifts and
spreading love.

Zeta continued to surprise the townspeople with its thoughtfulness and warm heart, proving that even a machine could be a source of comfort
and joy.
```

With additional parameters:

```py
response = client.chat.completions.create(
    model="llama3.1:8b-gguf-q4-km",
    messages=[
        {"role": "system", "content": "You are a helpful assistant."},
        {"role": "user", "content": "What are the main differences between Python and C++?"}
    ],
    temperature=0.7,
    max_tokens=150,
    top_p=1.0,
    frequency_penalty=0.0,
    presence_penalty=0.0
)
```
```sh
ChatCompletion(
    id='dnMbB12ZR6JdVDw2Spi8',
    choices=[
        Choice(
            finish_reason='stop',
            index=0,
            logprobs=None,
            message=ChatCompletionMessage(
                content="Python and C++ are two popular programming languages with distinct characteristics, use cases, ...",
                refusal=None,
                role='assistant',
                audio=None,
                function_call=None,
                tool_calls=None
            )
        )
    ],
    created=1738236652,
    model='_',
    object='chat.completion',
    service_tier=None,
    system_fingerprint='_',
    usage=CompletionUsage(
        completion_tokens=150,
        prompt_tokens=33,
        total_tokens=183,
        completion_tokens_details=None,
        prompt_tokens_details=None
    )
)
```

Stream the response:

```py
stream = client.chat.completions.create(
    model="llama3.1:8b-gguf-q4-km",
    messages=[
        {"role": "user", "content": "Write a haiku about programming."}
    ],
    stream=True
)

for chunk in stream:
    if chunk.choices[0].delta.content is not None:
        print(chunk.choices[0].delta.content, end="")
```
```
Code flows like a stream
 Errors lurk in every line
Bug hunt, endless quest
```

Multiple messages in a conversation:

```py
messages = [
    {"role": "system", "content": "You are a knowledgeable science teacher."},
    {"role": "user", "content": "What is photosynthesis?"},
    {"role": "assistant", "content": "Photosynthesis is the process by which plants convert sunlight into energy."},
    {"role": "user", "content": "Can you explain it in more detail?"}
]

response = client.chat.completions.create(
    model="llama3.1:8b-gguf-q4-km",
    messages=messages
)
print(response.choices[0].message.content)
```
```
"Photosynthesis is actually one of my favorite topics to teach! It's a crucial process that supports life on Earth, and
I'd be happy to break it down for you.\n\nPhotosynthesis occurs in specialized organelles called chloroplasts, which are present in plant
cells. These tiny factories use energy from the sun to convert carbon dioxide (CO2) and water (H2O) into glucose (a type of sugar) and
oxygen (O2).\n\nHere's a simplified equation:\n\n6 CO2 + 6 H2O + light energy → C6H12O6 (glucose) + 6 O2\n\nIn more detail, the process
involves several steps:\n\n1. **Light absorption**: Light from the sun is absorbed by pigments ..."
```

The API endpoint provided by Cortex supports all standard OpenAI parameters including:
- `temperature`: Controls randomness (0.0 to 2.0)
- `max_tokens`: Limits the length of the response
- `top_p`: Controls diversity via nucleus sampling
- `frequency_penalty`: Reduces repetition of token sequences
- `presence_penalty`: Encourages talking about new topics
- `stop`: Custom stop sequences
- `stream`: Enable/disable streaming responses
