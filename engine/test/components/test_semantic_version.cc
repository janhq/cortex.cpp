#include <gtest/gtest.h>
#include "utils/semantic_version_utils.h"

class SemanticVersionUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Teardown code if needed
    }
};

// Tests for semantic_version_utils

TEST_F(SemanticVersionUtilsTest, SplitVersion) {
    auto version = semantic_version_utils::SplitVersion("1.2.3");
    EXPECT_EQ(1, version.major);
    EXPECT_EQ(2, version.minor);
    EXPECT_EQ(3, version.patch);
}

TEST_F(SemanticVersionUtilsTest, SplitVersionPartial) {
    auto version = semantic_version_utils::SplitVersion("1.2");
    EXPECT_EQ(1, version.major);
    EXPECT_EQ(2, version.minor);
    EXPECT_EQ(0, version.patch);
}

TEST_F(SemanticVersionUtilsTest, SplitVersionEmpty) {
    auto version = semantic_version_utils::SplitVersion("");
    EXPECT_EQ(0, version.major);
    EXPECT_EQ(0, version.minor);
    EXPECT_EQ(0, version.patch);
}

TEST_F(SemanticVersionUtilsTest, CompareSemanticVersion) {
    EXPECT_EQ(0, semantic_version_utils::CompareSemanticVersion("1.2.3", "1.2.3"));
    EXPECT_EQ(-1, semantic_version_utils::CompareSemanticVersion("1.2.3", "1.2.4"));
    EXPECT_EQ(1, semantic_version_utils::CompareSemanticVersion("1.3.0", "1.2.9"));
    EXPECT_EQ(-1, semantic_version_utils::CompareSemanticVersion("1.9.9", "2.0.0"));
}