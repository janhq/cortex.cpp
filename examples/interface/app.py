import gradio as gr
import os
import time
import json
import requests

url = "http://localhost:3928/inferences/llamacpp/loadmodel"

headers = {
    'Content-Type': 'application/json'
}

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

    dat = {
        "llama_model_path": "nitro/interface/models/zephyr-7b-beta.Q5_K_M.gguf",
        "ctx_len": 2048,
        "ngl": 100,
        "embedding": True,
        "n_parallel": 4,
        "pre_prompt": "A chat between a curious user and an artificial intelligence",
        "user_prompt": "USER: ",
        "ai_prompt": "ASSISTANT: "
    }

    response = requests.post(url, headers = headers, data = json.dumps(dat))

    if response.status_code == 200:
        response_text = response.text
        output = json.loads(response_text)
        final_response = output['response']
    else:
        print("Error: ", response.status_code, response.text)
        
    history[-1][1] = ""
    for character in final_response:
        history[-1][1] += character
        time.sleep(0.05)
        yield history

# Setup the chatbot and input components
with gr.Blocks() as demo:
    chatbot = gr.Chatbot(
        [],
        elem_id="chatbot",
        bubble_full_width=False,
        avatar_images=(None, (os.path.join(os.path.dirname(__file__), "nitro/interface/avatar.png"))),
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
    demo.launch(allowed_paths=["nitro/interface/avatar.png"])
