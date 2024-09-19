#include "gtest/gtest.h"
#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include <fstream>
#include <cstdio>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

class GGUFParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        gguf_handler = std::make_unique<config::GGUFHandler>();
        yaml_handler = std::make_unique< config::YamlHandler>();
    }

    void TearDown() override {
    }

    std::unique_ptr<config::GGUFHandler> gguf_handler;
    std::unique_ptr<config::YamlHandler> yaml_handler;

    std::string getTempFilePath(const std::string& prefix, const std::string& extension) {
    #ifdef _WIN32
        char temp_path[MAX_PATH];
        char file_name[MAX_PATH];
        GetTempPathA(MAX_PATH, temp_path);
        GetTempFileNameA(temp_path, prefix.c_str(), 0, file_name);
        std::string path(file_name);
        DeleteFileA(file_name);  // Delete the file created by GetTempFileNameA
        return path + extension;
    #else
        std::string path = "/tmp/" + prefix + "XXXXXX" + extension;
        char* temp = strdup(path.c_str());
        int fd = mkstemps(temp, extension.length());
        if (fd == -1) {
            free(temp);
            throw std::runtime_error("Failed to create temporary file");
        }
        close(fd);
        std::string result(temp);
        free(temp);
        return result;
    #endif
    }

    std::string createMockGGUFFile() {
        std::string gguf_path = getTempFilePath("mock_tinyllama-model", ".gguf");
        std::ofstream file(gguf_path, std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to create mock GGUF file");
        }

        try {
            // GGUF magic number
            uint32_t magic = 0x46554747;
            file.write(reinterpret_cast<char*>(&magic), sizeof(magic));

            // Version
            uint32_t version = 2;
            file.write(reinterpret_cast<char*>(&version), sizeof(version));

            // Tensor count (not important for this test)
            uint64_t tensor_count = 0;
            file.write(reinterpret_cast<char*>(&tensor_count), sizeof(tensor_count));

            // Metadata key-value count
            uint64_t kv_count = 2;
            file.write(reinterpret_cast<char*>(&kv_count), sizeof(kv_count));

            // Helper function to write a string
            auto writeString = [&file](const std::string& str) {
                uint64_t length = str.length();
                file.write(reinterpret_cast<char*>(&length), sizeof(length));
                file.write(str.c_str(), length);
            };

            // Helper function to write a key-value pair
            auto writeKV = [&](const std::string& key, uint32_t type, const auto& value) {
                writeString(key);
                file.write(reinterpret_cast<const char*>(&type), sizeof(type));
                if constexpr (std::is_same_v<decltype(value), const std::string&>) {
                    writeString(value);
                } else {
                    file.write(reinterpret_cast<const char*>(&value), sizeof(value));
                }
            };

            // Write metadata
            writeKV("general.name", 8, std::string("tinyllama 1B"));
            writeKV("llama.context_length", 4, uint32_t(4096));

            file.close();

        } catch (const std::exception& e) {
            file.close();
            std::remove(gguf_path.c_str());
            throw std::runtime_error(std::string("Failed to write mock GGUF file: ") + e.what());
        }

        return gguf_path;
    }
};

TEST_F(GGUFParserTest, ParseMockTinyLlamaModel) {
    std::string gguf_path;
    std::string yaml_path;
    try {
        // Create a mock GGUF file
        gguf_path = createMockGGUFFile();

        // Parse the GGUF file
        gguf_handler->Parse(gguf_path);

        const config::ModelConfig& gguf_config = gguf_handler->GetModelConfig();

        // Load the expected configuration from YAML
        std::string yaml_content = R"(
name: tinyllama-1B
ctx_len: 4096
        )";

        yaml_path = getTempFilePath("expected_config", ".yaml");
        std::ofstream yaml_file(yaml_path);
        yaml_file << yaml_content;
        yaml_file.close();

        yaml_handler->ModelConfigFromFile(yaml_path);

        const config::ModelConfig& yaml_config = yaml_handler->GetModelConfig();

        // Compare GGUF parsed config with YAML config
        EXPECT_EQ(gguf_config.name, yaml_config.name);
        EXPECT_EQ(gguf_config.ctx_len, yaml_config.ctx_len);

        // Clean up
        std::remove(gguf_path.c_str());
        std::remove(yaml_path.c_str());
    }
    catch (const std::exception& e) {
        // If an exception was thrown, make sure to clean up the files
        if (!gguf_path.empty()) {
            std::remove(gguf_path.c_str());
        }
        if (!yaml_path.empty()) {
            std::remove(yaml_path.c_str());
        }
        FAIL() << "Exception thrown: " << e.what();
    }
}