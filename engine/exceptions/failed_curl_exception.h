#pragma once

#include <stdexcept>

class FailedCurlException : public std::runtime_error {

 public:
  FailedCurlException(const std::string& message)
      : std::runtime_error(message) {}
};
