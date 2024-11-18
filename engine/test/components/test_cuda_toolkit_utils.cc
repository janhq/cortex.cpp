#include <gtest/gtest.h>
#include "utils/cuda_toolkit_utils.h"

// Test fixture for cuda_toolkit_utils
class CudaToolkitUtilsTest : public ::testing::Test {};

// Tests for cuda_toolkit_utils

TEST_F(CudaToolkitUtilsTest, WindowsCompatibleVersions) {
  EXPECT_EQ("12.4", cuda_toolkit_utils::GetCompatibleCudaToolkitVersion(
                        "527.41", "windows", ""));
  EXPECT_EQ("11.7", cuda_toolkit_utils::GetCompatibleCudaToolkitVersion(
                        "452.39", "windows", ""));
}

TEST_F(CudaToolkitUtilsTest, LinuxCompatibleVersions) {
  EXPECT_EQ("12.4", cuda_toolkit_utils::GetCompatibleCudaToolkitVersion(
                        "525.60.13", "linux", ""));
  EXPECT_EQ("11.7", cuda_toolkit_utils::GetCompatibleCudaToolkitVersion(
                        "450.80.02", "linux", ""));
}

TEST_F(CudaToolkitUtilsTest, TensorRTLLMEngine) {
  EXPECT_EQ("12.4", cuda_toolkit_utils::GetCompatibleCudaToolkitVersion(
                        "527.41", "windows", "cortex.tensorrt-llm"));
  EXPECT_EQ("12.4", cuda_toolkit_utils::GetCompatibleCudaToolkitVersion(
                        "525.60.13", "linux", "cortex.tensorrt-llm"));
}

TEST_F(CudaToolkitUtilsTest, UnsupportedDriverVersion) {
  EXPECT_THROW(cuda_toolkit_utils::GetCompatibleCudaToolkitVersion(
                   "450.00", "windows", ""),
               std::runtime_error);
  EXPECT_THROW(cuda_toolkit_utils::GetCompatibleCudaToolkitVersion("450.00",
                                                                   "linux", ""),
               std::runtime_error);
}

TEST_F(CudaToolkitUtilsTest, UnsupportedOS) {
  EXPECT_THROW(cuda_toolkit_utils::GetCompatibleCudaToolkitVersion("527.41",
                                                                   "macos", ""),
               std::runtime_error);
}
