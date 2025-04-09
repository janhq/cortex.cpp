#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <filesystem>
#include <fstream>
#include "gtest/gtest.h"
#include "utils/config_yaml_utils.h"
#include <yaml-cpp/yaml.h>

namespace config_yaml_utils {
namespace cyu = config_yaml_utils;
class CortexConfigTest : public ::testing::Test {
 protected:
  const std::string test_file_path = "test_config.yaml";
  CortexConfig default_config;

  void SetUp() override {
    // Set up default configuration
    default_config = {
        "default_log_path",
        "default_llamacpp_log_path",
        "default_onnx_log_path",
        "default_data_path",
        1000,
        kDefaultHost,
        kDefaultPort,
        kDefaultCheckedForUpdateAt,
        kDefaultCheckedForLlamacppUpdateAt,
        kDefaultLatestRelease,
        "",
        "",
        "",
        "",
        "",
        "",
        false,
        {},
        "",
        false,
        false,
        "",
        "",
        "",
        false,
        false,
        "",
        "",
        {},
        0,
        {},
    };
  }

  void TearDown() override {
    // Clean up: remove the test file if it exists
    if (std::filesystem::exists(test_file_path)) {
      std::filesystem::remove(test_file_path);
    }
  }
};

TEST_F(CortexConfigTest, DumpYamlConfig_WritesCorrectly) {
  CortexConfig config = {
      "log_path",
      "default_llamacpp_log_path",
      "default_onnx_log_path",
      "data_path",
      5000,
      "localhost",
      "8080",
      123456789,
      123456789,
      "v1.0.0",
      "",
      "",
      "",
      "",
      "",
      "",
      false,
      {},
      "",
      false,
      false,
      "",
      "",
      "",
      false,
      false,
      "",
      "",
      {},
      0,
      {},
  };

  auto result = cyu::CortexConfigMgr::GetInstance().DumpYamlConfig(
      config, test_file_path);
  EXPECT_FALSE(result.has_error());

  // Verify that the file was created and contains the expected data
  YAML::Node node = YAML::LoadFile(test_file_path);
  EXPECT_EQ(node["logFolderPath"].as<std::string>(), config.logFolderPath);
  EXPECT_EQ(node["dataFolderPath"].as<std::string>(), config.dataFolderPath);
  EXPECT_EQ(node["maxLogLines"].as<int>(), config.maxLogLines);
  EXPECT_EQ(node["apiServerHost"].as<std::string>(), config.apiServerHost);
  EXPECT_EQ(node["apiServerPort"].as<std::string>(), config.apiServerPort);
  EXPECT_EQ(node["checkedForUpdateAt"].as<uint64_t>(),
            config.checkedForUpdateAt);
  EXPECT_EQ(node["latestRelease"].as<std::string>(), config.latestRelease);
}

TEST_F(CortexConfigTest, FromYaml_ReadsCorrectly) {
  // First, create a valid YAML configuration file
  CortexConfig config = {
      "log_path",
      "default_llamacpp_log_path",
      "default_onnx_log_path",
      "data_path",
      5000,
      "localhost",
      "8080",
      123456789,
      123456789,
      "v1.0.0",

      "",
      "",
      "",
      "",
      "",
      "",
      false,
      {},
      "",
      false,
      false,
      "",
      "",
      "",
      false,
      false,
      "",
      "",
      {},
      0,
      {},
  };

  auto result = cyu::CortexConfigMgr::GetInstance().DumpYamlConfig(
      config, test_file_path);
  EXPECT_FALSE(result.has_error());

  // Now read from the YAML file
  CortexConfig loaded_config = cyu::CortexConfigMgr::GetInstance().FromYaml(
      test_file_path, default_config);

  // Verify that the loaded configuration matches what was written
  EXPECT_EQ(loaded_config.logFolderPath, config.logFolderPath);
  EXPECT_EQ(loaded_config.dataFolderPath, config.dataFolderPath);
  EXPECT_EQ(loaded_config.maxLogLines, config.maxLogLines);
  EXPECT_EQ(loaded_config.apiServerHost, config.apiServerHost);
  EXPECT_EQ(loaded_config.apiServerPort, config.apiServerPort);
  EXPECT_EQ(loaded_config.checkedForUpdateAt, config.checkedForUpdateAt);
  EXPECT_EQ(loaded_config.latestRelease, config.latestRelease);
}

TEST_F(CortexConfigTest, FromYaml_FileNotFound) {
  std::filesystem::remove(test_file_path);  // Ensure the file does not exist

  EXPECT_THROW(
      {
        cyu::CortexConfigMgr::GetInstance().FromYaml(test_file_path,
                                                     default_config);
      },
      std::runtime_error);  // Expect a runtime error due to missing file
}

TEST_F(CortexConfigTest, FromYaml_IncompleteConfigUsesDefaults) {
  // Create an incomplete YAML configuration file
  std::ofstream out_file(test_file_path);
  out_file << "logFolderPath: log_path\n";  // Missing other fields
  out_file.close();

  CortexConfig loaded_config = cyu::CortexConfigMgr::GetInstance().FromYaml(
      test_file_path, default_config);

  // Verify that defaults are used where values are missing
  EXPECT_EQ(loaded_config.logFolderPath, "log_path");
  EXPECT_EQ(loaded_config.dataFolderPath,
            default_config.dataFolderPath);  // Default value
  EXPECT_EQ(loaded_config.maxLogLines,
            default_config.maxLogLines);  // Default value
  EXPECT_EQ(loaded_config.apiServerHost,
            default_config.apiServerHost);  // Default value
  EXPECT_EQ(loaded_config.apiServerPort,
            default_config.apiServerPort);  // Default value
  EXPECT_EQ(loaded_config.checkedForUpdateAt,
            default_config.checkedForUpdateAt);  // Default value
  EXPECT_EQ(loaded_config.latestRelease,
            default_config.latestRelease);  // Default value
}

}  // namespace config_yaml_utils
