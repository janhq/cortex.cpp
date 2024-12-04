#pragma once

#include <string>
#include "utils/string_utils.h"

namespace ThreadMessage {
// The entity that produced the message. One of user or assistant.
enum class Role { USER, ASSISTANT };

inline std::string RoleToString(Role role) {
  switch (role) {
    case Role::USER:
      return "user";
    case Role::ASSISTANT:
      return "assistant";
    default:
      throw new std::invalid_argument("Invalid role: " +
                                      std::to_string((int)role));
  }
}

inline Role RoleFromString(const std::string& input) {
  if (string_utils::EqualsIgnoreCase(input, "user")) {
    return Role::USER;
  } else {
    // for backward compatible with jan. Before, jan was mark text with `ready`
    return Role::ASSISTANT;
  }
}
};  // namespace ThreadMessage
