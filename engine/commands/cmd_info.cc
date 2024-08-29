#include "cmd_info.h"
#include <vector>
#include "trantor/utils/Logger.h"

namespace commands {
namespace {
constexpr const char* kDelimiter = ":";

std::vector<std::string> split(std::string& s, const std::string& delimiter) {
  std::vector<std::string> tokens;
  size_t pos = 0;
  std::string token;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    token = s.substr(0, pos);
    tokens.push_back(token);
    s.erase(0, pos + delimiter.length());
  }
  tokens.push_back(s);

  return tokens;
}
}  // namespace

CmdInfo::CmdInfo(std::string model_id) {
  Parse(std::move(model_id));
}

void CmdInfo::Parse(std::string model_id) {
  if (model_id.find(kDelimiter) == std::string::npos) {
    engine_name = "cortex.llamacpp";
    model_name = std::move(model_id);
    branch = "main";
  } else {
    auto res = split(model_id, kDelimiter);
    if (res.size() != 2) {
      LOG_ERROR << "model_id does not valid";
      return;
    } else {
      model_name = std::move(res[0]);
      branch = std::move(res[1]);
      if (branch.find("onnx") != std::string::npos) {
        engine_name = "cortex.onnx";
      } else if (branch.find("tensorrt") != std::string::npos) {
        engine_name = "cortex.tensorrt-llm";
      } else if (branch.find("gguf") != std::string::npos) {
        engine_name = "cortex.llamacpp";
      } else {
        LOG_ERROR << "Not a valid branch model_name " << branch;
      }
    }
  }
}

}  // namespace commands