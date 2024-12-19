#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>
#include "gtest/gtest.h"

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
#if defined(NDEBUG)
  ::testing::GTEST_FLAG(filter) = "-FileManagerConfigTest.*";
  int ret = RUN_ALL_TESTS();
  if (ret != 0)
    return ret;
  ::testing::GTEST_FLAG(filter) = "FileManagerConfigTest.*";
  ret = RUN_ALL_TESTS();
#else
  int ret = RUN_ALL_TESTS();
#endif
  return ret;
}
