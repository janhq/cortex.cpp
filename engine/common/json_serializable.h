#pragma once

#include <json/value.h>
#include "utils/result.hpp"

struct JsonSerializable {

  virtual cpp::result<Json::Value, std::string> ToJson() = 0;

  virtual ~JsonSerializable() = default;
};
