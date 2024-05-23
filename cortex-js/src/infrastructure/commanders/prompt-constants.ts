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

export const LLAMA_3_JINJA = `{% set loop_messages = messages %}{% for message in loop_messages %}{% set content = '<|start_header_id|>' + message['role'] + '<|end_header_id|>

'+ message['content'] | trim + '<|eot_id|>' %}{% if loop.index0 == 0 %}{% set content = bos_token + content %}{% endif %}{{ content }}{% endfor %}{% if add_generation_prompt %}{{ '<|start_header_id|>assistant<|end_header_id|>

' }}{% endif %}`;

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
export const LLAMA_3 =
  '<|begin_of_text|><|start_header_id|>system<|end_header_id|>\n\n{system_message}<|eot_id|><|start_header_id|>user<|end_header_id|>\n\n{prompt}<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n';
