#include "database/models.h"
#include "gtest/gtest.h"

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
      db_.exec(
          "CREATE TABLE models ("
          "model_id TEXT PRIMARY KEY,"
          "author_repo_id TEXT,"
          "branch_name TEXT,"
          "path_to_model_yaml TEXT,"
          "model_alias TEXT,"
          "model_format TEXT,"
          "model_source TEXT,"
          "status TEXT,"
          "engine TEXT,"
          "metadata TEXT"
          ")");
    } catch (const std::exception& e) {}
  }

  void TearDown() {
    try {
      db_.exec("DROP TABLE IF EXISTS models;");
    } catch (const std::exception& e) {}
  }

 protected:
  SQLite::Database db_;
  cortex::db::Models model_list_;

  const cortex::db::ModelEntry kTestModel{
      "test_model_id", "test_author",
      "main",          "/path/to/model.yaml",
      "test_alias",    "test_format",
      "test_source",   cortex::db::ModelStatus::Downloaded,
      "test_engine",   "",
  };
};

TEST_F(ModelsTestSuite, TestAddModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  auto retrieved_model = model_list_.GetModelInfo(kTestModel.model);
  EXPECT_TRUE(retrieved_model.has_value());
  EXPECT_EQ(retrieved_model.value().model, kTestModel.model);
  EXPECT_EQ(retrieved_model.value().author_repo_id, kTestModel.author_repo_id);
  EXPECT_EQ(retrieved_model.value().model_format, kTestModel.model_format);
  EXPECT_EQ(retrieved_model.value().model_source, kTestModel.model_source);
  EXPECT_EQ(retrieved_model.value().status, kTestModel.status);
  EXPECT_EQ(retrieved_model.value().engine, kTestModel.engine);

  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
}

TEST_F(ModelsTestSuite, TestGetModelInfo) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  auto model_by_id = model_list_.GetModelInfo(kTestModel.model);
  EXPECT_TRUE(model_by_id.has_value());
  EXPECT_EQ(model_by_id.value().model, kTestModel.model);

  EXPECT_TRUE(model_list_.GetModelInfo("non_existent_model").has_error());

  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
}

TEST_F(ModelsTestSuite, TestUpdateModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  cortex::db::ModelEntry updated_model = kTestModel;
  updated_model.status = cortex::db::ModelStatus::Downloaded;

  EXPECT_TRUE(
      model_list_.UpdateModelEntry(kTestModel.model, updated_model).value());

  auto retrieved_model = model_list_.GetModelInfo(kTestModel.model);
  EXPECT_TRUE(retrieved_model.has_value());
  EXPECT_EQ(retrieved_model.value().status, updated_model.status);

  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
}

TEST_F(ModelsTestSuite, TestDeleteModelEntry) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
  EXPECT_TRUE(model_list_.GetModelInfo(kTestModel.model).has_error());
}

TEST_F(ModelsTestSuite, TestPersistence) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  // Create a new ModelListUtils instance to test if it loads from file
  cortex::db::Models new_model_list(db_);
  auto retrieved_model = new_model_list.GetModelInfo(kTestModel.model);
  EXPECT_TRUE(retrieved_model.has_value());
  EXPECT_EQ(retrieved_model.value().model, kTestModel.model);
  EXPECT_EQ(retrieved_model.value().author_repo_id, kTestModel.author_repo_id);
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
}

TEST_F(ModelsTestSuite, TestHasModel) {
  EXPECT_TRUE(model_list_.AddModelEntry(kTestModel).value());

  EXPECT_TRUE(model_list_.HasModel(kTestModel.model));
  EXPECT_FALSE(model_list_.HasModel("non_existent_model"));
  // Clean up
  EXPECT_TRUE(model_list_.DeleteModelEntry(kTestModel.model).value());
}

}  // namespace cortex::db
