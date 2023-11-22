import React from "react";

import { Prism as SyntaxHighlighter } from "react-syntax-highlighter";
import theme from "react-syntax-highlighter/dist/esm/styles/prism/dracula";
import useBaseUrl from "@docusaurus/useBaseUrl";

import { useClipboard } from "@site/src/hooks/useClipboard";

import ThemedImage from "@theme/ThemedImage";

export default function OpenAI() {
  const clipboard = useClipboard({ timeout: 200 });

  const codeStringNitro = `curl http://localhost:3928/inferences/llamacpp/chat_completion
  -H "Content-Type: application/json"
  -d '{
    "model": "gpt-3.5-turbo",
    "messages": [
      {
        "role": "system",
        "content": "You are a helpful assistant."
      },
      {
        "role": "user",
        "content": "Who won the world series in 2020?"
      },
    ]
  }'`;

  const codeStringOpenAI = `curl https://api.openai.com/v1/chat/completions
  -H "Content-Type: application/json"
  -H "Authorization: Bearer $OPENAI_API_KEY"
  -d '{
    "model": "gpt-3.5-turbo",
    "messages": [
      {
        "role": "system",
        "content": "You are a helpful assistant."
      },
      {
        "role": "user",
        "content": "Who won the world series in 2020?"
      },
    ]
  }'`;

  return (
    <div className="container">
      <div className="text-center mb-10">
        <h2>OpenAI-Compatible</h2>
        <p className="mt-2">
          Nitro is a drop-in replacement for OpenAI's REST API&nbsp;
        </p>
      </div>
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-8">
        <div className="relative">
          <div className="mb-4">
            <ThemedImage
              alt="App screenshot"
              sources={{
                light: useBaseUrl("/img/logos/open-ai-dark.svg"),
                dark: useBaseUrl("/img/logos/open-ai-light.svg"),
              }}
            />
          </div>
          <div className="group dark:bg-[#27272A]/50 bg-[#F4F4F5] border border-gray-300 dark:border-none py-2 px-4 rounded-md relative">
            <div className="flex gap-x-2 items-center">
              <p className="dark:text-yellow-400 text-yellow-600 font-medium">
                POST
              </p>
              <p className="text-sm">
                https://api.openai.com/v1/chat/completions
              </p>
            </div>
            <div
              className="group-hover:block hidden absolute bottom-2 right-2 text-xs px-2 py-1 rounded-md bg-gray-700 cursor-pointer text-white"
              onClick={() =>
                clipboard.copy("https://api.openai.com/v1/chat/completions")
              }
            >
              {clipboard.copied ? "Copied" : "Copy"}
            </div>
          </div>
          <div className="group [&>pre]:min-h-[400px]">
            <SyntaxHighlighter language="bash" style={theme}>
              {codeStringOpenAI}
            </SyntaxHighlighter>
            <div
              className="group-hover:block hidden absolute top-24 right-2 text-xs px-2 py-1 rounded-md bg-gray-700 cursor-pointer text-white"
              onClick={() => clipboard.copy(codeStringOpenAI)}
            >
              {clipboard.copied ? "Copied" : "Copy"}
            </div>
          </div>
        </div>

        <div className="relative [&>pre]:min-h-[400px]">
          <div className="mb-2 flex items-center space-x-1">
            <img
              src="/img/logos/nitro.svg"
              alt="Element Lines"
              className="w-6 h-6"
            />
            <span className="text-lg font-bold">Nitro</span>
          </div>
          <div className="group dark:bg-[#27272A]/50 bg-[#F4F4F5] border border-gray-300 dark:border-none py-2 px-4 rounded-md relative">
            <div className="flex gap-x-2 items-center">
              <p className="dark:text-yellow-400 text-yellow-600 font-medium">
                POST
              </p>
              <p className="text-sm">
                https://localhost:1337/llama.cpp/v1/chat/completions
              </p>
            </div>
            <div
              className="group-hover:block hidden absolute bottom-2 right-2 text-xs px-2 py-1 rounded-md bg-gray-700 cursor-pointer text-white"
              onClick={() =>
                clipboard.copy(
                  "https://localhost:1337/llama.cpp/v1/chat/completions"
                )
              }
            >
              {clipboard.copied ? "Copied" : "Copy"}
            </div>
          </div>
          <div className="group [&>pre]:min-h-[400px]">
            <SyntaxHighlighter language="bash" style={theme}>
              {codeStringNitro}
            </SyntaxHighlighter>
            <div
              className="group-hover:block hidden absolute top-24 right-2 text-xs px-2 py-1 rounded-md bg-gray-700 cursor-pointer text-white"
              onClick={() => clipboard.copy(codeStringNitro)}
            >
              {clipboard.copied ? "Copied" : "Copy"}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
