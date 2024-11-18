#pragma once

#include <stdexcept>

class MalformedUrlException : public std::runtime_error {

 public:
  MalformedUrlException(const std::string& message)
      : std::runtime_error(message) {}
};
