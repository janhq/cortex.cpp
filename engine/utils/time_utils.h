#pragma once

#include <chrono>

namespace cortex_utils {
inline auto SecondsSinceEpoch() -> uint32_t {

  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}
}  // namespace cortex_utils
