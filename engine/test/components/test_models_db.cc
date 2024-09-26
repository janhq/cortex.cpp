#include <filesystem>
#include <iostream>
#include "database/models.h"
#include "gtest/gtest.h"
#include "utils/file_manager_utils.h"

namespace cortex::db {
namespace {
constexpr const auto kTestDb = "./test.db";
}
class ModelsTestSuite : public ::testing::Test {
 public:
  ModelsTestSuite()
      : db_(kTestDb, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE),
        model_list_(db_) {}
  void SetUp() {
    db_.exec("DELETE FROM models");
  }

 protected:
  SQLite::Database db_;
  cortex::db::Models model_list_;

  const cortex::db::ModelEntry kTestModel{
      kTestModel.model_id,   "test_author", "main",
      "/path/to/model.yaml", "test_alias",  cortex::db::ModelStatus::READY};
};

TEST_F(ModelsTestSuite, TestAddModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  auto retrieved_model = model_list_.GetModelInfo(kTestModel.model_id);
  EXPECT_EQ(retrieved_model.model_id, kTestModel.model_id);
  EXPECT_EQ(retrieved_model.author_repo_id, kTestModel.author_repo_id);

  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model_id));
}

TEST_F(ModelsTestSuite, TestGetModelInfo) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  auto model_by_id = model_list_.GetModelInfo(kTestModel.model_id);
  EXPECT_EQ(model_by_id.model_id, kTestModel.model_id);

  auto model_by_alias = model_list_.GetModelInfo("test_alias");
  EXPECT_EQ(model_by_alias.model_id, kTestModel.model_id);

  EXPECT_THROW(model_list_.GetModelInfo("non_existent_model"),
               std::runtime_error);

  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model_id));
}

TEST_F(ModelsTestSuite, TestUpdateModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  cortex::db::ModelEntry updated_model = kTestModel;
  updated_model.status = cortex::db::ModelStatus::RUNNING;

  EXPECT_TRUE(model_list_.UpdateModelEntry(kTestModel.model_id, updated_model));

  auto retrieved_model = model_list_.GetModelInfo(kTestModel.model_id);
  EXPECT_EQ(retrieved_model.status, cortex::db::ModelStatus::RUNNING);
  updated_model.status = cortex::db::ModelStatus::READY;
  EXPECT_TRUE(model_list_.UpdateModelEntry(kTestModel.model_id, updated_model));

  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model_id));
}

TEST_F(ModelsTestSuite, TestDeleteModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model_id));
  EXPECT_THROW(model_list_.GetModelInfo(kTestModel.model_id),
               std::runtime_error);
}

TEST_F(ModelsTestSuite, TestGenerateShortenedAlias) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));
  auto alias = model_list_.GenerateShortenedAlias(
      "huggingface.co/bartowski/llama3.1-7b-gguf/Model_ID_Xxx.gguf");
  EXPECT_EQ(alias, "model_id_xxx");
  EXPECT_TRUE(model_list_.UpdateModelAlias(kTestModel.model_id, alias));

  // Test with existing entries to force longer alias
  alias = model_list_.GenerateShortenedAlias(
      "huggingface.co/bartowski/llama3.1-7b-gguf/Model_ID_Xxx.gguf");
  EXPECT_EQ(alias, "llama3.1-7b-gguf:model_id_xxx");

  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model_id));
}

TEST_F(ModelsTestSuite, TestPersistence) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  // Create a new ModelListUtils instance to test if it loads from file
  cortex::db::Models new_model_list(db_);
  auto retrieved_model = new_model_list.GetModelInfo(kTestModel.model_id);

  EXPECT_EQ(retrieved_model.model_id, kTestModel.model_id);
  EXPECT_EQ(retrieved_model.author_repo_id, kTestModel.author_repo_id);
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model_id));
}

TEST_F(ModelsTestSuite, TestUpdateModelAlias) {
  constexpr const auto kNewTestAlias = "new_test_alias";
  constexpr const auto kNonExistentModel = "non_existent_model";
  constexpr const auto kAnotherAlias = "another_alias";
  constexpr const auto kFinalTestAlias = "final_test_alias";
  constexpr const auto kAnotherModelId = "another_model_id";
  // Add the test model
  ASSERT_TRUE(model_list_.AddModelEntry(kTestModel));

  // Test successful update
  EXPECT_TRUE(model_list_.UpdateModelAlias(kTestModel.model_id, kNewTestAlias));
  auto updated_model = model_list_.GetModelInfo(kNewTestAlias);
  EXPECT_EQ(updated_model.model_alias, kNewTestAlias);
  EXPECT_EQ(updated_model.model_id, kTestModel.model_id);

  // Test update with non-existent model
  EXPECT_FALSE(model_list_.UpdateModelAlias(kNonExistentModel, kAnotherAlias));

  // Test update with non-unique alias
  cortex::db::ModelEntry another_model = kTestModel;
  another_model.model_id = kAnotherModelId;
  another_model.model_alias = kAnotherAlias;
  ASSERT_TRUE(model_list_.AddModelEntry(another_model));

  EXPECT_FALSE(
      model_list_.UpdateModelAlias(kTestModel.model_id, kAnotherAlias));

  // Test update using model alias instead of model ID
  EXPECT_TRUE(model_list_.UpdateModelAlias(kNewTestAlias, kFinalTestAlias));
  updated_model = model_list_.GetModelInfo(kFinalTestAlias);
  EXPECT_EQ(updated_model.model_alias, kFinalTestAlias);
  EXPECT_EQ(updated_model.model_id, kTestModel.model_id);

  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model_id));
  EXPECT_TRUE(model_list_.DeleteModelEntry(kAnotherModelId));
}

TEST_F(ModelsTestSuite, TestHasModel) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  EXPECT_TRUE(model_list_.HasModel(kTestModel.model_id));
  EXPECT_TRUE(model_list_.HasModel("test_alias"));
  EXPECT_FALSE(model_list_.HasModel("non_existent_model"));
  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model_id));
}
}  // namespace cortex::db