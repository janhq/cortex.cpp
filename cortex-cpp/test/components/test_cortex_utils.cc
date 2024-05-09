#include "gtest/gtest.h"
#include "utils/cortex_utils.h"

class NitroUtilTest : public ::testing::Test {
};

TEST_F(NitroUtilTest, left_trim) {
    {
        std::string empty;
        cortex_utils::ltrim(empty);
        EXPECT_EQ(empty, "");
    }

    {
        std::string s = "abc";
        std::string expected = "abc";
        cortex_utils::ltrim(s);
        EXPECT_EQ(s, expected);
    }

    {
        std::string s = " abc";
        std::string expected = "abc";
        cortex_utils::ltrim(s);
        EXPECT_EQ(s, expected);
    }

    {
        std::string s = "1 abc 2 ";
        std::string expected = "1 abc 2 ";
        cortex_utils::ltrim(s);
        EXPECT_EQ(s, expected);
    }

    {
        std::string s = " |abc";
        std::string expected = "|abc";
        cortex_utils::ltrim(s);
        EXPECT_EQ(s, expected);
    }
}
