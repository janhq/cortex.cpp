#include "commands/cortex_upd_cmd.h"
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

TEST_F(CortexUpdCmdTest, return_true_if_self_replace) {
  EXPECT_TRUE(commands::ReplaceBinaryInflight("test", "test"));
}

TEST_F(CortexUpdCmdTest, replace_binary_successfully) {
  std::filesystem::path new_binary(kNewReleaseFile);
  std::filesystem::path cur_binary(kCurReleaseFile);
#if !defined(_WIN32)
  struct stat cur_file_stat;
  EXPECT_TRUE(stat(cur_binary.string().c_str(), &cur_file_stat) == 0);
#endif

  EXPECT_TRUE(commands::ReplaceBinaryInflight(new_binary, cur_binary));

#if !defined(_WIN32)
  EXPECT_FALSE(std::filesystem::exists(kCortexTemp));

  struct stat new_file_stat;
  EXPECT_TRUE(stat(cur_binary.string().c_str(), &new_file_stat) == 0);
  EXPECT_EQ(cur_file_stat.st_uid, new_file_stat.st_uid);
  EXPECT_EQ(cur_file_stat.st_gid, new_file_stat.st_gid);
  EXPECT_EQ(cur_file_stat.st_mode, new_file_stat.st_mode);
#else
  EXPECT_TRUE(std::filesystem::exists(kCortexTemp));
#endif
}

TEST_F(CortexUpdCmdTest, should_restore_old_binary_if_has_error) {
  std::filesystem::path new_binary("Non-exist");
  std::filesystem::path cur_binary(kCurReleaseFile);
  EXPECT_FALSE(commands::ReplaceBinaryInflight(new_binary, cur_binary));
  EXPECT_FALSE(std::filesystem::exists(kCortexTemp));
}