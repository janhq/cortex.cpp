#include "gtest/gtest.h"
#include "utils/url_parser.h"

class UrlParserTestSuite : public ::testing::Test {
 protected:
  constexpr static auto kValidUrlWithOnlyPaths{"https://jan.ai/path1/path2"};
};

TEST_F(UrlParserTestSuite, TestParseUrlCorrectly) {
  auto url = url_parser::FromUrlString(kValidUrlWithOnlyPaths);

  EXPECT_EQ(url.host, "jan.ai");
  EXPECT_EQ(url.protocol, "https");
  EXPECT_EQ(url.pathParams.size(), 2);
}

TEST_F(UrlParserTestSuite, ConstructUrlCorrectly) {
  auto url = url_parser::Url{
      .protocol = "https",
      .host = "jan.ai",
      .pathParams = {"path1", "path2"},
  };
  auto url_str = url_parser::FromUrl(url);

  EXPECT_EQ(url_str, kValidUrlWithOnlyPaths);
}
