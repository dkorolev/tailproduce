#include <gtest/gtest.h>

#include "../../src/helpers.h"

#include "mocks/data_storage.h"

template<typename T> class DataStorageTest : public ::testing::Test {};

// TODO(dkorolev): Add more data storages here, specifically the LevelDB one once it's ready.
typedef ::testing::Types<MockDataStorage> DataStorageImplementations;
TYPED_TEST_CASE(DataStorageTest, DataStorageImplementations);

TYPED_TEST(DataStorageTest, AddsEntries) {
    TypeParam storage;
    storage.Set("foo", bytes("bar"));
    storage.Set("1", bytes(42));
}

TYPED_TEST(DataStorageTest, DuplicateEntriesDeathTest) {
    TypeParam storage;
    storage.Set("key", bytes("old"));
    EXPECT_DEATH(
        storage.Set("key", bytes("new")),
        "'key', that is attempted to be set to 'new', has already been set to 'old'\\.");
}

TYPED_TEST(DataStorageTest, SetsOverwritesAndGets) {
    TypeParam storage;
    storage.Set("key", bytes("first"));
    EXPECT_EQ(bytes("first"), storage.Get("key"));
    storage.SetAllowingOverwrite("key", bytes("second"));
    EXPECT_EQ(bytes("second"), storage.Get("key"));
}

TYPED_TEST(DataStorageTest, BoundedRangeIterator) {
    TypeParam storage;
    storage.Set("001", bytes("one"));
    storage.Set("002", bytes("two"));
    storage.Set("003", bytes("three"));
    storage.Set("004", bytes("four"));
    storage.Set("005", bytes("five"));
    typename TypeParam::Iterator iterator = storage.GetIterator("002", "004");
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == "002");
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == "003");
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("four"));
    ASSERT_TRUE(iterator.Key() == "004");
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
}

TYPED_TEST(DataStorageTest, BoundedRangeDynamicIterator) {
    TypeParam storage;
    storage.Set("x1", bytes("one"));
    storage.Set("x2", bytes("two"));
    typename TypeParam::Iterator iterator = storage.GetIterator("x2", "x4");
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == "x2");
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    storage.Set("x3", bytes("three"));
    storage.Set("x4", bytes("four"));
    storage.Set("x5", bytes("five"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == "x3");
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("four"));
    ASSERT_TRUE(iterator.Key() == "x4");
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    storage.Set("x6", bytes("six"));
    ASSERT_TRUE(iterator.Done());
}

TYPED_TEST(DataStorageTest, SemiBoundedRangeIterator) {
    TypeParam storage;
    storage.Set("1", bytes("one"));
    storage.Set("2", bytes("two"));
    storage.Set("3", bytes("three"));
    storage.Set("4", bytes("four"));
    storage.Set("5", bytes("five"));
    typename TypeParam::Iterator iterator = storage.GetIterator("3");
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == "3");
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("four"));
    ASSERT_TRUE(iterator.Key() == "4");
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("five"));
    ASSERT_TRUE(iterator.Key() == "5");
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
}

TYPED_TEST(DataStorageTest, UnboundedRangeDynamicIterator) {
    TypeParam storage;
    typename TypeParam::Iterator iterator = storage.GetIterator();
    ASSERT_TRUE(iterator.Done());
    storage.Set("101", bytes("this"));
    storage.Set("102", bytes("too"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("this"));
    ASSERT_TRUE(iterator.Key() == "101");
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("too"));
    ASSERT_TRUE(iterator.Key() == "102");
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    storage.Set("103", bytes("shall"));
    storage.Set("104", bytes("pass"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("shall"));
    ASSERT_TRUE(iterator.Key() == "103");
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("pass"));
    ASSERT_TRUE(iterator.Key() == "104");
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
}

TYPED_TEST(DataStorageTest, BoundedIteratorOutOfBoundsDeathTest) {
    TypeParam storage;
    typename TypeParam::Iterator iterator = storage.GetIterator("2", "3");
    ASSERT_TRUE(iterator.Done());
    storage.Set("1", bytes("one"));
    storage.Set("2", bytes("two"));
    storage.Set("3", bytes("three"));
    storage.Set("4", bytes("four"));
    storage.Set("5", bytes("five"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == "2");
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == "3");
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    EXPECT_DEATH(
        iterator.Next(),
        "Attempted to Next\\(\\) an iterator for which Done\\(\\) is true\\.");
}

TYPED_TEST(DataStorageTest, UnboundedIteratorOutOfBoundsDeathTest) {
    TypeParam storage;
    typename TypeParam::Iterator iterator = storage.GetIterator("2");
    ASSERT_TRUE(iterator.Done());
    storage.Set("1", bytes("one"));
    ASSERT_TRUE(iterator.Done());
    storage.Set("2", bytes("two"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == "2");
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    storage.Set("3", bytes("three"));
    storage.Set("4", bytes("four"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == "3");
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("four"));
    ASSERT_TRUE(iterator.Key() == "4");
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    EXPECT_DEATH(
        iterator.Next(),
        "Attempted to Next\\(\\) an iterator for which Done\\(\\) is true\\.");
}

