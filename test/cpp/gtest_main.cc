#include <gtest/gtest.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    google::InitGoogleLogging(argv[0]);
    if (!google::ParseCommandLineFlags(&argc, &argv, true)) {
        return -1;
    }
    return RUN_ALL_TESTS();
}
