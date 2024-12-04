#pragma once

#include <string>
#include <unordered_map>
#include "json/json.h"

namespace commands {

class ModelStartCmd {
 public:
  bool Exec(const std::string& host, int port, const std::string& model_handle,
            const std::unordered_map<std::string, std::string>& options,
            bool print_success_log = true);
    private:
  bool UpdateConfig(Json::Value& data, const std::string& key,
                    const std::string& value);
};
}  // namespace commands
