---
title: Function Calling
---
# Function calling with OpenAI compatible

This tutorial, I use the `mistral-nemo:12b-gguf-q4-km` for testing function calling with cortex.cpp. All steps are reproduced from original openai instruction https://platform.openai.com/docs/guides/function-calling

## Step by step with function calling

### 1. Start server and run model.

```
cortex run mistral-nemo:12b-gguf-q4-km
```

### 2. Create a python script `function_calling.py` with this content:

```
from datetime import datetime
from openai import OpenAI
from pydantic import BaseModel
ENDPOINT = "http://localhost:39281/v1"
MODEL = "mistral-nemo:12b-gguf-q4-km"
client = OpenAI(
    base_url=ENDPOINT,
    api_key="not-needed"
)
```

This step creates OpenAI client in python

### 3. Start create a chat completion with tool calling

```
tools = [
    {
        "type": "function",
        "function": {
            "name": "get_delivery_date",

                "strict": True,
            "description": "Get the delivery date for a customer's order. Call this whenever you need to know the delivery date, for example when a customer asks 'Where is my package'",
            "parameters": {
                "type": "object",
                "properties": {
                    "order_id": {
                        "type": "string",
                        "description": "The customer's order ID.",
                    },
                },
                "required": ["order_id"],
                "additionalProperties": False,
            },
        }
    }
]
completion_payload = {
    "messages": [
        {"role": "system", "content": "You are a helpful customer support assistant. Use the supplied tools to assist the user."},
        {"role": "user", "content": "Hi, can you tell me the delivery date for my order?"},
    ]
}
response = client.chat.completions.create(
    top_p=0.9,
    temperature=0.6,
    model=MODEL,
    messages=completion_payload["messages"],
    tools=tools,
)
print(response)
```

Because you didn't provide the `order_id`, the model will ask again

```
ChatCompletion(
   id='1lblzWtLw9h5HG0GjYYi',
   choices=[
       Choice(
           finish_reason=None,
           index=0,
           logprobs=None,
           message=ChatCompletionMessage(
               content='Of course! Please provide your order ID so I can look it up.',
               refusal=None,
               role='assistant',
               audio=None,
               function_call=None,
               tool_calls=None
           )
       )
   ],
   created=1730204306,
   model='_',
   object='chat.completion',
   service_tier=None,
   system_fingerprint='_',
   usage=CompletionUsage(
       completion_tokens=15,
       prompt_tokens=449,
       total_tokens=464,
       completion_tokens_details=None,
       prompt_tokens_details=None
   )
)
```

### 4. Add new message user provide order id

```
completion_payload = {
    "messages": [
        {"role": "system", "content": "You are a helpful customer support assistant. Use the supplied tools to assist the user."},
        {"role": "user", "content": "Hi, can you tell me the delivery date for my order?"},
        {"role": "assistant", "content": "Of course! Please provide your order ID so I can look it up."},
        {"role": "user", "content": "i think it is order_12345"},
    ]
}

response = client.chat.completions.create(
    top_p=0.9,
    temperature=0.6,
    model=MODEL,
    messages=completion_payload["messages"],
    tools=tools
)
```

The response of the model will be

```
ChatCompletion(
   id='zUnHwEPCambJtrvWOAQy',
   choices=[
       Choice(
           finish_reason='tool_calls',
           index=0,
           logprobs=None,
           message=ChatCompletionMessage(
               content='',
               refusal=None,
               role='assistant',
               audio=None,
               function_call=None,
               tool_calls=[
                   ChatCompletionMessageToolCall(
                       id=None,
                       function=Function(
                           arguments='{"order_id": "order_12345"}',
                           name='get_delivery_date'
                       ),
                       type='function'
                   )
               ]
           )
       )
   ],
   created=1730204559,
   model='_',
   object='chat.completion',
   service_tier=None,
   system_fingerprint='_',
   usage=CompletionUsage(
       completion_tokens=23,
       prompt_tokens=483,
       total_tokens=506,
       completion_tokens_details=None,
       prompt_tokens_details=None
   )
)
```

It can return correct function with arguments

### 5. Push the response to the conversation and ask model to answer user

```
order_id = "order_12345"
delivery_date = datetime.now()

# Simulate the tool call response
response = {
    "choices": [
        {
            "message": {
                "role": "assistant",
                "tool_calls": [
                    {
                        "id": "call_62136354",
                        "type": "function",
                        "function": {
                            "arguments": "{'order_id': 'order_12345'}",
                            "name": "get_delivery_date"
                        }
                    }
                ]
            }
        }
    ]
}

# Create a message containing the result of the function call
function_call_result_message = {
    "role": "tool",
    "content": json.dumps({
        "order_id": order_id,
        "delivery_date": delivery_date.strftime('%Y-%m-%d %H:%M:%S')
    }),
    "tool_call_id": response['choices'][0]['message']['tool_calls'][0]['id']
}

# Prepare the chat completion call payload
completion_payload = {
    "messages": [
        {"role": "system", "content": "You are a helpful customer support assistant. Use the supplied tools to assist the user."},
        {"role": "user", "content": "Hi, can you tell me the delivery date for my order?"},
        {"role": "assistant", "content": "Sure! Could you please provide your order ID so I can look up the delivery date for you?"},
        {"role": "user", "content": "i think it is order_12345"},
        response["choices"][0]["message"],
        function_call_result_message
    ]
}

client = OpenAI(
    # This is the default and can be omitted
    base_url=ENDPOINT,
    api_key="not-needed"
)

response = client.chat.completions.create(
    top_p=0.9,
    temperature=0.6,
    model=MODEL,
    messages=completion_payload["messages"],
    tools=tools,
)
print(response)
```

The response will include all the content that processed by the function, where the delivery date is produced by query db, ....

```
ChatCompletion(
   id='l1xdCuKVMYBSC5tEDlAn',
   choices=[
       Choice(
           finish_reason=None,
           index=0,
           logprobs=None,
           message=ChatCompletionMessage(
               content="Your order with ID 'order_12345' is scheduled to be delivered on October 29, 2024. Is there anything else I can help you with?",
               refusal=None,
               role='assistant',
               audio=None,
               function_call=None,
               tool_calls=None
           )
       )
   ],
   created=1730205470,
   model='_',
   object='chat.completion',
   service_tier=None,
   system_fingerprint='_',
   usage=CompletionUsage(
       completion_tokens=40,
       prompt_tokens=568,
       total_tokens=608,
       completion_tokens_details=None,
       prompt_tokens_details=None
   )
)
```

## Handling parallel function calling

Cortex cpp support parallel function calling by default

```
tools = [
    {
        "type": "function",
        "function": {
            "name": "get_delivery_date",

                "strict": True,
            "description": "Get the delivery date for a customer's order. Call this whenever you need to know the delivery date, for example when a customer asks 'Where is my package'",
            "parameters": {
                "type": "object",
                "properties": {
                    "order_id": {
                        "type": "string",
                        "description": "The customer's order ID.",
                    },
                },
                "required": ["order_id"],
                "additionalProperties": False,
            },
        }
    },
    {
        "type": "function",
        "function": {
            "name": "get_current_conditions",
            "description": "Get the current weather conditions for a specific location",
            "parameters": {
                "type": "object",
                "properties": {
                    "location": {
                        "type": "string",
                        "description": "The city and state, e.g., San Francisco, CA"
                    },
                    "unit": {
                        "type": "string",
                        "enum": ["Celsius", "Fahrenheit"],
                        "description": "The temperature unit to use. Infer this from the user's location."
                    }
                },
                "required": ["location", "unit"]
            }
        }
    }
]

messages = [
    {"role": "user", "content": "Hi, can you tell me the delivery date for my order order_12345 and check the weather condition in LA?"}
]
response = client.chat.completions.create(
    top_p=0.9,
    temperature=0.6,
    model=MODEL,
    messages= messages, 
    tools=tools
)
print(response)
```

It will call 2 functions in parallel

```
ChatCompletion(
    id='5ot3qux399DojubnBFrG',
    choices=[
        Choice(
            finish_reason='tool_calls',
            index=0,
            logprobs=None,
            message=ChatCompletionMessage(
                content='',
                refusal=None,
                role='assistant',
                audio=None,
                function_call=None,
                tool_calls=[
                    ChatCompletionMessageToolCall(
                        id=None,
                        function=Function(
                            arguments='{"order_id": "order_12345"}',
                            name='get_delivery_date'
                        ),
                        type='function'
                    ),
                    ChatCompletionMessageToolCall(
                        id=None,
                        function=Function(
                            arguments='{"location": "LA", "unit": "Fahrenheit"}',
                            name='get_current_conditions'
                        ),
                        type='function'
                    )
                ]
            )
        )
    ],
    created=1730205975,
    model='_',
    object='chat.completion',
    service_tier=None,
    system_fingerprint='_',
    usage=CompletionUsage(
        completion_tokens=47,
        prompt_tokens=568,
        total_tokens=615,
        completion_tokens_details=None,
        prompt_tokens_details=None
    )
)
```

## Configuring function calling behavior using the tool_choice parameter

User can set `tool_choice=none` to disable function calling even if the tools are provided

```
response = client.chat.completions.create(
    top_p=0.9,
    temperature=0.6,
    model=MODEL,
    messages= messages, #completion_payload["messages"],
    tools=tools,
    tool_choice="none"
)
```

User can also force model to call a tool by specify the tool name, in this example it's the `get_current_conditions`

```
response = client.chat.completions.create(
    top_p=0.9,
    temperature=0.6,
    model=MODEL,
    messages= [{"role": "user", "content": "Hi, can you tell me the delivery date for my order order_12345 and check the weather condition in LA?"}],
    tools=tools,
    tool_choice= {"type": "function", "function": {"name": "get_current_conditions"}})

```

User can also specify the function with enum field to the tool definition to make model generate more accurate.

```
{
    "name": "pick_tshirt_size",
    "description": "Call this if the user specifies which size t-shirt they want",
    "parameters": {
        "type": "object",
        "properties": {
            "size": {
                "type": "string",
                "enum": ["s", "m", "l"],
                "description": "The size of the t-shirt that the user would like to order"
            }
        },
        "required": ["size"],
        "additionalProperties": false
    }
}
```

(*) Note that the accuracy of function calling heavily depends on the quality of the model. For small models like 8B or 12B, we should only use function calling with simple cases.

 The function calling feature from cortex.cpp is primarily an application of prompt engineering. When tools are specified, we inject a system prompt into the conversation to facilitate this functionality.

 Compatibility: This feature works best with models like llama3.1 and its derivatives, such as mistral-nemo or qwen.

 Customization: Users have the option to manually update the system prompt to fine-tune it for specific problems or use cases. The detail implementation is in this [PR](https://github.com/janhq/cortex.cpp/pull/1472/files).

 The full steps to mimic the function calling feature in Python using openai lib can be found [here](https://github.com/janhq/models/issues/16#issuecomment-2381129322).
