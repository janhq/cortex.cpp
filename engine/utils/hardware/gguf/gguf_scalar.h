#pragma once
#include <cstdint>
#include <cfloat> 
namespace hardware {
// GGUFBytesScalar is the scalar for bytes.
using GGUFBytesScalar = uint64_t;

// GGUFParametersScalar is the scalar for parameters.
using GGUFParametersScalar = uint64_t;

// GGUFBitsPerWeightScalar is the scalar for bits per weight.
using GGUFBitsPerWeightScalar = double;

// GGUFTokensPerSecondScalar is the scalar for tokens per second.
using GGUFTokensPerSecondScalar = double;
}