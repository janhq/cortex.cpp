#include <filesystem>
#include <fstream>
#include "gtest/gtest.h"

namespace {
constexpr const auto kNewReleaseFolder = "./cortex-release";
constexpr const auto kNewReleaseFile = "./cortex-release/cortexexe";
constexpr const auto kCurReleaseFile = "./cortexexe";
constexpr const auto kCortexTemp = "./cortex_temp";
}  // namespace

class CortexUpdCmdTest : public ::testing::Test {
  void SetUp() {
    // Create new release folder and files
    std::filesystem::path folder_path(kNewReleaseFolder);
    std::filesystem::create_directory(folder_path);
    std::ofstream src(kNewReleaseFile);
    src.close();
    std::ofstream dst(kCurReleaseFile);
    dst.close();
  }

  void TearDown() {
    std::filesystem::path folder_path(kNewReleaseFolder);
    if (std::filesystem::exists(folder_path)) {
      std::filesystem::remove_all(folder_path);
    }

    if (std::filesystem::exists(kCurReleaseFile)) {
      std::filesystem::remove(kCurReleaseFile);
    }

    if (std::filesystem::exists(kCortexTemp)) {
      std::filesystem::remove(kCortexTemp);
    }
  }
};
