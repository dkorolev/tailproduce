#include <gtest/gtest.h>

#include "mocks/data_storage.h"

template<typename T> std::vector<uint8_t> bytes(T x) {
    return std::vector<uint8_t>(&x, (&x) + 1);
}

template<> std::vector<uint8_t> bytes(const char* s) {
    return std::vector<uint8_t>(s, s + strlen(s));
}

template<> std::vector<uint8_t> bytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

template<typename T> class DataStorageTest : public ::testing::Test {};

// TODO(dkorolev): Add more data storages here, specifically the LevelDB one once it's ready.
typedef ::testing::Types<MockDataStorage> DataStorageImplementations;
TYPED_TEST_CASE(DataStorageTest, DataStorageImplementations);

TYPED_TEST(DataStorageTest, AddsEntries) {
    TypeParam storage;
    storage.Set(bytes("foo"), bytes("bar"));
    storage.Set(bytes(1), bytes(42));
}

TYPED_TEST(DataStorageTest, DuplicateEntriesDeathTest) {
    TypeParam storage;
    storage.Set(bytes("key"), bytes("old"));
    EXPECT_DEATH(
        storage.Set(bytes("key"), bytes("new")),
        "'key', that is attempted to be set to 'new', has already been set to 'old'\\.");
}

TYPED_TEST(DataStorageTest, BoundedRangeIterator) {
    TypeParam storage;
    storage.Set(bytes("001"), bytes("one"));
    storage.Set(bytes("002"), bytes("two"));
    storage.Set(bytes("003"), bytes("three"));
    storage.Set(bytes("004"), bytes("four"));
    storage.Set(bytes("005"), bytes("five"));
    typename TypeParam::Iterator iterator(storage, bytes("002"), bytes("004"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == bytes("002"));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == bytes("003"));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("four"));
    ASSERT_TRUE(iterator.Key() == bytes("004"));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
}

TYPED_TEST(DataStorageTest, BoundedRangeDynamicIterator) {
    TypeParam storage;
    storage.Set(bytes("x1"), bytes("one"));
    storage.Set(bytes("x2"), bytes("two"));
    typename TypeParam::Iterator iterator(storage, bytes("x2"), bytes("x4"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == bytes("x2"));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    storage.Set(bytes("x3"), bytes("three"));
    storage.Set(bytes("x4"), bytes("four"));
    storage.Set(bytes("x5"), bytes("five"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == bytes("x3"));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("four"));
    ASSERT_TRUE(iterator.Key() == bytes("x4"));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    storage.Set(bytes("x6"), bytes("six"));
    ASSERT_TRUE(iterator.Done());
}

TYPED_TEST(DataStorageTest, SemiBoundedRangeIterator) {
    TypeParam storage;
    storage.Set(bytes(1), bytes("one"));
    storage.Set(bytes(2), bytes("two"));
    storage.Set(bytes(3), bytes("three"));
    storage.Set(bytes(4), bytes("four"));
    storage.Set(bytes(5), bytes("five"));
    typename TypeParam::Iterator iterator(storage, bytes(3));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == bytes(3));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("four"));
    ASSERT_TRUE(iterator.Key() == bytes(4));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("five"));
    ASSERT_TRUE(iterator.Key() == bytes(5));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
}

TYPED_TEST(DataStorageTest, UnboundedRangeDynamicIterator) {
    TypeParam storage;
    typename TypeParam::Iterator iterator(storage);
    ASSERT_TRUE(iterator.Done());
    storage.Set(bytes(101), bytes("this"));
    storage.Set(bytes(102), bytes("too"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("this"));
    ASSERT_TRUE(iterator.Key() == bytes(101));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("too"));
    ASSERT_TRUE(iterator.Key() == bytes(102));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    storage.Set(bytes(103), bytes("shall"));
    storage.Set(bytes(104), bytes("pass"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("shall"));
    ASSERT_TRUE(iterator.Key() == bytes(103));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("pass"));
    ASSERT_TRUE(iterator.Key() == bytes(104));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
}

TYPED_TEST(DataStorageTest, BoundedIteratorOutOfBoundsDeathTest) {
    TypeParam storage;
    typename TypeParam::Iterator iterator(storage, bytes(2), bytes(3));
    ASSERT_TRUE(iterator.Done());
    storage.Set(bytes(1), bytes("one"));
    storage.Set(bytes(2), bytes("two"));
    storage.Set(bytes(3), bytes("three"));
    storage.Set(bytes(4), bytes("four"));
    storage.Set(bytes(5), bytes("five"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == bytes(2));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == bytes(3));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    EXPECT_DEATH(
        iterator.Next(),
        "Attempted to Next\\(\\) an iterator for which Done\\(\\) is true\\.");
}

TYPED_TEST(DataStorageTest, UnboundedIteratorOutOfBoundsDeathTest) {
    TypeParam storage;
    typename TypeParam::Iterator iterator(storage, bytes(2));
    ASSERT_TRUE(iterator.Done());
    storage.Set(bytes(1), bytes("one"));
    ASSERT_TRUE(iterator.Done());
    storage.Set(bytes(2), bytes("two"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == bytes(2));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    storage.Set(bytes(3), bytes("three"));
    storage.Set(bytes(4), bytes("four"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == bytes(3));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("four"));
    ASSERT_TRUE(iterator.Key() == bytes(4));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    EXPECT_DEATH(
        iterator.Next(),
        "Attempted to Next\\(\\) an iterator for which Done\\(\\) is true\\.");
}
