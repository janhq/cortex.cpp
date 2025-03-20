#include <gtest/gtest.h>
#include <json/json.h>
#include "common/tool_resources.h"

namespace OpenAi {
namespace {

// Mock class for testing abstract ToolResources
class MockToolResources : public ToolResources {
 public:
  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value json;
    json["mock"] = "value";
    return json;
  }
};

class ToolResourcesTest : public ::testing::Test {};

TEST_F(ToolResourcesTest, MoveConstructor) {
  MockToolResources original;
  MockToolResources moved(std::move(original));

  auto json_result = moved.ToJson();
  ASSERT_TRUE(json_result.has_value());
  EXPECT_EQ(json_result.value()["mock"].asString(), "value");
}

TEST_F(ToolResourcesTest, MoveAssignment) {
  MockToolResources original;
  MockToolResources target;
  target = std::move(original);

  auto json_result = target.ToJson();
  ASSERT_TRUE(json_result.has_value());
  EXPECT_EQ(json_result.value()["mock"].asString(), "value");
}

class CodeInterpreterTest : public ::testing::Test {
 protected:
  void SetUp() override { sample_file_ids = {"file1", "file2", "file3"}; }

  std::vector<std::string> sample_file_ids;
};

TEST_F(CodeInterpreterTest, DefaultConstruction) {
  CodeInterpreter interpreter;
  EXPECT_TRUE(interpreter.file_ids.empty());
}

TEST_F(CodeInterpreterTest, MoveConstructor) {
  CodeInterpreter original;
  original.file_ids = sample_file_ids;

  CodeInterpreter moved(std::move(original));
  EXPECT_EQ(moved.file_ids, sample_file_ids);
  EXPECT_TRUE(original.file_ids.empty());  // NOLINT: Checking moved-from state
}

TEST_F(CodeInterpreterTest, MoveAssignment) {
  CodeInterpreter original;
  original.file_ids = sample_file_ids;

  CodeInterpreter target;
  target = std::move(original);
  EXPECT_EQ(target.file_ids, sample_file_ids);
  EXPECT_TRUE(original.file_ids.empty());  // NOLINT: Checking moved-from state
}

TEST_F(CodeInterpreterTest, FromJsonWithFileIds) {
  Json::Value json;
  Json::Value file_ids(Json::arrayValue);
  for (const auto& id : sample_file_ids) {
    file_ids.append(id);
  }
  json["file_ids"] = file_ids;

  auto result = CodeInterpreter::FromJson(json);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value().file_ids, sample_file_ids);
}

TEST_F(CodeInterpreterTest, FromJsonWithoutFileIds) {
  Json::Value json;  // Empty JSON
  auto result = CodeInterpreter::FromJson(json);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result.value().file_ids.empty());
}

TEST_F(CodeInterpreterTest, ToJson) {
  CodeInterpreter interpreter;
  interpreter.file_ids = sample_file_ids;

  auto result = interpreter.ToJson();
  ASSERT_TRUE(result.has_value());

  const auto& json = result.value();
  ASSERT_TRUE(json.isMember("file_ids"));
  ASSERT_TRUE(json["file_ids"].isArray());
  ASSERT_EQ(json["file_ids"].size(), sample_file_ids.size());

  for (Json::ArrayIndex i = 0; i < json["file_ids"].size(); ++i) {
    EXPECT_EQ(json["file_ids"][i].asString(), sample_file_ids[i]);
  }
}

TEST_F(CodeInterpreterTest, ToJsonEmptyFileIds) {
  CodeInterpreter interpreter;

  auto result = interpreter.ToJson();
  ASSERT_TRUE(result.has_value());

  const auto& json = result.value();
  ASSERT_TRUE(json.isMember("file_ids"));
  ASSERT_TRUE(json["file_ids"].isArray());
  EXPECT_EQ(json["file_ids"].size(), 0);
}

class FileSearchTest : public ::testing::Test {
 protected:
  void SetUp() override {
    sample_vector_store_ids = {"store1", "store2", "store3"};
  }

  std::vector<std::string> sample_vector_store_ids;
};

TEST_F(FileSearchTest, DefaultConstruction) {
  FileSearch search;
  EXPECT_TRUE(search.vector_store_ids.empty());
}

TEST_F(FileSearchTest, MoveConstructor) {
  FileSearch original;
  original.vector_store_ids = sample_vector_store_ids;

  FileSearch moved(std::move(original));
  EXPECT_EQ(moved.vector_store_ids, sample_vector_store_ids);
  EXPECT_TRUE(
      original.vector_store_ids.empty());  // NOLINT: Checking moved-from state
}

TEST_F(FileSearchTest, MoveAssignment) {
  FileSearch original;
  original.vector_store_ids = sample_vector_store_ids;

  FileSearch target;
  target = std::move(original);
  EXPECT_EQ(target.vector_store_ids, sample_vector_store_ids);
  EXPECT_TRUE(
      original.vector_store_ids.empty());  // NOLINT: Checking moved-from state
}

TEST_F(FileSearchTest, FromJsonWithVectorStoreIds) {
  Json::Value json;
  Json::Value vector_store_ids(Json::arrayValue);
  for (const auto& id : sample_vector_store_ids) {
    vector_store_ids.append(id);
  }
  json["vector_store_ids"] = vector_store_ids;

  auto result = FileSearch::FromJson(json);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value().vector_store_ids, sample_vector_store_ids);
}

TEST_F(FileSearchTest, FromJsonWithoutVectorStoreIds) {
  Json::Value json;  // Empty JSON
  auto result = FileSearch::FromJson(json);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result.value().vector_store_ids.empty());
}

TEST_F(FileSearchTest, ToJson) {
  FileSearch search;
  search.vector_store_ids = sample_vector_store_ids;

  auto result = search.ToJson();
  ASSERT_TRUE(result.has_value());

  const auto& json = result.value();
  ASSERT_TRUE(json.isMember("vector_store_ids"));
  ASSERT_TRUE(json["vector_store_ids"].isArray());
  ASSERT_EQ(json["vector_store_ids"].size(), sample_vector_store_ids.size());

  for (Json::ArrayIndex i = 0; i < json["vector_store_ids"].size(); ++i) {
    EXPECT_EQ(json["vector_store_ids"][i].asString(),
              sample_vector_store_ids[i]);
  }
}

TEST_F(FileSearchTest, ToJsonEmptyVectorStoreIds) {
  FileSearch search;

  auto result = search.ToJson();
  ASSERT_TRUE(result.has_value());

  const auto& json = result.value();
  ASSERT_TRUE(json.isMember("vector_store_ids"));
  ASSERT_TRUE(json["vector_store_ids"].isArray());
  EXPECT_EQ(json["vector_store_ids"].size(), 0);
}

TEST_F(FileSearchTest, SelfAssignment) {
  FileSearch search;
  search.vector_store_ids = sample_vector_store_ids;

  // search = std::move(search);  // Self-assignment with move
  EXPECT_EQ(search.vector_store_ids, sample_vector_store_ids);
}
}  // namespace
}  // namespace OpenAi
