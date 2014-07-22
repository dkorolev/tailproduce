// Ensuring gflags is available and built.

#include <gtest/gtest.h>
#include <gflags/gflags.h>

DEFINE_string(question, "", "The question.");
DEFINE_int32(answer, 0, "The answer.");

// The values are set from the command line in the Makefile.
TEST(GFlagsTest, Smoke) {
    EXPECT_EQ("six by nine", FLAGS_question);
    EXPECT_EQ(42, FLAGS_answer);
}
