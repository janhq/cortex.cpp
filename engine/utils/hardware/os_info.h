#pragma once
#include <json/json.h>
#include <string>
#include "common/hardware_common.h"
#include "hwinfo/hwinfo.h"

namespace cortex::hw {

inline OS GetOSInfo() {
  hwinfo::OS os;
  return OS{
      os.name(),                          //name
      os.version(),                       //version
      os.is32bit() ? "32 bit" : "64 bit"  //arch
  };
}
}  // namespace cortex::hw

