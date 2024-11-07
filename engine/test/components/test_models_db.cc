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
    try {
      db_.exec("DELETE FROM models");
    } catch (const std::exception& e) {}
  }

 protected:
  SQLite::Database db_;
  cortex::db::Models model_list_;

  const cortex::db::ModelEntry kTestModel{"test_model_id", "test_author",
                                          "main", "/path/to/model.yaml",
                                          "test_alias"};
};

TEST_F(ModelsTestSuite, TestAddModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  auto retrieved_model = model_list_.GetModelInfo(kTestModel.model);
  EXPECT_TRUE(retrieved_model);
  EXPECT_EQ(retrieved_model.value().model, kTestModel.model);
  EXPECT_EQ(retrieved_model.value().author_repo_id, kTestModel.author_repo_id);

  // // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
}

TEST_F(ModelsTestSuite, TestGetModelInfo) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  auto model_by_id = model_list_.GetModelInfo(kTestModel.model);
  EXPECT_TRUE(model_by_id);
  EXPECT_EQ(model_by_id.value().model, kTestModel.model);

  auto model_by_alias = model_list_.GetModelInfo("test_alias");
  EXPECT_TRUE(model_by_alias);
  EXPECT_EQ(model_by_alias.value().model, kTestModel.model);

  EXPECT_TRUE(model_list_.GetModelInfo("non_existent_model").has_error());

  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
}

TEST_F(ModelsTestSuite, TestUpdateModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  cortex::db::ModelEntry updated_model = kTestModel;

  EXPECT_TRUE(
      model_list_.UpdateModelEntry(kTestModel.model, updated_model).value());

  auto retrieved_model = model_list_.GetModelInfo(kTestModel.model);
  EXPECT_TRUE(retrieved_model);
  EXPECT_TRUE(
      model_list_.UpdateModelEntry(kTestModel.model, updated_model).value());

  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
}

TEST_F(ModelsTestSuite, TestDeleteModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
  EXPECT_TRUE(model_list_.GetModelInfo(kTestModel.model).has_error());
}

TEST_F(ModelsTestSuite, TestGenerateShortenedAlias) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());
  auto models1 = model_list_.LoadModelList();
  auto alias = model_list_.GenerateShortenedAlias(
      "huggingface.co:bartowski:llama3.1-7b-gguf:Model_ID_Xxx.gguf",
      models1.value());
  EXPECT_EQ(alias, "model_id_xxx");
  EXPECT_TRUE(model_list_.UpdateModelAlias(kTestModel.model, alias).value());

  // Test with existing entries to force longer alias
  auto models2 = model_list_.LoadModelList();
  alias = model_list_.GenerateShortenedAlias(
      "huggingface.co:bartowski:llama3.1-7b-gguf:Model_ID_Xxx.gguf",
      models2.value());
  EXPECT_EQ(alias, "llama3.1-7b-gguf:model_id_xxx");

  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
}

TEST_F(ModelsTestSuite, TestPersistence) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  // Create a new ModelListUtils instance to test if it loads from file
  cortex::db::Models new_model_list(db_);
  auto retrieved_model = new_model_list.GetModelInfo(kTestModel.model);
  EXPECT_TRUE(retrieved_model);
  EXPECT_EQ(retrieved_model.value().model, kTestModel.model);
  EXPECT_EQ(retrieved_model.value().author_repo_id, kTestModel.author_repo_id);
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
}

TEST_F(ModelsTestSuite, TestUpdateModelAlias) {
  constexpr const auto kNewTestAlias = "new_test_alias";
  constexpr const auto kNonExistentModel = "non_existent_model";
  constexpr const auto kAnotherAlias = "another_alias";
  constexpr const auto kFinalTestAlias = "final_test_alias";
  constexpr const auto kAnotherModelId = "another_model_id";
  // Add the test model
  ASSERT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  // Test successful update
  EXPECT_TRUE(
      model_list_.UpdateModelAlias(kTestModel.model, kNewTestAlias).value());
  auto updated_model = model_list_.GetModelInfo(kNewTestAlias);
  EXPECT_TRUE(updated_model);
  EXPECT_EQ(updated_model.value().model_alias, kNewTestAlias);
  EXPECT_EQ(updated_model.value().model, kTestModel.model);

  // Test update with non-existent model
  EXPECT_TRUE(model_list_.UpdateModelAlias(kNonExistentModel, kAnotherAlias)
                  .has_error());

  // Test update with non-unique alias
  cortex::db::ModelEntry another_model = kTestModel;
  another_model.model = kAnotherModelId;
  another_model.model_alias = kAnotherAlias;
  ASSERT_TRUE(model_list_.AddModelEntry(another_model).value());

  EXPECT_FALSE(
      model_list_.UpdateModelAlias(kTestModel.model, kAnotherAlias).value());

  // Test update using model alias instead of model ID
  EXPECT_TRUE(model_list_.UpdateModelAlias(kNewTestAlias, kFinalTestAlias));
  updated_model = model_list_.GetModelInfo(kFinalTestAlias);
  EXPECT_TRUE(updated_model);
  EXPECT_EQ(updated_model.value().model_alias, kFinalTestAlias);
  EXPECT_EQ(updated_model.value().model, kTestModel.model);

  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
  EXPECT_TRUE(model_list_.DeleteModelEntry(kAnotherModelId).value());
}

TEST_F(ModelsTestSuite, TestHasModel) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  EXPECT_TRUE(model_list_.HasModel(kTestModel.model));
  EXPECT_TRUE(model_list_.HasModel("test_alias"));
  EXPECT_FALSE(model_list_.HasModel("non_existent_model"));
  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
}
}  // namespace cortex::db
