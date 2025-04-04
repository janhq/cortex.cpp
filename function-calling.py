from datetime import datetime
from openai import OpenAI
from pydantic import BaseModel
import json

# MODEL = "deepseek-r1-distill-qwen-7b:7b"
MODEL = "llama3.1:8b-q8"

client = OpenAI(
    base_url="http://localhost:39281/v1",
    api_key="not-needed",  # Authentication is not required for local deployment
)

tools = [
    {
        "type": "function",
        "function": {
            "name": "puppeteer_navigate",
            "description": "Navigate to a URL",
            "parameters": {
                "properties": {"url": {"type": "string"}},
                "required": ["url"],
                "type": "object",
            },
            "strict": False,
        },
    },
    {
        "type": "function",
        "function": {
            "name": "puppeteer_screenshot",
            "description": "Take a screenshot of the current page or a specific element",
            "parameters": {
                "properties": {
                    "height": {
                        "description": "Height in pixels (default: 600)",
                        "type": "number",
                    },
                    "name": {
                        "description": "Name for the screenshot",
                        "type": "string",
                    },
                    "selector": {
                        "description": "CSS selector for element to screenshot",
                        "type": "string",
                    },
                    "width": {
                        "description": "Width in pixels (default: 800)",
                        "type": "number",
                    },
                },
                "required": ["name"],
                "type": "object",
            },
            "strict": False,
        },
    },
    {
        "type": "function",
        "function": {
            "name": "puppeteer_click",
            "description": "Click an element on the page",
            "parameters": {
                "properties": {
                    "selector": {
                        "description": "CSS selector for element to click",
                        "type": "string",
                    }
                },
                "required": ["selector"],
                "type": "object",
            },
            "strict": False,
        },
    },
    {
        "type": "function",
        "function": {
            "name": "puppeteer_fill",
            "description": "Fill out an input field",
            "parameters": {
                "properties": {
                    "selector": {
                        "description": "CSS selector for input field",
                        "type": "string",
                    },
                    "value": {"description": "Value to fill", "type": "string"},
                },
                "required": ["selector", "value"],
                "type": "object",
            },
            "strict": False,
        },
    },
    {
        "type": "function",
        "function": {
            "name": "puppeteer_select",
            "description": "Select an element on the page with Select tag",
            "parameters": {
                "properties": {
                    "selector": {
                        "description": "CSS selector for element to select",
                        "type": "string",
                    },
                    "value": {"description": "Value to select", "type": "string"},
                },
                "required": ["selector", "value"],
                "type": "object",
            },
            "strict": False,
        },
    },
    {
        "type": "function",
        "function": {
            "name": "puppeteer_hover",
            "description": "Hover an element on the page",
            "parameters": {
                "properties": {
                    "selector": {
                        "description": "CSS selector for element to hover",
                        "type": "string",
                    }
                },
                "required": ["selector"],
                "type": "object",
            },
            "strict": False,
        },
    },
    {
        "type": "function",
        "function": {
            "name": "puppeteer_evaluate",
            "description": "Execute JavaScript in the browser console",
            "parameters": {
                "properties": {
                    "script": {
                        "description": "JavaScript code to execute",
                        "type": "string",
                    }
                },
                "required": ["script"],
                "type": "object",
            },
            "strict": False,
        },
    },
]

completion_payload = {
    "messages": [
        {
            "role": "system",
            "content": 'You have access to the following CUSTOM functions:\n\n<CUSTOM_FUNCTIONS>\n\nIf a you choose to call a function ONLY reply in the following format:\n<{start_tag}={function_name}>{parameters}{end_tag}\nwhere\n\nstart_tag => `<function`\nparameters => a JSON dict with the function argument name as key and function argument value as value.\nend_tag => `</function>`\n\nHere is an example,\n<function=example_function_name>{"example_name": "example_value"}</function>\n\nReminder:\n- Function calls MUST follow the specified format\n- Required parameters MUST be specified\n- You can call one or more functions at a time, but remember only chose correct function\n- Put the entire function call reply on one line\n- Always add your sources when using search results to answer the user query\n- If you can not find correct parameters or arguments corresponding to function in the user\'s message, ask user again to provide, do not make assumptions.\n- No explanation are needed when calling a function.\n\nYou are a helpful assistant.',
        },
        {
            "role": "user",
            "content": "go to google search",
        },
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