#include <filesystem>
#include <iostream>
#include "gtest/gtest.h"
#include "utils/file_manager_utils.h"
#include "utils/modellist_utils.h"

namespace {
constexpr const auto kTestDb = "./test.db";
}
class ModelListUtilsTestSuite : public ::testing::Test {
 public:
  ModelListUtilsTestSuite() : model_list_(kTestDb) {}

 protected:
  modellist_utils::ModelListUtils model_list_;

  const modellist_utils::ModelEntry kTestModel{
      kTestModel.model_id,
      "test_author",
      "main",
      "/path/to/model.yaml",
      "test_alias",
      modellist_utils::ModelStatus::READY};

  void SetUp() {
  }

  void TearDown() {
    // Clean up the temporary directory
    std::remove(kTestDb);
  }
};

TEST_F(ModelListUtilsTestSuite, TestAddModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  auto retrieved_model = model_list_.GetModelInfo(kTestModel.model_id);
  EXPECT_EQ(retrieved_model.model_id, kTestModel.model_id);
  EXPECT_EQ(retrieved_model.author_repo_id, kTestModel.author_repo_id);
}

TEST_F(ModelListUtilsTestSuite, TestGetModelInfo) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  auto model_by_id = model_list_.GetModelInfo(kTestModel.model_id);
  EXPECT_EQ(model_by_id.model_id, kTestModel.model_id);

  auto model_by_alias = model_list_.GetModelInfo("test_alias");
  EXPECT_EQ(model_by_alias.model_id, kTestModel.model_id);

  EXPECT_THROW(model_list_.GetModelInfo("non_existent_model"),
               std::runtime_error);
}

TEST_F(ModelListUtilsTestSuite, TestUpdateModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  modellist_utils::ModelEntry updated_model = kTestModel;
  updated_model.status = modellist_utils::ModelStatus::RUNNING;

  EXPECT_TRUE(model_list_.UpdateModelEntry(kTestModel.model_id, updated_model));

  auto retrieved_model = model_list_.GetModelInfo(kTestModel.model_id);
  EXPECT_EQ(retrieved_model.status, modellist_utils::ModelStatus::RUNNING);
  updated_model.status = modellist_utils::ModelStatus::READY;
  EXPECT_TRUE(model_list_.UpdateModelEntry(kTestModel.model_id, updated_model));
}

TEST_F(ModelListUtilsTestSuite, TestDeleteModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model_id));
  EXPECT_THROW(model_list_.GetModelInfo(kTestModel.model_id),
               std::runtime_error);
}

TEST_F(ModelListUtilsTestSuite, TestGenerateShortenedAlias) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));
  auto alias = model_list_.GenerateShortenedAlias(
      "huggingface.co/bartowski/llama3.1-7b-gguf/Model_ID_Xxx.gguf");
  EXPECT_EQ(alias, "model_id_xxx");
  EXPECT_TRUE(model_list_.UpdateModelAlias(kTestModel.model_id, alias));

  // Test with existing entries to force longer alias
  alias = model_list_.GenerateShortenedAlias(
      "huggingface.co/bartowski/llama3.1-7b-gguf/Model_ID_Xxx.gguf");
  EXPECT_EQ(alias, "llama3.1-7b-gguf:model_id_xxx");
}

TEST_F(ModelListUtilsTestSuite, TestPersistence) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  // Create a new ModelListUtils instance to test if it loads from file
  modellist_utils::ModelListUtils new_model_list(kTestDb);
  auto retrieved_model = new_model_list.GetModelInfo(kTestModel.model_id);

  EXPECT_EQ(retrieved_model.model_id, kTestModel.model_id);
  EXPECT_EQ(retrieved_model.author_repo_id, kTestModel.author_repo_id);
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model_id));
}

TEST_F(ModelListUtilsTestSuite, TestUpdateModelAlias) {
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
  modellist_utils::ModelEntry another_model = kTestModel;
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

TEST_F(ModelListUtilsTestSuite, TestHasModel) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  EXPECT_TRUE(model_list_.HasModel(kTestModel.model_id));
  EXPECT_TRUE(model_list_.HasModel("test_alias"));
  EXPECT_FALSE(model_list_.HasModel("non_existent_model"));
  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model_id));
}