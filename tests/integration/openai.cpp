#include "gtest/gtest.h"
#include <cstdlib>
#include <thread>

namespace {
// Assert Nitro supports all API exposed by OpenAI
// We can leverage existing openAI clients to make requests to Nitro
// e.g: https://github.com/D7EAD/liboai and assert the result
class OpenAI : public testing::Test {
private:
  std::thread nitroShell;

protected:
  virtual ~OpenAI() {}

  void SetUp() override {
    // start nitro in another process
    // TODO: preferably in the same process to support same-process debug
    // TODO: does nitro allow setting up with a light-weight model that is
    // sufficient to run integration test?
    nitroShell = std::thread([]() { int result = system("./nitro"); });
  }

  void TearDown() override {
    // shutdown nitro
    nitroShell.join();
  }
};

TEST_F(OpenAI, healthCheck) {}
TEST_F(OpenAI, chat) {}

TEST_F(OpenAI, image) {}
} // namespace
