#include "commands/cortex_upd_cmd.h"
#include "gtest/gtest.h"

namespace {
    constexpr const auto kNewReleaseFolder = "./cortex-release";
    constexpr const auto kNewReleaseFile = "./cortex-release/cortexexe";
    constexpr const auto kCurReleaseFile = "./cortexexe";
    constexpr const auto kTemp = "./cortex_temp";
}

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
  }
};

TEST_F(CortexUpdCmdTest, return_true_if_self_replace) {
    EXPECT_TRUE(commands::ReplaceBinaryInflight("test", "test"));
}

TEST_F(CortexUpdCmdTest, replace_binary_successfully) {
    std::filesystem::path new_binary(kNewReleaseFile);
    std::filesystem::path cur_binary(kCurReleaseFile);
    EXPECT_TRUE(commands::ReplaceBinaryInflight(new_binary, cur_binary));
    EXPECT_TRUE(std::filesystem::exists(kTemp));
}

TEST_F(CortexUpdCmdTest, should_restore_old_binary_if_has_error) {
    std::filesystem::path new_binary("Non-exist");
    std::filesystem::path cur_binary(kCurReleaseFile);
    EXPECT_FALSE(commands::ReplaceBinaryInflight(new_binary, cur_binary));
    EXPECT_FALSE(std::filesystem::exists(kTemp));
}