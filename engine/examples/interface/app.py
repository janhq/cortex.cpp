import gradio as gr
import os
import time
import json
import requests

# URLs for model loading and chat completion
load_model_url = "http://localhost:3928/inferences/server/loadmodel"
chat_completion_url = "http://localhost:3928/inferences/server/chat_completion"

headers = {
    'Content-Type': 'application/json'
}

# Function to load the model
def load_model():
    load_data = {
        "llama_model_path": "cortex-cpp/model/llama-2-7b-chat.Q5_K_M.gguf?download=true"
        # Add other necessary parameters if required
    }
    response = requests.post(load_model_url, headers=headers, data=json.dumps(load_data))
    if response.status_code != 200:
        print("Error in loading model: ", response.status_code, response.text)
    else:
        print("Model loaded successfully.")

# Define the vote function for like/dislike functionality
def vote(data: gr.LikeData):
    if data.liked:
        print("You upvoted this response: " + data.value)
    else:
        print("You downvoted this response: " + data.value)

# Function to handle text input
def add_text(history, text):
    return history + [(text, None)]

# Function to handle file input
def add_file(history, file):
    return history + [((file.name,), None)]

# Bot response function with markdown support and like/dislike feature
def bot(history):
    last_message = history[-1][0] if history else ""
    dat = {
        "llama_model_path": "cortex-cpp/model/llama-2-7b-chat.Q5_K_M.gguf?download=true",
        "messages": [
            {
                "role": "user",
                "content": last_message
            },
        ]
    }

    response = requests.post(chat_completion_url, headers=headers, data=json.dumps(dat))

    if response.status_code == 200:
        response_text = response.text
        output = json.loads(response_text)
        final_response = output['response']
    else:
        print("Error: ", response.status_code, response.text)
        
    history[-1][1] = final_response
    yield history

# Load the model at the start
load_model()

# Setup the chatbot and input components
with gr.Blocks() as demo:
    chatbot = gr.Chatbot(
        [],
        elem_id="chatbot",
        bubble_full_width=False,
        avatar_images=(None, (os.path.join(os.path.dirname(__file__), "cortex-cpp/example/avatar.png"))),
    )

    with gr.Row():
        txt = gr.Textbox(scale=4, show_label=False, placeholder="Enter text and press enter, or upload an image", container=False)
        btn = gr.UploadButton("📁", file_types=["image", "video", "audio"])

    txt_msg = txt.submit(add_text, [chatbot, txt], [chatbot], queue=False).then(bot, chatbot, chatbot, api_name="bot_response")
    file_msg = btn.upload(add_file, [chatbot, btn], [chatbot], queue=False).then(bot, chatbot, chatbot)

    # Attach the like/dislike functionality to the chatbot
    chatbot.like(vote, None, None)

# Launch the application
if __name__ == "__main__":
    demo.queue()
    demo.launch(allowed_paths=["cortex-cpp/example/avatar.png"])