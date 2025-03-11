#include "normalize_engine.h"
#include "engine_constants.h"

namespace cortex::engine {

std::string NormalizeEngine(const std::string& engine) {
  if (engine == kLlamaEngine) {
    return kLlamaRepo;
  }
  return engine;
}

}  // namespace cortex::engine