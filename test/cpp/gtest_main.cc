#include <gtest/gtest.h>
#include <gflags/gflags.h>

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    if (!google::ParseCommandLineFlags(&argc, &argv, true)) {
        return -1;
    }
    return RUN_ALL_TESTS();
}
