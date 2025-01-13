#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include "config/yaml_config.h"
#include "gtest/gtest.h"

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

class YamlHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { handler = new config::YamlHandler(); }

  void TearDown() override { delete handler; }

  config::YamlHandler* handler;

  // Helper function to create a temporary YAML file
  std::string createTempYamlFile(const std::string& content) {
    std::string filename;
#ifdef _WIN32
    char tempPath[MAX_PATH];
    char tempFileName[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    GetTempFileNameA(tempPath, "yaml", 0, tempFileName);
    filename = tempFileName;
#else
    char tempFileName[] = "/tmp/yaml_test_XXXXXX";
    int fd = mkstemp(tempFileName);
    if (fd == -1) {
      throw std::runtime_error("Failed to create temporary file");
    }
    close(fd);
    filename = tempFileName;
#endif

    std::ofstream file(filename);
    file << content;
    file.close();
    return filename;
  }

  // Helper function to remove a file
  void removeFile(const std::string& filename) {
    std::remove(filename.c_str());
  }
};

TEST_F(YamlHandlerTest, ModelConfigFromFile) {
  std::string yaml_content = R"(
name: test_model
model: test_model_v1
version: 1.0
engine: test_engine
prompt_template: "Test prompt {system_message} {prompt}"
top_p: 0.9
temperature: 0.7
max_tokens: 100
stream: true
n_parallel: 2
cpu_threads: 3
stop:
  - "END"
files:
  - "test_file.gguf"
    )";

  std::string filename = createTempYamlFile(yaml_content);

  handler->ModelConfigFromFile(filename);
  const config::ModelConfig& config = handler->GetModelConfig();

  EXPECT_EQ(config.name, "test_model");
  EXPECT_EQ(config.model, "test_model_v1");
  EXPECT_EQ(config.version, "1.0");
  EXPECT_EQ(config.engine, "test_engine");
  EXPECT_EQ(config.prompt_template, "Test prompt {system_message} {prompt}");
  EXPECT_FLOAT_EQ(config.top_p, 0.9f);
  EXPECT_FLOAT_EQ(config.temperature, 0.7f);
  EXPECT_EQ(config.max_tokens, 100);
  EXPECT_TRUE(config.stream);
  EXPECT_EQ(config.n_parallel, 2);
  EXPECT_EQ(config.cpu_threads, 3);
  EXPECT_EQ(config.stop.size(), 1);
  EXPECT_EQ(config.stop[0], "END");
  EXPECT_EQ(config.files.size(), 1);
  EXPECT_EQ(config.files[0], "test_file.gguf");

  removeFile(filename);
}

TEST_F(YamlHandlerTest, UpdateModelConfig) {
  config::ModelConfig new_config;
  new_config.name = "updated_model";
  new_config.model = "updated_model_v2";
  new_config.version = "2.0";
  new_config.engine = "updated_engine";
  new_config.prompt_template = "Updated prompt {system_message} {prompt}";
  new_config.top_p = 0.95f;
  new_config.temperature = 0.8f;
  new_config.max_tokens = 200;
  new_config.stream = false;
  new_config.n_parallel = 2;
  new_config.cpu_threads = 3;
  new_config.stop = {"STOP", "END"};
  new_config.files = {"updated_file1.gguf", "updated_file2.gguf"};

  handler->UpdateModelConfig(new_config);
  const config::ModelConfig& config = handler->GetModelConfig();

  EXPECT_EQ(config.name, "updated_model");
  EXPECT_EQ(config.model, "updated_model_v2");
  EXPECT_EQ(config.version, "2.0");
  EXPECT_EQ(config.engine, "updated_engine");
  EXPECT_EQ(config.prompt_template, "Updated prompt {system_message} {prompt}");
  EXPECT_FLOAT_EQ(config.top_p, 0.95f);
  EXPECT_FLOAT_EQ(config.temperature, 0.8f);
  EXPECT_EQ(config.max_tokens, 200);
  EXPECT_FALSE(config.stream);
  EXPECT_EQ(config.n_parallel, 2);
  EXPECT_EQ(config.cpu_threads, 3);
  EXPECT_EQ(config.stop.size(), 2);
  EXPECT_EQ(config.stop[0], "STOP");
  EXPECT_EQ(config.stop[1], "END");
  EXPECT_EQ(config.files.size(), 2);
  EXPECT_EQ(config.files[0], "updated_file1.gguf");
  EXPECT_EQ(config.files[1], "updated_file2.gguf");
}

TEST_F(YamlHandlerTest, WriteYamlFile) {
  config::ModelConfig new_config;
  new_config.name = "write_test_model";
  new_config.model = "write_test_model_v1";
  new_config.version = "1.0";
  new_config.engine = "write_test_engine";
  new_config.prompt_template = "Write test prompt {system_message} {prompt}";
  new_config.top_p = 0.85f;
  new_config.temperature = 0.6f;
  new_config.max_tokens = 150;
  new_config.stream = true;
  new_config.n_parallel = 2;
  new_config.cpu_threads = 3;
  new_config.stop = {"HALT"};
  new_config.files = {"write_test_file.gguf"};

  handler->UpdateModelConfig(new_config);

  std::string filename = createTempYamlFile("");  // Create empty file
  handler->WriteYamlFile(filename);

  // Read the file back and verify its contents
  config::YamlHandler read_handler;
  read_handler.ModelConfigFromFile(filename);
  const config::ModelConfig& read_config = read_handler.GetModelConfig();

  EXPECT_EQ(read_config.name, "write_test_model");
  EXPECT_EQ(read_config.model, "write_test_model_v1");
  EXPECT_EQ(read_config.version, "1.0");
  EXPECT_EQ(read_config.engine, "write_test_engine");
  EXPECT_EQ(read_config.prompt_template,
            "Write test prompt {system_message} {prompt}");
  EXPECT_FLOAT_EQ(read_config.top_p, 0.85f);
  EXPECT_FLOAT_EQ(read_config.temperature, 0.6f);
  EXPECT_EQ(read_config.max_tokens, 150);
  EXPECT_TRUE(read_config.stream);
  EXPECT_EQ(read_config.n_parallel, 2);
  EXPECT_EQ(read_config.cpu_threads, 3);
  EXPECT_EQ(read_config.stop.size(), 1);
  EXPECT_EQ(read_config.stop[0], "HALT");
  EXPECT_EQ(read_config.files.size(), 1);
  EXPECT_EQ(read_config.files[0], "write_test_file.gguf");

  removeFile(filename);
}

TEST_F(YamlHandlerTest, Reset) {
  config::ModelConfig new_config;
  new_config.name = "test_reset_model";
  new_config.model = "test_reset_model_v1";
  handler->UpdateModelConfig(new_config);

  handler->Reset();
  const config::ModelConfig& config = handler->GetModelConfig();

  EXPECT_TRUE(config.name.empty());
  EXPECT_TRUE(config.model.empty());
}

TEST_F(YamlHandlerTest, InvalidYamlFile) {
  std::string invalid_yaml_content = R"(
name: test_model
model: test_model_v1
version: 1.0
engine: test_engine
prompt_template: "Test prompt {system_message} {prompt}"
top_p: not_a_float
seed: also_not_an_int
    )";

  std::string filename = createTempYamlFile(invalid_yaml_content);
  handler->ModelConfigFromFile(filename);
  config::ModelConfig new_config = handler->GetModelConfig();
  EXPECT_EQ(new_config.seed, -1);

  removeFile(filename);
}