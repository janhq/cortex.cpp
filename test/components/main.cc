#include "gtest/gtest.h"
#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
