#pragma once
#include <string>
#include <utility>

namespace cortex::cpuid::llamacpp {
std::pair<bool, std::string> IsValidInstructions();
}  // namespace cortex::cpuid::llamacpp