#pragma once

#include <cstdint>

namespace OpenAi {
struct RunUsage {
  uint64_t completion_tokens;

  uint64_t prompt_tokens;

  uint64_t total_tokens;
};
}  // namespace OpenAi
