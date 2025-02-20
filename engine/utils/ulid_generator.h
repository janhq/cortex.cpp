#pragma once

#include <string>
#include "utils/ulid/ulid.hh"

namespace ulid {
inline std::string GenerateUlid() {
  auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count();

  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<unsigned int> dis(0, 255);
  auto ulid = ulid::Create(
      millisecs, [&dis, &gen]() { return static_cast<uint8_t>(dis(gen)); });
  return ulid::Marshal(ulid);
}
}  // namespace ulid
