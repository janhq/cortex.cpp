#pragma once

#include <string>
#include "common/message_content.h"
#include "common/message_role.h"

namespace OpenAi {
struct MessageDelta {
  struct Delta {
    Role role;

    std::vector<std::unique_ptr<Content>> content;
  };

  /**
   * The identifier of the message, which can be referenced in API endpoints.
   */
  std::string id;

  /**
   * The object type, which is always thread.message.delta.
   */
  std::string object{"thread.message.delta"};

  /**
   * The delta containing the fields that have changed on the Message.
   */
  Delta delta;
};
}  // namespace OpenAi
