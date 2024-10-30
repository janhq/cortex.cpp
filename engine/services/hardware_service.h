#pragma once
#include <string>
#include <vector>
#include <stdint.h>

namespace services {




struct CPU {
    int cores;
    std::string arch;
    std::string model;
    std::vector<std::string> instructions;
};

struct RAM {
  uint64_t total;
  uint64_t available;
  std::string type;
};

struct RamHelper {

};

struct GPU {

};

struct GPUS {

}; 
class HardwareService {

};
}  // namespace services
