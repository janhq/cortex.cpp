#include "nitro/controllers/llamaCPP.h"
#include "gtest/gtest.h"
#include <cstdlib>
#include <thread>

namespace {

TEST(llamaCPP, base64_decode) {
  std::string encoded = "SGVsbG8gV29ybGQ="; // This is 'Hello World' in Base64
  // Expected decoded result (byte representation of 'Hello World')
  std::vector<uint8_t> expected = {'H', 'e', 'l', 'l', 'o', ' ',
                                   'W', 'o', 'r', 'l', 'd'};

  // Call the function
  std::vector<uint8_t> result = base64_decode(encoded);

  // Assert that the decoded result matches the expected value
  EXPECT_EQ(result, expected);
}

} // namespace
