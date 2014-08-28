#include <gtest/gtest.h>

#include "../../src/config_values.h"

using ::TailProduce::ConfigValues;

struct StreamTraits {
    std::string name = "foo";
};
StreamTraits stream_traits;

TEST(ConfigValues, StandardSettings) {
    ConfigValues cv("s", "d", ':');
    EXPECT_EQ("s:foo", cv.HeadStorageKey(stream_traits));
    EXPECT_EQ("d:foo;", cv.EndDataStorageKey(stream_traits));
    EXPECT_EQ("s:foo:", cv.GetStreamMetaPrefix(stream_traits));
    EXPECT_EQ("d:foo:", cv.GetStreamDataPrefix(stream_traits));
}

TEST(ConfigValues, NonStandardSettings) {
    ConfigValues cv("Stream", "Data", '.');
    EXPECT_EQ("Stream.foo", cv.HeadStorageKey(stream_traits));
    EXPECT_EQ("Data.foo/", cv.EndDataStorageKey(stream_traits));
    EXPECT_EQ("Stream.foo.", cv.GetStreamMetaPrefix(stream_traits));
    EXPECT_EQ("Data.foo.", cv.GetStreamDataPrefix(stream_traits));
}
