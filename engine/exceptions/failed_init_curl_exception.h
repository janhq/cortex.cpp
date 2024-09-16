#pragma once

#include <stdexcept>

class FailedInitCurlException : public std::runtime_error {
  constexpr static auto kErrorMessage = "Failed to init CURL";

 public:
  FailedInitCurlException() : std::runtime_error(kErrorMessage) {}
};
