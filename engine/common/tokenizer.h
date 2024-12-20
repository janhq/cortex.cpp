#pragma once

#include <sstream>
#include <string>

struct Tokenizer {
  std::string eos_token = "";
  bool add_eos_token = true;

  std::string bos_token = "";
  bool add_bos_token = true;

  std::string unknown_token = "";
  std::string padding_token = "";

  std::string chat_template = "";

  bool add_generation_prompt = true;

  // Helper function for common fields
  std::string BaseToString() const {
    std::ostringstream ss;
    ss << "eos_token: \"" << eos_token << "\"\n"
       << "add_eos_token: " << (add_eos_token ? "true" : "false") << "\n"
       << "bos_token: \"" << bos_token << "\"\n"
       << "add_bos_token: " << (add_bos_token ? "true" : "false") << "\n"
       << "unknown_token: \"" << unknown_token << "\"\n"
       << "padding_token: \"" << padding_token << "\"\n"
       << "chat_template: \"" << chat_template << "\"\n"
       << "add_generation_prompt: "
       << (add_generation_prompt ? "true" : "false") << "\"";
    return ss.str();
  }

  virtual ~Tokenizer() = default;

  virtual std::string ToString() = 0;
};

struct GgufTokenizer : public Tokenizer {
  std::string pre = "";

  ~GgufTokenizer() override = default;

  std::string ToString() override {
    std::ostringstream ss;
    ss << "GgufTokenizer {\n";
    // Add base class members
    ss << BaseToString() << "\n";
    // Add derived class members
    ss << "pre: \"" << pre << "\"\n";
    ss << "}";
    return ss.str();
  }
};

struct SafeTensorTokenizer : public Tokenizer {
  bool add_prefix_space = true;

  ~SafeTensorTokenizer() = default;

  std::string ToString() override {
    std::ostringstream ss;
    ss << "SafeTensorTokenizer {\n";
    // Add base class members
    ss << BaseToString() << "\n";
    // Add derived class members
    ss << "add_prefix_space: " << (add_prefix_space ? "true" : "false") << "\n";
    ss << "}";
    return ss.str();
  }
};
