#include "normalize_engine.h"
#include "engine_constants.h"

// Define the NormalizeEngine function inside the cortex::engine namespace
namespace cortex::engine {

// Define the NormalizeEngine function
std::string NormalizeEngine(const std::string& engine) {
    if (engine == kLlamaEngine) {
        return kLlamaRepo;
    }
    return engine;
}

} // namespace cortex::engine
