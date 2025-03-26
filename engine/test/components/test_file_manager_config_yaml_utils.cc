#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "utils/config_yaml_utils.h"
#include "utils/file_manager_utils.h"

// Mock for filesystem operations

// Test fixture
class FileManagerConfigTest : public ::testing::Test {};

// Tests for file_manager_utils

TEST_F(FileManagerConfigTest, GetExecutableFolderContainerPath) {
  auto path = file_manager_utils::GetExecutableFolderContainerPath();
  EXPECT_FALSE(path.empty());
  EXPECT_TRUE(std::filesystem::is_directory(path));
}

TEST_F(FileManagerConfigTest, GetHomeDirectoryPath) {
  auto path = file_manager_utils::GetHomeDirectoryPath();
  EXPECT_FALSE(path.empty());
  EXPECT_TRUE(std::filesystem::is_directory(path));
}

TEST_F(FileManagerConfigTest, GetConfigurationPath) {
  auto path = file_manager_utils::GetConfigurationPath();
  EXPECT_FALSE(path.empty());
  EXPECT_TRUE(path.has_filename());
}

TEST_F(FileManagerConfigTest, GetDefaultDataFolderName) {
  auto folder_name = file_manager_utils::GetDefaultDataFolderName();
  EXPECT_FALSE(folder_name.empty());
  EXPECT_TRUE(folder_name.find("cortexcpp") != std::string::npos);
}

TEST_F(FileManagerConfigTest, CreateConfigFileIfNotExist) {

  auto result = file_manager_utils::CreateConfigFileIfNotExist();
  EXPECT_FALSE(result.has_error());
  EXPECT_TRUE(
      std::filesystem::exists(file_manager_utils::GetConfigurationPath()));
  std::filesystem::remove(file_manager_utils::GetConfigurationPath());
}

TEST_F(FileManagerConfigTest, GetCortexConfig) {
  auto result = file_manager_utils::CreateConfigFileIfNotExist();
  EXPECT_FALSE(result.has_error());
  auto config = file_manager_utils::GetCortexConfig();
  EXPECT_FALSE(config.dataFolderPath.empty());
  EXPECT_FALSE(config.logFolderPath.empty());
  EXPECT_GT(config.maxLogLines, 0);
}

// Tests for config_yaml_utils

TEST_F(FileManagerConfigTest, DumpYamlConfig) {
  config_yaml_utils::CortexConfig config{
      /* .logFolderPath = */ "/path/to/logs",
      /* .logLlamaCppPath = */ file_manager_utils::kLogsLlamacppBaseName,
      /* .logOnnxPath = */ file_manager_utils::kLogsOnnxBaseName,
      /* .dataFolderPath = */ "/path/to/data",
      /* .maxLogLines = */ 1000,
      /* .apiServerHost = */ "localhost",
      /* .apiServerPort = */ "8080",

      /* .checkedForUpdateAt = */ config_yaml_utils::kDefaultCheckedForUpdateAt,
      /* .checkedForLlamacppUpdateAt = */
          config_yaml_utils::kDefaultCheckedForLlamacppUpdateAt,
      /* .latestRelease = */ config_yaml_utils::kDefaultLatestRelease,
      /* .latestLlamacppRelease = */ config_yaml_utils::kDefaultLatestLlamacppRelease,
      /* .huggingFaceToken = */ "",
      /* .gitHubUserAgent = */ "",
      /* .gitHubToken = */ "",
      /* .llamacppVariant = */ "",
      /* .llamacppVersion = */ "",
      /* .enableCors = */ config_yaml_utils::kDefaultCorsEnabled,
      /* .allowedOrigins = */ config_yaml_utils::kDefaultEnabledOrigins,
      /* .proxyUrl = */ "",
      /* .verifyProxySsl = */ true,
      /* .verifyProxyHostSsl = */ true,
      /* .proxyUsername = */ "",
      /* .proxyPassword = */ "",
      /* .noProxy = */ config_yaml_utils::kDefaultNoProxy,
      /* .verifyPeerSsl = */ true,
      /* .verifyHostSsl = */ true,

      /* .sslCertPath = */ "",
      /* .sslKeyPath = */ "",
      /* .supportedEngines = */ config_yaml_utils::kDefaultSupportedEngines,
      /* .checkedForSyncHubAt = */ 0u,
      /* .apiKeys = */ {},
	};

  std::string test_file = "test_config.yaml";
  auto result =
      config_yaml_utils::CortexConfigMgr::GetInstance().DumpYamlConfig(
          config, test_file);
  EXPECT_FALSE(result.has_error());
  EXPECT_TRUE(std::filesystem::exists(test_file));

  // Clean up
  std::filesystem::remove(test_file);
}

TEST_F(FileManagerConfigTest, FromYaml) {
  // Create a test YAML file
  std::string test_file = "test_config.yaml";
  std::ofstream out_file(test_file);
  out_file << "logFolderPath: /path/to/logs\n"
           << "dataFolderPath: /path/to/data\n"
           << "maxLogLines: 1000\n"
           << "apiServerHost: localhost\n"
           << "apiServerPort: '8080'\n";
  out_file.close();

  config_yaml_utils::CortexConfig default_config{};
  auto config = config_yaml_utils::CortexConfigMgr::GetInstance().FromYaml(
      test_file, default_config);

  EXPECT_EQ(config.logFolderPath, "/path/to/logs");
  EXPECT_EQ(config.dataFolderPath, "/path/to/data");
  EXPECT_EQ(config.maxLogLines, 1000);
  EXPECT_EQ(config.apiServerHost, "localhost");
  EXPECT_EQ(config.apiServerPort, "8080");

  // Clean up
  std::filesystem::remove(test_file);
}
