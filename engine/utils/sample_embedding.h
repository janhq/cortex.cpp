#pragma once

#include <cmath>
#include <random>
#include <vector>

namespace cortex {

inline std::vector<float> GetEmbedding(const std::string& text) {
  // Use text as seed for reproducible results
  std::seed_seq seed(text.begin(), text.end());
  std::mt19937 gen(seed);
  std::normal_distribution<float> dist(0.0f, 1.0f);

  // Create vector of size 2048 (matching your configuration)
  std::vector<float> embedding(2048);

  // Fill with random values
  for (auto& val : embedding) {
    val = dist(gen);
  }

  // Normalize the vector (important for cosine similarity)
  float norm = 0.0f;
  for (const auto& val : embedding) {
    norm += val * val;
  }
  norm = std::sqrt(norm);

  for (auto& val : embedding) {
    val /= norm;
  }

  return embedding;
}
};  // namespace cortex
