---
title: Structured Outputs
---
# Structured Outputs

The Structured Outputs/Response Format feature in [OpenAI](https://platform.openai.com/docs/guides/structured-outputs) is fundamentally a prompt engineering challenge. While its goal is to use system prompts to generate JSON output matching a specific schema, popular open-source models like Llama 3.1 and Mistral Nemo struggle to consistently generate exact JSON output that matches the requirements. An easy way to directly guild the model to reponse in json format in system message:

```
from openai import OpenAI
from pydantic import BaseModel
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

response = client.chat.completions.create(
    top_p=0.9,
    temperature=0.6,
    model=MODEL,
    messages=completion_payload["messages"]
)

print(response)
```

The output of the model like this

```

ChatCompletion(
    id='OZI0q8hghjYQY7NXlLId',
    choices=[
        Choice(
            finish_reason=None,
            index=0,
            logprobs=None,
            message=ChatCompletionMessage(
                content='''Here's how you can solve it:

{
    "steps": [
        {
            "explanation": "First, we need to isolate the variable x. To do this, subtract 7 from both sides of the equation.",
            "output": "8x + 7 - 7 = -23 - 7"
        },
        {
            "explanation": "This simplifies to 8x = -30",
            "output": "8x = -30"
        },
        {
            "explanation": "Next, divide both sides of the equation by 8 to solve for x.",
            "output": "(8x) / 8 = -30 / 8"
        },
        {
            "explanation": "This simplifies to x = -3.75",
            "output": "x = -3.75"
        }
    ],
    "final_output": "-3.75"
}''',
                refusal=None,
                role='assistant',
                audio=None,
                function_call=None,
                tool_calls=None
            )
        )
    ],
    created=1730645716,
    model='_',
    object='chat.completion',
    service_tier=None,
    system_fingerprint='_',
    usage=CompletionUsage(
        completion_tokens=190,
        prompt_tokens=78,
        total_tokens=268,
        completion_tokens_details=None,
        prompt_tokens_details=None
    )
)
```

From the output, you can easily parse the response to get correct json format as you guild the model in the system prompt.

Howerver, open source model like llama3.1 or mistral nemo still truggling on mimic newest OpenAI API on response format. For example, consider this request created using the OpenAI library with very simple request like [OpenAI](https://platform.openai.com/docs/guides/structured-outputs#chain-of-thought):

```
from openai import OpenAI
ENDPOINT = "http://localhost:39281/v1"
MODEL = "llama3.1:8b-gguf-q4-km"
client = OpenAI(
    base_url=ENDPOINT,
    api_key="not-needed"
)

class Step(BaseModel):
    explanation: str
    output: str


class MathReasoning(BaseModel):
    steps: List[Step]
    final_answer: str

  
completion_payload = {
    "messages": [
        {"role": "system", "content": f"You are a helpful math tutor. Guide the user through the solution step by step.\n"},
        {"role": "user", "content": "how can I solve 8x + 7 = -23"}
    ]
}

response = client.beta.chat.completions.parse(
    top_p=0.9,
    temperature=0.6,
    model=MODEL,
    messages= completion_payload["messages"],
    response_format=MathReasoning
)
```

The response format parsed by OpenAI before sending to the server is quite complex for the `MathReasoning` schema. Unlike GPT models, Llama 3.1 and Mistral Nemo cannot reliably generate responses that can be parsed as shown in the [OpenAI tutorial](https://platform.openai.com/docs/guides/structured-outputs/example-response). This may be due to these models not being trained on similar structured output tasks.

```
"response_format" : 
        {
                "json_schema" : 
                {
                        "name" : "MathReasoning",
                        "schema" : 
                        {
                                "$defs" : 
                                {
                                        "Step" : 
                                        {
                                                "additionalProperties" : false,
                                                "properties" : 
                                                {
                                                        "explanation" : 
                                                        {
                                                                "title" : "Explanation",
                                                                "type" : "string"
                                                        },
                                                        "output" : 
                                                        {
                                                                "title" : "Output",
                                                                "type" : "string"
                                                        }
                                                },
                                                "required" : 
                                                [
                                                        "explanation",
                                                        "output"
                                                ],
                                                "title" : "Step",
                                                "type" : "object"
                                        }
                                },
                                "additionalProperties" : false,
                                "properties" : 
                                {
                                        "final_answer" : 
                                        {
                                                "title" : "Final Answer",
                                                "type" : "string"
                                        },
                                        "steps" : 
                                        {
                                                "items" : 
                                                {
                                                        "$ref" : "#/$defs/Step"
                                                },
                                                "title" : "Steps",
                                                "type" : "array"
                                        }
                                },
                                "required" : 
                                [
                                        "steps",
                                        "final_answer"
                                ],
                                "title" : "MathReasoning",
                                "type" : "object"
                        },
                        "strict" : true
                },
                "type" : "json_schema"
        }
```

The response for this request by `mistral-nemo` and `llama3.1` can not be used to parse result like in the [original tutorial by openAI](https://platform.openai.com/docs/guides/structured-outputs/example-response). Maybe `llama3.1` and `mistral-nemo` didn't train with this kind of data, so it fails to handle this case.

```
Response: {
        "choices" : 
        [
                {
                        "finish_reason" : null,
                        "index" : 0,
                        "message" : 
                        {
                                "content" : "Here's a step-by-step guide to solving the equation 8x + 7 = -23:\n\n```json\n{\n  \"name\": \"MathReasoning\",\n  \"schema\": {\n    \"$defs\": {\n      \"Step\": {\n        \"additionalProperties\": false,\n        \"properties\": {\n          \"explanation\": {\"title\": \"Explanation\", \"type\": \"string\"},\n          \"output\": {\"title\": \"Output\", \"type\": \"string\"}\n        },\n        \"required\": [\"explanation\", \"output\"],\n        \"title\": \"Step\",\n        \"type\": \"object\"\n      }\n    },\n    \"additionalProperties\": false,\n    \"properties\": {\n      \"final_answer\": {\"title\": \"Final Answer\", \"type\": \"string\"},\n      \"steps\": {\n        \"items\": {\"$ref\": \"#/$defs/Step\"},\n        \"title\": \"Steps\",\n        \"type\": \"array\"\n      }\n    },\n    \"required\": [\"steps\", \"final_answer\"],\n    \"title\": \"MathReasoning\",\n    \"type\": \"object\"\n  },\n  \"strict\": true\n}\n```\n\n1. **Subtract 7 from both sides** to isolate the term with x:\n\n   - Explanation: To get rid of the +7 on the left side, we add -7 to both sides of the equation.\n   - Output: `8x + 7 - 7 = -23 - 7`\n\n   This simplifies to:\n   ```\n   8x = -30\n   ```\n\n2. **Divide both sides by 8** to solve for x:\n\n   - Explanation: To get rid of the 8 on the left side, we multiply both sides of the equation by the reciprocal of 8, which is 1/8.\n   - Output: `8x / 8 = -30 / 8`\n\n   This simplifies to:\n   ```\n   x = -3.75\n   ```\n\nSo, the final answer is:\n\n- Final Answer: `x = -3.75`",
                                "role" : "assistant"
                        }
                }
        ],
```

This feature currently works reliably only with GPT models, not with open-source models. Given these limitations, we suggest that you should only use Response Format feature as the first example (guild the json format for the reponse for model). Besides, the response format maybe just in beta because we have to use `client.beta.chat.completions.parse` to create chat completion instead of `client.chat.completion.create`
