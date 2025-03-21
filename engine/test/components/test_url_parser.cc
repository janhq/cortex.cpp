#include "gtest/gtest.h"
#include "utils/url_parser.h"

class UrlParserTestSuite : public ::testing::Test {
 protected:
  constexpr static auto kValidUrlWithOnlyPaths{"https://jan.ai/path1/path2"};
};

TEST_F(UrlParserTestSuite, TestParseUrlCorrectly) {
  auto url = url_parser::FromUrlString(kValidUrlWithOnlyPaths);

  EXPECT_EQ(url->host, "jan.ai");
  EXPECT_EQ(url->protocol, "https");
  EXPECT_EQ(url->pathParams.size(), 2);
}

TEST_F(UrlParserTestSuite, ConstructUrlCorrectly) {
  auto url = url_parser::Url{
      /* .protocol = */ "https",
      /* .host = */ "jan.ai",
      /* .pathParams = */ {"path1", "path2"},
      /* .queries = */ {},
  };
  auto url_str = url_parser::FromUrl(url);

  EXPECT_EQ(url_str, kValidUrlWithOnlyPaths);
}

TEST_F(UrlParserTestSuite, ConstructUrlWithQueryCorrectly) {
  auto url = url_parser::Url{
      /* .protocol = */ "https",
      /* .host = */ "jan.ai",
      /* .pathParams = */ {"path1", "path2"},
      /* .queries = */ {{"key1", "value1"}, {"key2", 2}, {"key3", true}},
  };
  auto url_str = url_parser::FromUrl(url);

  auto contains_key1 = url_str.find("key1=value1") != std::string::npos;
  auto contains_key2 = url_str.find("key2=2") != std::string::npos;
  auto contains_key3 = url_str.find("key3=true") != std::string::npos;

  EXPECT_TRUE(contains_key1);
  EXPECT_TRUE(contains_key2);
  EXPECT_TRUE(contains_key3);
}

TEST_F(UrlParserTestSuite, ConstructUrlWithEmptyPathCorrectly) {
  auto url = url_parser::Url{
      /* .protocol = */ "https",
      /* .host = */ "jan.ai",
      /* .pathParams = */ {},
      /* .queries = */ {},
  };
  auto url_str = url_parser::FromUrl(url);

  EXPECT_EQ(url_str, "https://jan.ai");
}

TEST_F(UrlParserTestSuite, GetProtocolAndHostCorrectly) {
  auto url = url_parser::Url{
      /* .protocol = */ "https",
      /* .host = */ "jan.ai",
      /* .pathParams = */ {},
      /* .queries= */ {},
  };
  auto protocol_and_host = url.GetProtocolAndHost();
  EXPECT_EQ(protocol_and_host, "https://jan.ai");
}

TEST_F(UrlParserTestSuite, GetPathAndQueryCorrectly) {
  auto url = url_parser::Url{
      /* .protocol = */ "https",
      /* .host = */ "jan.ai",
      /* .pathParams = */ {"path1", "path2"},
      /* .queries = */ {},
  };
  auto path_and_query = url.GetPathAndQuery();
  EXPECT_EQ(path_and_query, "/path1/path2");
}
