#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "utils/engine_matcher_utils.h"

class EngineMatcherUtilsTestSuite : public ::testing::Test {
 protected:
  const std::vector<std::string> cortex_llamacpp_variants{
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx-cuda-11-7.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx-cuda-12-0.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx2-cuda-11-7.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx2-cuda-12-0.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx2.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx512-cuda-11-7.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx512-cuda-12-0.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx512.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-noavx-cuda-11-7.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-noavx-cuda-12-0.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-noavx.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-vulkan.tar.gz",
      "cortex.llamacpp-0.1.43-linux-arm64.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-mac-amd64.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-mac-arm64.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx-cuda-11-7.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx-cuda-12-0.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx2-cuda-11-7.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx2-cuda-12-0.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx2.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx512-cuda-11-7.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx512-cuda-12-0.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx512.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-noavx-cuda-11-7.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-noavx-cuda-12-0.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-noavx.tar.gz",
      "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-vulkan.tar.gz",
  };

  const std::vector<std::string> cortex_tensorrt_variants{
      "cortex.tensorrt-llm-0.0.9-linux-cuda-12-4.tar.gz",
      "cortex.tensorrt-llm-0.0.9-windows-cuda-12-4.tar.gz"};

  const std::vector<std::string> cortex_onnx_variants{
      "cortex.onnx-0.1.7-windows-amd64.tar.gz"};
};

TEST_F(EngineMatcherUtilsTestSuite, TestValidateOnnx) {

  {
    auto expect_matched_variant = cortex_onnx_variants[0];
    auto result = engine_matcher_utils::ValidateOnnx(cortex_onnx_variants,
                                                     "windows", "amd64");

    EXPECT_EQ(result, expect_matched_variant);
  }

  {
    // should return an empty variant because no variant matched
    auto expect_matched_variant{""};
    auto windows_arm_result = engine_matcher_utils::ValidateOnnx(
        cortex_onnx_variants, "windows", "arm");
    auto mac_arm64_result = engine_matcher_utils::ValidateOnnx(
        cortex_onnx_variants, "mac", "arm64");

    EXPECT_EQ(windows_arm_result, expect_matched_variant);
    EXPECT_EQ(mac_arm64_result, expect_matched_variant);
  }
}

TEST_F(EngineMatcherUtilsTestSuite, TestValidateTensorrt) {

  {
    auto windows_expect_matched_variant{cortex_tensorrt_variants[1]};
    auto linux_expect_matched_variant{cortex_tensorrt_variants[0]};
    auto windows{"windows"};
    auto linux{"linux"};
    auto cuda_version{"12.4"};
    auto windows_result = engine_matcher_utils::ValidateTensorrtLlm(
        cortex_tensorrt_variants, windows, cuda_version);
    auto linux_result = engine_matcher_utils::ValidateTensorrtLlm(
        cortex_tensorrt_variants, linux, cuda_version);

    EXPECT_EQ(windows_result, windows_expect_matched_variant);
    EXPECT_EQ(linux_result, linux_expect_matched_variant);
  }

  {  // macos is not supported
    auto os = "mac";
    auto cuda_version{"12.4"};

    auto result = engine_matcher_utils::ValidateTensorrtLlm(
        cortex_tensorrt_variants, os, cuda_version);
    EXPECT_EQ(result, "");
  }
}

TEST_F(EngineMatcherUtilsTestSuite, TestValidate) {
  {
    auto os{"win"};
    auto cpu_arch{"amd64"};
    auto suitable_avx{"avx2"};
    auto cuda_version{"12.4"};

    auto variant = engine_matcher_utils::Validate(
        cortex_llamacpp_variants, os, cpu_arch, suitable_avx, cuda_version);

    EXPECT_EQ(
        variant,
        "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx2-cuda-12-0.tar.gz");
  }

  {
    auto os{"mac"};
    auto cpu_arch{"amd64"};
    auto suitable_avx{""};
    auto cuda_version{""};

    auto variant = engine_matcher_utils::Validate(
        cortex_llamacpp_variants, os, cpu_arch, suitable_avx, cuda_version);

    EXPECT_EQ(variant, "cortex.llamacpp-0.1.25-25.08.24-mac-amd64.tar.gz");
  }

  {
    auto os{"win"};
    auto cpu_arch{"amd64"};
    auto suitable_avx{"avx2"};
    auto cuda_version{"10"};

    auto variant = engine_matcher_utils::Validate(
        cortex_llamacpp_variants, os, cpu_arch, suitable_avx, cuda_version);

    // fallback to no cuda version
    EXPECT_EQ(variant,
              "cortex.llamacpp-0.1.25-25.08.24-windows-amd64-avx2.tar.gz");
  }

  {
    auto os{"linux"};
    auto cpu_arch{"arm64"};
    auto suitable_avx{""};
    auto cuda_version{""};

    auto variant = engine_matcher_utils::Validate(
        cortex_llamacpp_variants, os, cpu_arch, suitable_avx, cuda_version);

    EXPECT_EQ(variant, "cortex.llamacpp-0.1.43-linux-arm64.tar.gz");
  }
}

TEST_F(EngineMatcherUtilsTestSuite, TestGetVersionAndArch) {
  {
    std::string variant =
        "cortex.llamacpp-0.1.25-25.08.24-linux-amd64-avx-cuda-11-7.tar.gz";
    auto [version, arch] = engine_matcher_utils::GetVersionAndArch(variant);
    EXPECT_EQ(version, "v0.1.25-25.08.24");
    EXPECT_EQ(arch, "linux-amd64-avx-cuda-11-7");
  }

  {
    std::string variant = "cortex.llamacpp-0.1.25-windows-amd64-avx2.tar.gz";
    auto [version, arch] = engine_matcher_utils::GetVersionAndArch(variant);
    EXPECT_EQ(version, "v0.1.25");
    EXPECT_EQ(arch, "windows-amd64-avx2");
  }

  {
    std::string variant = "cortex.llamacpp-0.1.25-25.08.24-mac-amd64.tar.gz";
    auto [version, arch] = engine_matcher_utils::GetVersionAndArch(variant);
    EXPECT_EQ(version, "v0.1.25-25.08.24");
    EXPECT_EQ(arch, "mac-amd64");
  }
}
