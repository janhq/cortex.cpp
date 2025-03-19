#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "utils/engine_matcher_utils.h"

class EngineMatcherUtilsTestSuite : public ::testing::Test {
 protected:
  const std::vector<std::string> cortex_llamacpp_variants{
      "llama-b4920-bin-ubuntu-arm64.zip",
      "llama-b4920-bin-linux-avx-cuda-cu11.7-x64.tar.gz",
      "llama-b4920-bin-linux-avx-cuda-cu12.0-x64.tar.gz",
      "llama-b4920-bin-linux-avx-x64.tar.gz",
      "llama-b4920-bin-linux-avx2-cuda-cu11.7-x64.tar.gz",
      "llama-b4920-bin-linux-avx2-cuda-cu12.0-x64.tar.gz",
      "llama-b4920-bin-ubuntu-x64.tar.gz",
      "llama-b4920-bin-linux-avx512-cuda-cu11.7-x64.tar.gz",
      "llama-b4920-bin-linux-avx512-cuda-cu12.0-x64.tar.gz",
      "llama-b4920-bin-linux-avx512-x64.tar.gz",
      "llama-b4920-bin-linux-noavx-cuda-cu11.7-x64.tar.gz",
      "llama-b4920-bin-linux-noavx-cuda-cu12.0-x64.tar.gz",
      "llama-b4920-bin-linux-noavx-x64.tar.gz",
      "llama-b4920-bin-ubuntu-vulkan-x64.tar.gz",
      "llama-b4920-bin-macos-arm64.zip",
      "llama-b4920-bin-macos-x64.zip",
      "llama-b4920-bin-windows-amd64-avx-cuda-11-7.tar.gz",
      "llama-b4920-bin-windows-amd64-avx-cuda-12-0.tar.gz",
      "llama-b4920-bin-win-avx-x64.zip",
      "llama-b4920-bin-windows-amd64-avx2-cuda-11-7.tar.gz",
      "llama-b4920-bin-windows-amd64-avx2-cuda-12-0.tar.gz",
      "llama-b4920-bin-win-avx2-x64.zip",
      "llama-b4920-bin-windows-amd64-avx512-cuda-11-7.tar.gz",
      "llama-b4920-bin-windows-amd64-avx512-cuda-12-0.tar.gz",
      "llama-b4920-bin-win-avx512-x64.zip",
      "llama-b4920-bin-windows-amd64-noavx-cuda-11-7.tar.gz",
      "llama-b4920-bin-windows-amd64-noavx-cuda-12-0.tar.gz",
      "llama-b4920-bin-win-noavx-x64.zip",
      "llama-b4920-bin-win-vulkan-x64.zip",
  };
};

TEST_F(EngineMatcherUtilsTestSuite, TestValidate) {
  {
    auto os{"win"};
    auto cpu_arch{"amd64"};
    auto suitable_avx{"avx2"};
    auto cuda_version{"12.4"};

    auto variant = engine_matcher_utils::Validate(
        cortex_llamacpp_variants, os, cpu_arch, suitable_avx, cuda_version);

    EXPECT_EQ(variant, "llama-b4920-bin-windows-amd64-avx2-cuda-12-0.tar.gz");
  }

  {
    auto os{"mac"};
    auto cpu_arch{"x64"};
    auto suitable_avx{""};
    auto cuda_version{""};

    auto variant = engine_matcher_utils::Validate(
        cortex_llamacpp_variants, os, cpu_arch, suitable_avx, cuda_version);

    EXPECT_EQ(variant, "llama-b4920-bin-macos-x64.zip");
  }

  {
    auto os{"win"};
    auto cpu_arch{"x64"};
    auto suitable_avx{"avx2"};
    auto cuda_version{"10"};

    auto variant = engine_matcher_utils::Validate(
        cortex_llamacpp_variants, os, cpu_arch, suitable_avx, cuda_version);

    // fallback to no cuda version
    EXPECT_EQ(variant, "llama-b4920-bin-win-avx2-x64.zip");
  }

  {
    auto os{"linux"};
    auto cpu_arch{"arm64"};
    auto suitable_avx{""};
    auto cuda_version{""};

    auto variant = engine_matcher_utils::Validate(
        cortex_llamacpp_variants, os, cpu_arch, suitable_avx, cuda_version);

    EXPECT_EQ(variant, "llama-b4920-bin-ubuntu-arm64.zip");
  }
}

TEST_F(EngineMatcherUtilsTestSuite, TestGetVersionAndArch) {
  {
    std::string variant = "llama-b4920-bin-linux-avx-cuda-cu11.7-x64.tar.gz";
    auto [version, arch] = engine_matcher_utils::GetVersionAndArch(variant);
    EXPECT_EQ(version, "b4920");
    EXPECT_EQ(arch, "linux-avx-cuda-cu11.7-x64");
  }

  {
    std::string variant = "llama-b4920-bin-ubuntu-arm64.zip";
    auto [version, arch] = engine_matcher_utils::GetVersionAndArch(variant);
    EXPECT_EQ(version, "b4920");
    EXPECT_EQ(arch, "ubuntu-arm64");
  }

  {
    std::string variant = "llama-b4920-bin-win-avx2-x64.zip";
    auto [version, arch] = engine_matcher_utils::GetVersionAndArch(variant);
    EXPECT_EQ(version, "b4920");
    EXPECT_EQ(arch, "win-avx2-x64");
  }

  {
    std::string variant = "llama-b4920-bin-macos-x64.tar.gz";
    auto [version, arch] = engine_matcher_utils::GetVersionAndArch(variant);
    EXPECT_EQ(version, "b4920");
    EXPECT_EQ(arch, "macos-x64");
  }
}
