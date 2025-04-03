---
title: Structured Outputs
---

This guide demonstrates methods for getting structured JSON output from locally-hosted language models
like Llama and Mistral. We'll cover techniques for generating predictable data structures using open source LLMs.

## Start the model

```sh
cortex run -d llama3.1:8b-gguf-q4-km
```
```
llama3.1:8b-gguf-q4-km model started successfully. Use `cortex run llama3.1:8b-gguf-q4-km` for interactive chat shell
```

## Basic Example: Calendar Event

```python
from pydantic import BaseModel
from openai import OpenAI
import json
```
```py
client = OpenAI(
    base_url="http://localhost:39281/v1",
    api_key="not-needed"
)

class CalendarEvent(BaseModel):
    name: str
    date: str
    participants: list[str]
```
```py
completion = client.beta.chat.completions.parse(
    model="llama3.1:8b-gguf-q4-km",
    messages=[
        {"role": "system", "content": "Extract the event info as JSON"},
        {"role": "user", "content": "Alice and Bob are going to a science fair on Friday"}
    ],
    response_format=CalendarEvent,
    stop=["<|eot_id|>"]
)
print(json.dumps(completion.choices[0].message.parsed.dict(), indent=2))
```
```json
{
  "name": "science fair",
  "date": "Friday",
  "participants": ["Alice", "Bob"]
}
```

## Complex Example: Math Steps

Let's try something more complex with nested schemas. Here's structured math reasoning:

```py
class Step(BaseModel):
    explanation: str
    output: str

class MathReasoning(BaseModel):
    steps: list[Step]
    final_answer: str
```
```py
response = client.beta.chat.completions.parse(
    model="llama3.1:8b-gguf-q4-km",
    messages=[
        {
            "role": "system",
            "content": "Solve this math problem step by step. Output as JSON."
        },
        {
            "role": "user",
            "content": "how can I solve in a lot of detail, the equation 8x + 7 = -23"
        }
    ],
    response_format=MathReasoning,
    stop=["<|eot_id|>"]
)
print(json.dumps(response.choices[0].message.parsed.model_dump(), indent=2))
```
```json
{
  "steps": [
    {
      "explanation": "The given equation is 8x + 7 = -23. To isolate x, we need to get rid of the constant term (+7) on the left side.",
      "output": ""
    },
    {
      "explanation": "We can subtract 7 from both sides of the equation to get: 8x = -30",
      "output": "8x = -30"
    },
    {
      "explanation": "Now, we need to isolate x. To do this, we'll divide both sides of the equation by 8.",
      "output": ""
    },
    {
      "explanation": "Dividing both sides by 8 gives us: x = -3.75",
      "output": "x = -3.75"
    },
    {
      "explanation": "However, looking back at the original problem, we see that it's asking for the value of x in the equation 8x + 7 = -23.",
      "output": ""
    },
    {
      "explanation": "We can simplify this further by converting the decimal to a fraction.",
      "output": ""
    },
    {
      "explanation": "The decimal -3.75 is equivalent to -15/4. Therefore, x = -15/4",
      "output": "x = -15/4"
    }
  ],
  "final_answer": "x = -3"
}
```

## Quick JSON Lists

For straightforward lists, you can use the json_object response format:

```py
completion = client.chat.completions.create(
    model="llama3.1:8b-gguf-q4-km",
    messages=[
        {
            "role": "system",
            "content": "List songs in {song_name: release_year} format"
        },
        {
            "role": "user",
            "content": "List 10 songs"
        }
    ],
    response_format={"type": "json_object"},
    stop=["<|eot_id|>"]
)
print(json.dumps(json.loads(completion.choices[0].message.content), indent=2))
```

Output:
```json
{
  "Hotel California": 1976,
  "Stairway to Heaven": 1971,
  "Bohemian Rhapsody": 1975,
  "Smells Like Teen Spirit": 1991,
  "Viva la Vida": 2008,
  "Imagine": 1971,
  "Hotel Yorba": 2001,
  "Mr. Brightside": 2004,
  "Sweet Child O Mine": 1987,
  "Livin on a Prayer": 1986
}
```

## Pro Tips

Open source models have come a long way with structured outputs. A few things to keep in mind:

- Be explicit in your prompts about JSON formatting
- Use Pydantic models to enforce schema compliance
- Consider using the stop token to prevent extra output
- Some advanced features are still in beta

With proper prompting and schema validation, you can get reliable structured outputs from your local models. No cloud required!
