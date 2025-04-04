# Output file for the GIF
Output cortex_text_generation.gif

# Set up terminal aesthetics
Set FontSize 30
Set Width 1800
Set Height 900
Set Theme "Catppuccin Frappe"

# Pull the model from Cortex
Type "cortex pull deepseek-r1-distill-qwen-1.5b:1.5b"
Enter
Wait

# Start the model in detached mode
Type "cortex run -d deepseek-r1-distill-qwen-1.5b:1.5b"
Enter
Wait
Sleep 2s

Type "uv venv .venv --python 3.13"
Enter
Sleep 2s

#Type "source .venv/bin/activate"
#Enter
#Sleep 1s

Type "uv pip install rich openai"
Enter
Wait

# Launch Python interactive shell
Type "uv run python"
Enter
Sleep 1s

# Import OpenAI module and create a client instance
Type "from openai import OpenAI"
Enter
Sleep 1s
Type "from rich import print"
Enter
Type "model = 'deepseek-r1-distill-qwen-1.5b:1.5b'"
Enter
Sleep 1s
Type "client = OpenAI(base_url='http://localhost:39281/v1', api_key='not-needed')"
Enter
Sleep 1s

# Send a prompt to generate a short story
Type "response = client.chat.completions.create(model=model, messages=[{'role': 'user', 'content': 'Tell me a short story about a friendly robot.'}])"
Enter
Wait
Sleep 5s

# Print the generated text
Type "print(response.choices[0].message.content)"
Enter
Sleep 5s
Type "print('Thank you for coming to my Ted Talk! 👋')"
Enter
Sleep 1s


# Exit Python shell
Type "exit()"
Enter
Sleep 1s

