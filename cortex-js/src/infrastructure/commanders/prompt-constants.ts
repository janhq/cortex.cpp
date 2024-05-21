//// HF Chat template
export const OPEN_CHAT_3_5_JINJA = `{{ bos_token }}{% for message in messages %}{{ 'GPT4 Correct ' + message['role'].title() + ': ' + message['content'] + '<|end_of_turn|>'}}{% endfor %}{% if add_generation_prompt %}{{ 'GPT4 Correct Assistant:' }}{% endif %}`;

export const ZEPHYR_JINJA = `{% for message in messages %}
{% if message['role'] == 'user' %}
{{ '<|user|>
' + message['content'] + eos_token }}
{% elif message['role'] == 'system' %}
{{ '<|system|>
' + message['content'] + eos_token }}
{% elif message['role'] == 'assistant' %}
{{ '<|assistant|>
'  + message['content'] + eos_token }}
{% endif %}
{% if loop.last and add_generation_prompt %}
{{ '<|assistant|>' }}
{% endif %}
{% endfor %}`;

//// Corresponding prompt template
export const OPEN_CHAT_3_5 = `GPT4 Correct User: {prompt}<|end_of_turn|>GPT4 Correct Assistant:`;

export const ZEPHYR = `<|system|>
{system_message}</s>
<|user|>
{prompt}</s>
<|assistant|>
`;

export const COMMAND_R = `<BOS_TOKEN><|START_OF_TURN_TOKEN|><|SYSTEM_TOKEN|>{system}<|END_OF_TURN_TOKEN|><|START_OF_TURN_TOKEN|><|USER_TOKEN|>{prompt}<|END_OF_TURN_TOKEN|><|START_OF_TURN_TOKEN|><|CHATBOT_TOKEN|>{response}
`;

// getting from https://huggingface.co/TheBloke/Llama-2-70B-Chat-GGUF
export const LLAMA_2 = `[INST] <<SYS>>
{system_message}
<</SYS>>
{prompt}[/INST]`;
