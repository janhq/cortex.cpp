#include "gtest/gtest.h"
#include "utils/nitro_utils.h"

class NitroUtilTest : public ::testing::Test {};

TEST_F(NitroUtilTest, left_trim) {
  {
    std::string empty;
    nitro_utils::ltrim(empty);
    EXPECT_EQ(empty, "");
  }

  {
    std::string s = "abc";
    std::string expected = "abc";
    nitro_utils::ltrim(s);
    EXPECT_EQ(s, expected);
  }

  {
    std::string s = " abc";
    std::string expected = "abc";
    nitro_utils::ltrim(s);
    EXPECT_EQ(s, expected);
  }

  {
    std::string s = "1 abc 2 ";
    std::string expected = "1 abc 2 ";
    nitro_utils::ltrim(s);
    EXPECT_EQ(s, expected);
  }

  {
    std::string s = " |abc";
    std::string expected = "|abc";
    nitro_utils::ltrim(s);
    EXPECT_EQ(s, expected);
  }
}

TEST_F(NitroUtilTest, get_model_id) {
  // linux
  {
    Json::Value data;
    data["llama_model_path"] =
        "e:/workspace/model/"
        "Starling_Monarch_Westlake_Garten-7B-v0.1-IQ4_XS.gguf";
    std::string expected =
        "Starling_Monarch_Westlake_Garten-7B-v0.1-IQ4_XS";
    EXPECT_EQ(nitro_utils::getModelId(data), expected);
  }

  {
    Json::Value data;
    data["llama_model_path"] =
        "Starling_Monarch_Westlake_Garten-7B-v0.1-IQ4_XS.gguf";
    std::string expected =
        "Starling_Monarch_Westlake_Garten-7B-v0.1-IQ4_XS";
    EXPECT_EQ(nitro_utils::getModelId(data), expected);
  }

  // windows
  {
    Json::Value data;
    data["llama_model_path"] =
        "e:\\workspace\\model\\"
        "Starling_Monarch_Westlake_Garten-7B-v0.1-IQ4_XS.gguf";
    std::string expected =
        "Starling_Monarch_Westlake_Garten-7B-v0.1-IQ4_XS";
    EXPECT_EQ(nitro_utils::getModelId(data), expected);
  }

  {
    Json::Value data;
    data["llama_model_path"] =
        "Starling_Monarch_Westlake_Garten-7B-v0.1-IQ4_XS";
    std::string expected = "Starling_Monarch_Westlake_Garten-7B-v0.1-IQ4_XS";
    EXPECT_EQ(nitro_utils::getModelId(data), expected);
  }

  {
    Json::Value data;
    data["llama_model_path"] = "";
    data["model_alias"] =
        "Starling_Monarch_Westlake_Garten-7B-v0.1-IQ4_XS";
    std::string expected = "Starling_Monarch_Westlake_Garten-7B-v0.1-IQ4_XS";
    EXPECT_EQ(nitro_utils::getModelId(data), expected);
  }

  {
    Json::Value data;
    data["llama_model_path"] = "";
    std::string expected = "";
    EXPECT_EQ(nitro_utils::getModelId(data), expected);
  }

  // For embedding request
  {
    Json::Value data;
    data["model"] =
        "nomic-embed-text-v1.5.f16";
    std::string expected = "nomic-embed-text-v1.5.f16";
    EXPECT_EQ(nitro_utils::getModelId(data), expected);
  }

  {
    Json::Value data;
    data["llama_model_path"] = "C:\\Users\\runneradmin\\AppData\\Local\\Temp\\testllm";
    std::string expected = "testllm";
    EXPECT_EQ(nitro_utils::getModelId(data), expected);
  }
}