#include <filesystem>
#include <iostream>
#include "gtest/gtest.h"
#include "utils/modellist_utils.h"
#include "utils/file_manager_utils.h"
class ModelListUtilsTestSuite : public ::testing::Test {
 protected:
  modellist_utils::ModelListUtils model_list_;

  const modellist_utils::ModelEntry kTestModel{
      "test_model_id", "test_author",
      "main",          "/path/to/model.yaml",
      "test_alias",    modellist_utils::ModelStatus::READY};
};
  void SetUp()  {
    // Create a temporary directory for tests
    file_manager_utils::CreateConfigFileIfNotExist();
  }

  void TearDown() {
    // Clean up the temporary directory
  }
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

TEST_F(ModelListUtilsTestSuite, TestUpdateModelAlias) {
  // Add the test model
  ASSERT_TRUE(model_list_.AddModelEntry(kTestModel));

  // Test successful update
  EXPECT_TRUE(model_list_.UpdateModelAlias("test_model_id", "new_test_alias"));
  auto updated_model = model_list_.GetModelInfo("new_test_alias");
  EXPECT_EQ(updated_model.model_alias, "new_test_alias");
  EXPECT_EQ(updated_model.model_id, "test_model_id");

  // Test update with non-existent model
  EXPECT_FALSE(model_list_.UpdateModelAlias("non_existent_model", "another_alias"));

  // Test update with non-unique alias
  modellist_utils::ModelEntry another_model = kTestModel;
  another_model.model_id = "another_model_id";
  another_model.model_alias = "another_alias";
  ASSERT_TRUE(model_list_.AddModelEntry(another_model));

  EXPECT_FALSE(model_list_.UpdateModelAlias("test_model_id", "another_alias"));

  // Test update using model alias instead of model ID
  EXPECT_TRUE(model_list_.UpdateModelAlias("new_test_alias", "final_test_alias"));
  updated_model = model_list_.GetModelInfo("final_test_alias");
  EXPECT_EQ(updated_model.model_alias, "final_test_alias");
  EXPECT_EQ(updated_model.model_id, "test_model_id");

  // Clean up
  model_list_.DeleteModelEntry("test_model_id");
  model_list_.DeleteModelEntry("another_model_id");
}

TEST_F(ModelListUtilsTestSuite, TestHasModel) {
  model_list_.AddModelEntry(kTestModel);

  EXPECT_TRUE(model_list_.HasModel("test_model_id"));
  EXPECT_TRUE(model_list_.HasModel("test_alias"));
  EXPECT_FALSE(model_list_.HasModel("non_existent_model"));
}