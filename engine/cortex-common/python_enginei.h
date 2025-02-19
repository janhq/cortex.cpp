#pragma once

#include <functional>
#include <memory>

#include "json/value.h"

class PythonEngineI {
 public:
  virtual ~PythonEngineI() {}

  virtual bool IsSupported(const std::string& f) = 0;

  virtual void ExecutePythonFile(std::string binary_execute_path,
                                 std::string file_execution_path,
                                 std::string python_library_path) = 0;

  virtual void HandlePythonFileExecutionRequest(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
};
