// A tree-minutes intro to gtest.
//
// 0) gtest ships as source code and should be compiled per-project.
//    Makefile in this directory does it.
//
//    Installation instructions for Ubuntu:
//    $ sudo -s
//    # apt-get install libgtest-dev
//    # cd /usr/src/gtest
//    # cmake .
//    # make
//    # cp libg* /usr/lib/
//    # logout
//    $
//
// 1) ASSERT_* interrupts the TEST() { .. } if failing,
//    EXPECT_* considers the TEST() failed, but continues to execute it.
//
// 2) ASSERT-s and EXPECT-s can be used as output streams. No newline needed.
//    EXPECT_EQ(4, 2 * 2) << "Something is wrong with multiplication."
//
// 3) ASSERT-s and EXPECT-s can use {EQ,NE,LT,GT,LE,NE,TRUE} after the underscore.
//    This results in more meaningful messages.
//
// 4) For {ASSERT,EXPECT}_{EQ,NE}, put the expected value as the first parameter.
//    For cleaner error messages.
//
// 5) ASSERT_THROW(statement, exception_type) ensures the exception is thrown.
//
// 6) ASSERT_DEATH(function(), "Expected regex for the last line of standard error.")
//    can be used to ensure certain call fails. The convention is to use
//    the "DeathTest" suffix for those tests and to not mix functional tests with death tests.
//
// 7) gtest supports templated tests, where objects of various tests are passed to the same test method.
//    Each type results in the whole new statically compiled test.
//
// 8) Prefix a test name with "DISABLED_" to exclude it from being run.
//    Use sparingly and try to keep master clean from disabled tests.

#include <vector>
#include <queue>

#include <gtest/gtest.h>
#include <glog/logging.h>

// Basic test syntax.
TEST(GTestTest, Trivial) {
    EXPECT_EQ(42, 42);
    ASSERT_TRUE(42 == 42);
}

// Various comparisons.
TEST(GTestTest, Passing) {
    EXPECT_EQ(4, 2 + 2);
    EXPECT_GT(2 * 2, 3);
    EXPECT_GE(2 * 2, 4);
    EXPECT_LE(2 * 2, 4);
    EXPECT_LT(2 * 2, 5);
    EXPECT_NE(2 * 2, 0);
    EXPECT_TRUE(2 * 2 == 4);
    EXPECT_FALSE(2 * 2 != 4);
}

// A simple death test.
TEST(GTestTest, DeathTest) {
    auto die = []() {
        // More on google-glog: http://google-glog.googlecode.com/svn/trunk/doc/glog.html
        const int tmp = 42;
        LOG(FATAL) << "Example " << tmp << " error message.";
    };
    ASSERT_DEATH(die(), "Example .* message\\.");
}

// An example of a test throwing an exception.
TEST(GTestTest, ExceptionThrowingTest) {
    struct FooException {};
    auto f = [](int a) {
        if (a < 0) throw FooException();
    };
    f(+1);
    ASSERT_THROW(f(-1), FooException);
}

// An example of a templated test.
// https://code.google.com/p/googletest/wiki/AdvancedGuide
// https://code.google.com/p/googletest/source/browse/trunk/samples/sample6_unittest.cc
template <typename T> class GTestTemplatedTest : public ::testing::Test {};

// The typedef is necessary for the TYPED_TEST_CASE macro to parse correctly.
typedef ::testing::Types<std::vector<int>, std::deque<int>> ContainersTypeList;
TYPED_TEST_CASE(GTestTemplatedTest, ContainersTypeList);

// Tests that std::vector<T> and std::deque<T> support both push_back() and pop_back().
TYPED_TEST(GTestTemplatedTest, TemplatedContainersTest) {
    TypeParam container;
    EXPECT_TRUE(container.empty());
    EXPECT_EQ(0, container.size());
    container.push_back(42);
    EXPECT_FALSE(container.empty());
    EXPECT_EQ(1, container.size());
    EXPECT_EQ(42, container.back());
    container.pop_back();
    EXPECT_TRUE(container.empty());
    EXPECT_EQ(0, container.size());
}

// An example of a failing test.
// It is both marked as DISABLED and guarded by #if false
// to not pollute the output with the "YOU HAVE 1 DISABLED TEST" message.

#if false
TEST(GTestTest, DISABLED_Failing) { 
    EXPECT_EQ(5, 2 + 2) << "Fails as expected: (2 + 2) is " << 2 + 2 << ", not 5.";
    EXPECT_TRUE(false) << "Fails as expected: false is not true.";
    EXPECT_FALSE(true) << "Fails as expected: true is not false.";
}
#endif
