---
title: Assistants
---

# Building Local AI Assistants

While Cortex doesn't yet support the full OpenAI Assistants API, we can build assistant-like functionality
using the chat completions API. Here's how to create persistent, specialized assistants locally.

## Get Started

First, fire up our model:

```sh
cortex run -d llama3.1:8b-gguf-q4-km
```

Set up your Python environment:

```bash
mkdir assistant-test
cd assistant-test
python -m venv .venv
source .venv/bin/activate
pip install openai
```

## Creating an Assistant

Here's how to create an assistant-like experience using chat completions:

```python
from openai import OpenAI
from typing import List, Dict

class LocalAssistant:
    def __init__(self, name: str, instructions: str):
        self.client = OpenAI(
            base_url="http://localhost:39281/v1",
            api_key="not-needed"
        )
        self.name = name
        self.instructions = instructions
        self.conversation_history: List[Dict] = []

    def add_message(self, content: str, role: str = "user") -> str:
        # Add message to history
        self.conversation_history.append({"role": role, "content": content})

        # Prepare messages with system instructions and history
        messages = [
            {"role": "system", "content": self.instructions},
            *self.conversation_history
        ]

        # Get response
        response = self.client.chat.completions.create(
            model="llama3.1:8b-gguf-q4-km",
            messages=messages
        )

        # Add assistant's response to history
        assistant_message = response.choices[0].message.content
        self.conversation_history.append({"role": "assistant", "content": assistant_message})

        return assistant_message

# Create a coding assistant
coding_assistant = LocalAssistant(
    name="Code Buddy",
    instructions="""You are a helpful coding assistant who:
    - Explains concepts with practical examples
    - Provides working code snippets
    - Points out potential pitfalls
    - Keeps responses concise but informative"""
)

# Ask a question
response = coding_assistant.add_message("Can you explain Python list comprehensions with examples?")
print(response)

# Follow-up question (with conversation history maintained)
response = coding_assistant.add_message("Can you show a more complex example with filtering?")
print(response)
```

## Specialized Assistants

You can create different types of assistants by changing the instructions:

```python
# Math tutor assistant
math_tutor = LocalAssistant(
    name="Math Buddy",
    instructions="""You are a patient math tutor who:
    - Breaks down problems step by step
    - Uses clear explanations
    - Provides practice problems
    - Encourages understanding over memorization"""
)

# Writing assistant
writing_assistant = LocalAssistant(
    name="Writing Buddy",
    instructions="""You are a writing assistant who:
    - Helps improve clarity and structure
    - Suggests better word choices
    - Maintains the author's voice
    - Explains the reasoning behind suggestions"""
)
```

## Working with Context

Here's how to create an assistant that can work with context:

```python
class ContextAwareAssistant(LocalAssistant):
    def __init__(self, name: str, instructions: str, context: str):
        super().__init__(name, instructions)
        self.context = context

    def add_message(self, content: str, role: str = "user") -> str:
        # Include context in the system message
        messages = [
            {"role": "system", "content": f"{self.instructions}\n\nContext:\n{self.context}"},
            *self.conversation_history,
            {"role": role, "content": content}
        ]

        response = self.client.chat.completions.create(
            model="llama3.1:8b-gguf-q4-km",
            messages=messages
        )

        assistant_message = response.choices[0].message.content
        self.conversation_history.append({"role": role, "content": content})
        self.conversation_history.append({"role": "assistant", "content": assistant_message})

        return assistant_message

# Example usage with code review context
code_context = """
def calculate_average(numbers):
    total = 0
    for num in numbers:
        total += num
    return total / len(numbers)
"""

code_reviewer = ContextAwareAssistant(
    name="Code Reviewer",
    instructions="You are a helpful code reviewer. Suggest improvements while being constructive.",
    context=code_context
)

response = code_reviewer.add_message("Can you review this code and suggest improvements?")
print(response)
```

## Pro Tips

- Keep the conversation history focused - clear it when starting a new topic
- Use specific instructions to get better responses
- Consider using temperature and max_tokens parameters for different use cases
- Remember that responses are stateless - maintain context yourself

## Memory Management

For longer conversations, you might want to limit the history:

```python
def trim_conversation_history(self, max_messages: int = 10):
    if len(self.conversation_history) > max_messages:
        # Keep system message and last N messages
        self.conversation_history = self.conversation_history[-max_messages:]
```

That's it! While we don't have the full Assistants API yet, we can still create powerful assistant-like
experiences using the chat completions API. The best part? It's all running locally on your machine.
