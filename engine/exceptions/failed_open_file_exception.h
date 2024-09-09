#pragma once

#include <stdexcept>

class FailedOpenFileException : public std::runtime_error {

 public:
  FailedOpenFileException(const std::string& message)
      : std::runtime_error(message) {}
};
