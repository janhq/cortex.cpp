/*
 * This file contains code derived from the llama.cpp project.
 * Original project: https://github.com/ggerganov/llama.cpp
 *
 * Original work Copyright (c) 2023 Georgi Gerganov
 * Modified work Copyright (c) 2024 [Homebrew.ltd]
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 *
 * This file incorporates work covered by the above copyright and permission notice.
 * Any modifications made to this file are covered under the copyright of the modifying party.
 *
 * Modifications:
 * [Brief description of modifications made to the original code, if any]
 *
 * For more information about the llama.cpp project and its license, please visit:
 * https://github.com/ggerganov/llama.cpp/blob/master/LICENSE
 */

//
// chat templates
//

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>
namespace config {

#if (defined(_MSC_VER) && _MSC_VER >= 1900 && defined(__cpp_char8_t)) || __cplusplus >= 202002L
    #define LU8(x) reinterpret_cast<const char*>(u8##x)
#else
    #define LU8(x) u8##x
#endif

typedef struct llama_chat_message {
  const char* role;
  const char* content;
} llama_chat_message;

struct llama_chat_msg {
  std::string role;
  std::string content;
};

static std::string trim(const std::string& str) {
  size_t start = 0;
  size_t end = str.size();
  while (start < end && isspace(str[start])) {
    start += 1;
  }
  while (end > start && isspace(str[end - 1])) {
    end -= 1;
  }
  return str.substr(start, end - start);
}
// Simple version of "llama_apply_chat_template" that only works with strings
// This function uses heuristic checks to determine commonly used template. It is not a jinja parser.
static int32_t llama_chat_apply_template_internal(
    const std::string& tmpl, const std::vector<const llama_chat_message*>& chat,
    std::string& dest, bool add_ass) {
  // Taken from the research: https://github.com/ggerganov/llama.cpp/issues/5527
  std::stringstream ss;
  auto tmpl_contains = [&tmpl](std::string haystack) -> bool {
    return tmpl.find(haystack) != std::string::npos;
  };
  if (tmpl == "chatml" || tmpl_contains("<|im_start|>")) {
    // chatml template
    for (auto message : chat) {
      ss << "<|im_start|>" << message->role << "\n"
         << message->content << "<|im_end|>\n";
    }
    if (add_ass) {
      ss << "<|im_start|>assistant\n";
    }
  } else if (tmpl == "llama2" || tmpl == "mistral" || tmpl_contains("[INST]")) {
    // llama2 template and its variants
    // [variant] support system message
    bool support_system_message = tmpl_contains("<<SYS>>") || tmpl == "mistral";
    // [variant] space before + after response
    bool space_around_response = tmpl_contains("' ' + eos_token");
    // [variant] add BOS inside history
    bool add_bos_inside_history = tmpl_contains("bos_token + '[INST]");
    // [variant] trim spaces from the input message
    bool strip_message = tmpl_contains("content.strip()");
    // construct the prompt
    bool is_inside_turn = true;  // skip BOS at the beginning
    ss << "[INST] ";
    for (auto message : chat) {
      std::string content =
          strip_message ? trim(message->content) : message->content;
      std::string role(message->role);
      if (!is_inside_turn) {
        is_inside_turn = true;
        ss << (add_bos_inside_history ? "<s>[INST] " : "[INST] ");
      }
      if (role == "system") {
        if (support_system_message) {
          ss << "<<SYS>>\n" << content << "\n<</SYS>>\n\n";
        } else {
          // if the model does not support system message, we still include it in the first message, but without <<SYS>>
          ss << content << "\n";
        }
      } else if (role == "user") {
        ss << content << " [/INST]";
      } else {
        ss << (space_around_response ? " " : "") << content
           << (space_around_response ? " " : "") << "</s>";
        is_inside_turn = false;
      }
    }
    // llama2 templates seem to not care about "add_generation_prompt"
  } else if (tmpl == "phi3" ||
             (tmpl_contains("<|assistant|>") && tmpl_contains("<|end|>"))) {
    // Phi 3
    for (auto message : chat) {
      std::string role(message->role);
      ss << "<|" << role << "|>\n" << message->content << "<|end|>\n";
    }
    if (add_ass) {
      ss << "<|assistant|>\n";
    }
  } else if (tmpl == "zephyr" || tmpl_contains("<|user|>")) {
    // zephyr template
    for (auto message : chat) {
      ss << "<|" << message->role << "|>" << "\n"
         << message->content << "<|endoftext|>\n";
    }
    if (add_ass) {
      ss << "<|assistant|>\n";
    }
  } else if (tmpl == "monarch" ||
             tmpl_contains("bos_token + message['role']")) {
    // mlabonne/AlphaMonarch-7B template (the <s> is included inside history)
    for (auto message : chat) {
      std::string bos =
          (message == chat.front()) ? "" : "<s>";  // skip BOS for first message
      ss << bos << message->role << "\n" << message->content << "</s>\n";
    }
    if (add_ass) {
      ss << "<s>assistant\n";
    }
  } else if (tmpl == "gemma" || tmpl == "gemma2" ||
             tmpl_contains("<start_of_turn>")) {
    // google/gemma-7b-it
    std::string system_prompt = "";
    for (auto message : chat) {
      std::string role(message->role);
      if (role == "system") {
        // there is no system message for gemma, but we will merge it with user prompt, so nothing is broken
        system_prompt = trim(message->content);
        continue;
      }
      // in gemma, "assistant" is "model"
      role = role == "assistant" ? "model" : message->role;
      ss << "<start_of_turn>" << role << "\n";
      if (!system_prompt.empty() && role != "model") {
        ss << system_prompt << "\n\n";
        system_prompt = "";
      }
      ss << trim(message->content) << "<end_of_turn>\n";
    }
    if (add_ass) {
      ss << "<start_of_turn>model\n";
    }
  } else if (tmpl == "orion" ||
             tmpl_contains("'\\n\\nAssistant: ' + eos_token")) {
    // OrionStarAI/Orion-14B-Chat
    std::string system_prompt = "";
    for (auto message : chat) {
      std::string role(message->role);
      if (role == "system") {
        // there is no system message support, we will merge it with user prompt
        system_prompt = message->content;
        continue;
      } else if (role == "user") {
        ss << "Human: ";
        if (!system_prompt.empty()) {
          ss << system_prompt << "\n\n";
          system_prompt = "";
        }
        ss << message->content << "\n\nAssistant: </s>";
      } else {
        ss << message->content << "</s>";
      }
    }
  } else if (tmpl == "openchat" || tmpl_contains("GPT4 Correct ")) {
    // openchat/openchat-3.5-0106,
    for (auto message : chat) {
      std::string role(message->role);
      if (role == "system") {
        ss << message->content << "<|end_of_turn|>";
      } else {
        role[0] = toupper(role[0]);
        ss << "GPT4 Correct " << role << ": " << message->content
           << "<|end_of_turn|>";
      }
    }
    if (add_ass) {
      ss << "GPT4 Correct Assistant:";
    }
  } else if (tmpl == "vicuna" || tmpl == "vicuna-orca" ||
             (tmpl_contains("USER: ") && tmpl_contains("ASSISTANT: "))) {
    // eachadea/vicuna-13b-1.1 (and Orca variant)
    for (auto message : chat) {
      std::string role(message->role);
      if (role == "system") {
        // Orca-Vicuna variant uses a system prefix
        if (tmpl == "vicuna-orca" || tmpl_contains("SYSTEM: ")) {
          ss << "SYSTEM: " << message->content << "\n";
        } else {
          ss << message->content << "\n\n";
        }
      } else if (role == "user") {
        ss << "USER: " << message->content << "\n";
      } else if (role == "assistant") {
        ss << "ASSISTANT: " << message->content << "</s>\n";
      }
    }
    if (add_ass) {
      ss << "ASSISTANT:";
    }
  } else if (tmpl == "deepseek" ||
             (tmpl_contains("### Instruction:") && tmpl_contains("<|EOT|>"))) {
    // deepseek-ai/deepseek-coder-33b-instruct
    for (auto message : chat) {
      std::string role(message->role);
      if (role == "system") {
        ss << message->content;
      } else if (role == "user") {
        ss << "### Instruction:\n" << message->content << "\n";
      } else if (role == "assistant") {
        ss << "### Response:\n" << message->content << "\n<|EOT|>\n";
      }
    }
    if (add_ass) {
      ss << "### Response:\n";
    }
  } else if (tmpl == "command-r" || (tmpl_contains("<|START_OF_TURN_TOKEN|>") &&
                                     tmpl_contains("<|USER_TOKEN|>"))) {
    // CohereForAI/c4ai-command-r-plus
    for (auto message : chat) {
      std::string role(message->role);
      if (role == "system") {
        ss << "<|START_OF_TURN_TOKEN|><|SYSTEM_TOKEN|>"
           << trim(message->content) << "<|END_OF_TURN_TOKEN|>";
      } else if (role == "user") {
        ss << "<|START_OF_TURN_TOKEN|><|USER_TOKEN|>" << trim(message->content)
           << "<|END_OF_TURN_TOKEN|>";
      } else if (role == "assistant") {
        ss << "<|START_OF_TURN_TOKEN|><|CHATBOT_TOKEN|>"
           << trim(message->content) << "<|END_OF_TURN_TOKEN|>";
      }
    }
    if (add_ass) {
      ss << "<|START_OF_TURN_TOKEN|><|CHATBOT_TOKEN|>";
    }
  } else if (tmpl == "llama3" || (tmpl_contains("<|start_header_id|>") &&
                                  tmpl_contains("<|end_header_id|>"))) {
    // Llama 3
    for (auto message : chat) {
      std::string role(message->role);
      ss << "<|start_header_id|>" << role << "<|end_header_id|>\n\n"
         << trim(message->content) << "<|eot_id|>";
    }
    if (add_ass) {
      ss << "<|start_header_id|>assistant<|end_header_id|>\n\n";
    }
  } else if (tmpl == "chatglm3" || tmpl_contains("[gMASK]sop")) {
    // chatglm3-6b
    ss << "[gMASK]" << "sop";
    for (auto message : chat) {
      std::string role(message->role);
      ss << "<|" << role << "|>" << "\n " << message->content;
    }
    if (add_ass) {
      ss << "<|assistant|>";
    }
  } else if (tmpl == "chatglm4" || tmpl_contains("[gMASK]<sop>")) {
    ss << "[gMASK]" << "<sop>";
    for (auto message : chat) {
      std::string role(message->role);
      ss << "<|" << role << "|>" << "\n" << message->content;
    }
    if (add_ass) {
      ss << "<|assistant|>";
    }
  } else if (tmpl == "minicpm" || tmpl_contains(LU8("<用户>"))) {
    // MiniCPM-3B-OpenHermes-2.5-v2-GGUF
    for (auto message : chat) {
      std::string role(message->role);
      if (role == "user") {
        ss << LU8("<用户>");
        ss << trim(message->content);
        ss << "<AI>";
      } else {
        ss << trim(message->content);
      }
    }
  } else if (tmpl == "deepseek2" ||
             tmpl_contains("'Assistant: ' + message['content'] + eos_token")) {
    // DeepSeek-V2
    for (auto message : chat) {
      std::string role(message->role);
      if (role == "system") {
        ss << message->content << "\n\n";
      } else if (role == "user") {
        ss << "User: " << message->content << "\n\n";
      } else if (role == "assistant") {
        ss << "Assistant: " << message->content << LU8("<｜end▁of▁sentence｜>");
      }
    }
    if (add_ass) {
      ss << "Assistant:";
    }
  } else if (tmpl == "exaone3" ||
             (tmpl_contains("[|system|]") && tmpl_contains("[|assistant|]") &&
              tmpl_contains("[|endofturn|]"))) {
    // ref: https://huggingface.co/LGAI-EXAONE/EXAONE-3.0-7.8B-Instruct/discussions/8#66bae61b1893d14ee8ed85bb
    // EXAONE-3.0-7.8B-Instruct
    for (auto message : chat) {
      std::string role(message->role);
      if (role == "system") {
        ss << "[|system|]" << trim(message->content) << "[|endofturn|]\n";
      } else if (role == "user") {
        ss << "[|user|]" << trim(message->content) << "\n";
      } else if (role == "assistant") {
        ss << "[|assistant|]" << trim(message->content) << "[|endofturn|]\n";
      }
    }
    if (add_ass) {
      ss << "[|assistant|]";
    }
  } else {
    // template not supported
    return -1;
  }
  dest = ss.str();
  return dest.size();
}

int32_t llama_chat_apply_template(const char* tmpl,
                                  const struct llama_chat_message* chat,
                                  size_t n_msg, bool add_ass, char* buf,
                                  int32_t length) {
  std::string curr_tmpl(tmpl == nullptr ? "" : tmpl);

  // format the chat to string
  std::vector<const llama_chat_message*> chat_vec;
  chat_vec.resize(n_msg);
  for (size_t i = 0; i < n_msg; i++) {
    chat_vec[i] = &chat[i];
  }

  std::string formatted_chat;
  int32_t res = llama_chat_apply_template_internal(curr_tmpl, chat_vec,
                                                   formatted_chat, add_ass);
  if (res < 0) {
    return res;
  }
  if (buf && length > 0) {
    strncpy(buf, formatted_chat.c_str(), length);
  }
  return res;
}

std::string llama_chat_apply_template(const std::string& tmpl,
                                      const std::vector<llama_chat_msg>& msgs,
                                      bool add_ass) {
  int alloc_size = 0;
  bool fallback = false;  // indicate if we must fallback to default chatml
  std::vector<llama_chat_message> chat;
  for (auto& msg : msgs) {
    chat.push_back({msg.role.c_str(), msg.content.c_str()});
    alloc_size += (msg.role.size() + msg.content.size()) * 1.25;
  }

  const char* ptr_tmpl = tmpl.empty() ? nullptr : tmpl.c_str();
  std::vector<char> buf(alloc_size);

  // run the first time to get the total output length
  int32_t res = llama_chat_apply_template(ptr_tmpl, chat.data(), chat.size(),
                                          add_ass, buf.data(), buf.size());

  // error: chat template is not supported
  if (res < 0) {
    if (ptr_tmpl != nullptr) {
      // if the custom "tmpl" is not supported, we throw an error
      // this is a bit redundant (for good), since we're not sure if user validated the custom template with llama_chat_verify_template()
      throw std::runtime_error("this custom template is not supported");
    } else {
      // If the built-in template is not supported, we default to chatml
      res = llama_chat_apply_template("chatml", chat.data(), chat.size(),
                                      add_ass, buf.data(), buf.size());
      fallback = true;
    }
  }

  // if it turns out that our buffer is too small, we resize it
  if ((size_t)res > buf.size()) {
    buf.resize(res);
    res =
        llama_chat_apply_template(fallback ? "chatml" : ptr_tmpl, chat.data(),
                                  chat.size(), add_ass, buf.data(), buf.size());
  }

  std::string formatted_chat(buf.data(), res);
  return formatted_chat;
}
}  // namespace config