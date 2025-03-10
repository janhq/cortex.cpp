#pragma once
#include <array>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

class CommandExecutor {
 public:
  CommandExecutor(const std::string& command) {
    FILE* pipe = POPEN(command.c_str(), "r");
    if (!pipe) {
      throw std::runtime_error("popen() failed!");
    }
    m_pipe = std::unique_ptr<FILE, decltype(&PCLOSE)>(pipe, PCLOSE);
  }

  CommandExecutor(const CommandExecutor&) = delete;
  CommandExecutor& operator=(const CommandExecutor&) = delete;
  CommandExecutor(CommandExecutor&&) = default;
  CommandExecutor& operator=(CommandExecutor&&) = default;
  ~CommandExecutor() = default;

  std::string execute() {
    if (!m_pipe) {
      throw std::runtime_error("Command not initialized!");
    }

    std::array<char, 128> buffer;
    std::string result;

    while (fgets(buffer.data(), static_cast<int>(buffer.size()),
                 m_pipe.get()) != nullptr) {
      result += buffer.data();
    }

    return result;
  }

 private:
  std::unique_ptr<FILE, decltype(&PCLOSE)> m_pipe{nullptr, PCLOSE};
};
