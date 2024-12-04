#pragma once

#include <string>
#include "utils/string_utils.h"

namespace ThreadMessage {
// The status of the message, which can be either in_progress, incomplete, or completed.
enum class Status { IN_PROGRESS, INCOMPLETE, COMPLETED };

// Convert a Status enum to a string.
inline std::string StatusToString(Status status) {
  switch (status) {
    case Status::IN_PROGRESS:
      return "in_progress";
    case Status::INCOMPLETE:
      return "incomplete";
    // default as completed for backward compatible with jan
    default:
      return "completed";
  }
}

// Convert a string to a Status enum.
inline Status StatusFromString(const std::string& input) {
  if (string_utils::EqualsIgnoreCase(input, "in_progress")) {
    return Status::IN_PROGRESS;
  } else if (string_utils::EqualsIgnoreCase(input, "incomplete")) {
    return Status::INCOMPLETE;
  } else {
    // for backward compatible with jan. Before, jan was mark text with `ready`
    return Status::COMPLETED;
  }
}
};  // namespace ThreadMessage
