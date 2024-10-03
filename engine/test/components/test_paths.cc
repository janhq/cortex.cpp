#include "gtest/gtest.h"
#include "utils/file_manager_utils.h"

namespace file_manager_utils {
// Test suite
class PathTests : public ::testing::Test {};

// Test cases for GetAbsolutePath
TEST_F(PathTests, GetAbsolutePath_AbsolutePath_ReturnsSamePath) {
  auto base = std::filesystem::path("/base");
  auto relative = std::filesystem::path("/absolute/path");
  EXPECT_EQ(GetAbsolutePath(base, relative), relative);
}

TEST_F(PathTests, GetAbsolutePath_RelativePath_ReturnsCombinedPath) {
  auto base = std::filesystem::path("/base");
  auto relative = std::filesystem::path("relative/path");
  EXPECT_EQ(GetAbsolutePath(base, relative), base / relative);
}

// Test cases for IsSubpath
TEST_F(PathTests, IsSubpath_Subpath_ReturnsTrue) {
  auto base = std::filesystem::path("/base");
  auto subpath = std::filesystem::path("/base/relative/path");
  EXPECT_TRUE(IsSubpath(base, subpath));
}

TEST_F(PathTests, IsSubpath_NotSubpath_ReturnsFalse) {
  auto base = std::filesystem::path("/base");
  auto notSubpath = std::filesystem::path("/other/path");
  EXPECT_FALSE(IsSubpath(base, notSubpath));
}

TEST_F(PathTests, IsSubpath_EmptyRelative_ReturnsFalse) {
  auto base = std::filesystem::path("/base");
  auto emptyPath = std::filesystem::path("/base/");
  EXPECT_FALSE(IsSubpath(base, emptyPath));
}

// Test cases for Subtract
TEST_F(PathTests, Subtract_SubtractingBaseFromSubPath_ReturnsRelativePath) {
  auto base = std::filesystem::path("/base");
  auto subPath = std::filesystem::path("/base/relative/path");
  EXPECT_EQ(Subtract(subPath, base), std::filesystem::path("relative/path"));
}

TEST_F(PathTests, Subtract_NotASubPath_ReturnsOriginalPath) {
  auto base = std::filesystem::path("/base");
  auto notSubPath = std::filesystem::path("/other/path");
  EXPECT_EQ(Subtract(notSubPath, base), notSubPath);
}

TEST_F(PathTests, Subtract_IdenticalPaths_ReturnsEmptyRelative) {
  auto base = std::filesystem::path("/base");
  EXPECT_EQ(Subtract(base, base), std::filesystem::path("."));
}
}  // namespace file_manager_utils