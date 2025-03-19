#include "gtest/gtest.h"
#include "utils/github_release_utils.h"

class GitHubReleaseUtilsTest : public ::testing::Test {};

TEST_F(GitHubReleaseUtilsTest, AbleToGetReleaseByVersion) {
  auto version{"v0.1.36"};
  auto result = github_release_utils::GetReleaseByVersion(
      kMenloOrg, "cortex.llamacpp", version);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->tag_name, version);
}

TEST_F(GitHubReleaseUtilsTest, AbleToGetReleaseList) {
  auto result = github_release_utils::GetReleases(kMenloOrg, "cortex.llamacpp");

  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->size() > 0);
}
