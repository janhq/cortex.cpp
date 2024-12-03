---
title: Structured Outputs
---
# Structured Outputs

Structured outputs, or response formats, are a feature designed to generate responses in a defined JSON schema, enabling more predictable and machine-readable outputs. This is essential for applications where data consistency and format adherence are crucial, such as automated data processing, structured data generation, and integrations with other systems.

In recent developments, systems like OpenAI's models have excelled at producing these structured outputs. However, while open-source models like Llama 3.1 and Mistral Nemo offer powerful capabilities, they currently struggle to produce reliably structured JSON outputs required for advanced use cases. 

This guide explores the concept of structured outputs using these models, highlights the challenges faced in achieving consistent output formatting, and provides strategies for improving output accuracy, particularly when using models that don't inherently support this feature as robustly as GPT models.

By understanding these nuances, users can make informed decisions when choosing models for tasks requiring structured outputs, ensuring that the tools they select align with their project's formatting requirements and expected accuracy.

The Structured Outputs/Response Format feature in [OpenAI](https://platform.openai.com/docs/guides/structured-outputs) is fundamentally a prompt engineering challenge. While its goal is to use system prompts to generate JSON output matching a specific schema, popular open-source models like Llama 3.1 and Mistral Nemo struggle to consistently generate exact JSON output that matches the requirements. An easy way to directly guild the model to reponse in json format in system message, you just need to pass the pydantic model to `response_format`:

```
from pydantic import BaseModel
from openai import OpenAI
import json
ENDPOINT = "http://localhost:39281/v1"
MODEL = "llama3.1:8b-gguf-q4-km"

client = OpenAI(
    base_url=ENDPOINT,
    api_key="not-needed"
)


class CalendarEvent(BaseModel):
    name: str
    date: str
    participants: list[str]


completion = client.beta.chat.completions.parse(
    model=MODEL,
    messages=[
        {"role": "system", "content": "Extract the event information."},
        {"role": "user", "content": "Alice and Bob are going to a science fair on Friday."},
    ],
    response_format=CalendarEvent,
    stop=["<|eot_id|>"]
)

event = completion.choices[0].message.parsed

print(json.dumps(event.dict(), indent=4))
```

The output of the model like this

```
{
    "name": "science fair",
    "date": "Friday",
    "participants": [
        "Alice",
        "Bob"
    ]
}
```

With more complex json format, llama3.1 still struggle to response correct answer:

```

from openai import OpenAI
from pydantic import BaseModel
import json
ENDPOINT = "http://localhost:39281/v1"
MODEL = "llama3.1:8b-gguf-q4-km"
client = OpenAI(
    base_url=ENDPOINT,
    api_key="not-needed"
)

format = {
    "steps": [{
        "explanation": "string",
        "output": "string"
    }
    ],
    "final_output": "string"
}

completion_payload = {
    "messages": [
        {"role": "system", "content": f"You are a helpful math tutor. Guide the user through the solution step by step. You have to response in this json format {format}\n"},
        {"role": "user", "content": "how can I solve 8x + 7 = -23"}
    ]
}


class Step(BaseModel):
    explanation: str
    output: str


class MathReasoning(BaseModel):
    steps: list[Step]
    final_answer: str


response = client.beta.chat.completions.parse(
    top_p=0.9,
    temperature=0.6,
    model=MODEL,
    messages=completion_payload["messages"],
    stop=["<|eot_id|>"],
    response_format=MathReasoning
)

math_reasoning = response.choices[0].message.parsed
print(json.dumps(math_reasoning.dict(), indent=4))
```

The output of model looks like this

```
{
    "steps": [
        {
            "explanation": "To isolate the variable x, we need to get rid of the constant term on the left-hand side. We can do this by subtracting 7 from both sides of the equation.",
            "output": "8x + 7 - 7 = -23 - 7"
        },
        {
            "explanation": "Simplifying the left-hand side, we get:",
            "output": "8x = -30"
        },
        {
            "explanation": "Now, to solve for x, we need to isolate it by dividing both sides of the equation by 8.",
            "output": "8x / 8 = -30 / 8"
        },
        {
            "explanation": "Simplifying the right-hand side, we get:",
            "output": "x = -3.75"
        }
    ],
    "final_answer": "There is no final answer yet, let's break it down step by step."
}
```

Even if the model can generate correct format but the information doesn't 100% accurate, the `final_answer` should be `-3.75` instead of `There is no final answer yet, let's break it down step by step.`.

Another usecase for structured output with json response, you can provide the  `response_format={"type" : "json_object"}`, the model will be force to generate json output.

```
json_format = {"song_name":"release date"}
completion = client.chat.completions.create(
    model=MODEL,
    messages=[
        {"role": "system", "content": f"You are a helpful assistant, you must reponse with this format: '{json_format}'"},
        {"role": "user", "content": "List 10 songs for me"}
    ],
    response_format={"type": "json_object"},
    stop=["<|eot_id|>"]
)

print(json.dumps(json.loads(completion.choices[0].message.content), indent=4))
```

The output will looks like this:

```
{
    "Happy": "2013",
    "Uptown Funk": "2014",
    "Shut Up and Dance": "2014",
    "Can't Stop the Feeling!": "2016",
    "We Found Love": "2011",
    "All About That Bass": "2014",
    "Radioactive": "2012",
    "SexyBack": "2006",
    "Crazy": "2007",
    "Viva la Vida": "2008"
}
```

## Limitations of Open-Source Models for Structured Outputs

While the concept of structured outputs is compelling, particularly for applications requiring machine-readable data, it's important to understand that not all models support this capability equally. Open-source models such as Llama 3.1 and Mistral Nemo face notable challenges in generating outputs that adhere strictly to defined JSON schemas. Here are the key limitations:

- Lack of Training Data: These models have not been specifically trained on tasks demanding precise JSON formatting, unlike some proprietary models which have been fine-tuned for such tasks.
- Inconsistency in Output: Due to their training scope, `Llama 3.1` and `Mistral Nemo` often produce outputs that may deviate from the intended schema. This can include additional natural language explanations or incorrectly nested JSON structures.
- Complexity in Parsing: Without consistent JSON formatting, downstream processes that rely on predictable data schemas may encounter errors, leading to challenges in automation and data integration tasks.
- Beta Features: Some features related to structured outputs may still be in beta, requiring usage of specific methods like `client.beta.chat.completions.parse`, which suggests they are not yet fully reliable in all scenarios.

Given these constraints, users should consider these limitations when choosing a model for tasks involving structured outputs. Where strict compliance with a JSON schema is critical, alternative models designed for such precision might be a more suitable choice.
