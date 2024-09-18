#include <filesystem>
#include <iostream>
#include "gtest/gtest.h"
#include "utils/modellist_utils.h"

class ModelListUtilsTestSuite : public ::testing::Test {
 protected:
  modellist_utils::ModelListUtils model_list_;

  const modellist_utils::ModelEntry kTestModel{
      "test_model_id", "test_author",
      "main",          "/path/to/model.yaml",
      "test_alias",    modellist_utils::ModelStatus::READY};
};

TEST_F(ModelListUtilsTestSuite, TestAddModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel));

  auto retrieved_model = model_list_.GetModelInfo("test_model_id");
  EXPECT_EQ(retrieved_model.model_id, kTestModel.model_id);
  EXPECT_EQ(retrieved_model.author_repo_id, kTestModel.author_repo_id);
}

TEST_F(ModelListUtilsTestSuite, TestGetModelInfo) {
  model_list_.AddModelEntry(kTestModel);

  auto model_by_id = model_list_.GetModelInfo("test_model_id");
  EXPECT_EQ(model_by_id.model_id, kTestModel.model_id);

  auto model_by_alias = model_list_.GetModelInfo("test_alias");
  EXPECT_EQ(model_by_alias.model_id, kTestModel.model_id);

  EXPECT_THROW(model_list_.GetModelInfo("non_existent_model"),
               std::runtime_error);
}

TEST_F(ModelListUtilsTestSuite, TestUpdateModelEntry) {
  model_list_.AddModelEntry(kTestModel);

  modellist_utils::ModelEntry updated_model = kTestModel;
  updated_model.status = modellist_utils::ModelStatus::RUNNING;

  EXPECT_TRUE(model_list_.UpdateModelEntry("test_model_id", updated_model));

  auto retrieved_model = model_list_.GetModelInfo("test_model_id");
  EXPECT_EQ(retrieved_model.status, modellist_utils::ModelStatus::RUNNING);
  updated_model.status = modellist_utils::ModelStatus::READY;
  model_list_.UpdateModelEntry("test_model_id", updated_model);
}

TEST_F(ModelListUtilsTestSuite, TestDeleteModelEntry) {
  model_list_.AddModelEntry(kTestModel);

  EXPECT_TRUE(model_list_.DeleteModelEntry("test_model_id"));
  EXPECT_THROW(model_list_.GetModelInfo("test_model_id"), std::runtime_error);
}

TEST_F(ModelListUtilsTestSuite, TestGenerateShortenedAlias) {
  auto alias = model_list_.GenerateShortenedAlias(
      "huggingface.co/bartowski/llama3.1-7b-gguf/Model_ID_Xxx.gguf", {});
  EXPECT_EQ(alias, "model_id_xxx");

  // Test with existing entries to force longer alias
  modellist_utils::ModelEntry existing_model = kTestModel;
  existing_model.model_alias = "model_id_xxx";
  std::vector<modellist_utils::ModelEntry> existing_entries = {existing_model};

  alias = model_list_.GenerateShortenedAlias(
      "huggingface.co/bartowski/llama3.1-7b-gguf/Model_ID_Xxx.gguf",
      existing_entries);
  EXPECT_EQ(alias, "llama3.1-7b-gguf:model_id_xxx");
}

TEST_F(ModelListUtilsTestSuite, TestPersistence) {
  model_list_.AddModelEntry(kTestModel);

  // Create a new ModelListUtils instance to test if it loads from file
  modellist_utils::ModelListUtils new_model_list;
  auto retrieved_model = new_model_list.GetModelInfo("test_model_id");

  EXPECT_EQ(retrieved_model.model_id, kTestModel.model_id);
  EXPECT_EQ(retrieved_model.author_repo_id, kTestModel.author_repo_id);
  model_list_.DeleteModelEntry("test_model_id");
}