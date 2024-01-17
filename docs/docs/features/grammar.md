---
title: GBNF Grammar
description: What Nitro supports
keywords: [Nitro, Jan, fast inference, inference server, local AI, large language model, OpenAI compatible, open source, llama]
---

## GBNF Grammar

GBNF (GGML BNF) makes it easy to set rules for how a model talks or writes. Think of it like teaching the model to always speak correctly, whether it's in emoji or proper JSON format.

Bakus-Naur Form (BNF) is a way to describe the rules of computer languages, files, and how they talk to each other. GBNF builds on BNF, adding modern features similar to those found in regular expressions.

In GBNF, we create rules (production rules) to guide how a model forms its responses. These rules use a mix of fixed characters (like letters or emojis) and flexible parts that can change. Each rule follows a format: `nonterminal ::= sequence...`.

To get a clearer picture, check out [this guide](https://github.com/ggerganov/llama.cpp/blob/master/grammars/README.md).

## Use GBNF Grammar in Nitro

To make your Nitro model follow specific speaking or writing rules, use this command:

```bash title="Nitro Inference With Grammar" {9}
curl http://localhost:3928/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "messages": [
      {
        "role": "user",
        "content": "Who won the world series in 2020?"
      },
    ],
    "grammar_file": "/path/to/grammarfile"
  }'
```
