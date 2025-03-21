#include "gtest/gtest.h"
#include "utils/github_release_utils.h"

class GitHubReleaseUtilsTest : public ::testing::Test {};

TEST_F(GitHubReleaseUtilsTest, AbleToGetReleaseByVersion) {
  auto version{"b4920"};
  auto result = github_release_utils::GetReleaseByVersion(
      kMenloOrg, "llama.cpp", version);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->tag_name, version);
}

TEST_F(GitHubReleaseUtilsTest, AbleToGetReleaseList) {
  auto result = github_release_utils::GetReleases(kMenloOrg, "llama.cpp");

  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->size() > 0);
}
