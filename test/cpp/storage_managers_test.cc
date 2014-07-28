#include <gtest/gtest.h>

#include "helpers/storages.h"

using ::TailProduce::bytes;
using ::TailProduce::antibytes;

template <typename T> class DataStorageTest : public ::testing::Test {};
TYPED_TEST_CASE(DataStorageTest, TestDataStorageImplementationsTypeList);

// TODO(dkorolev): Add similar tests to other files.
TYPED_TEST(DataStorageTest, HasRightBaseClass) {
    typedef ::TailProduce::Storage S;
    static_assert(std::is_base_of<S, TypeParam>::value, "Storage must inherit from ::TailProduce::Storage.");
}

TYPED_TEST(DataStorageTest, AddsEntries) {
    TypeParam storage;
    EXPECT_FALSE(storage.Has("foo"));
    EXPECT_FALSE(storage.Has("1"));
    storage.Set("foo", bytes("bar"));
    EXPECT_TRUE(storage.Has("foo"));
    EXPECT_FALSE(storage.Has("1"));
    storage.Set("1", bytes(42));
    EXPECT_TRUE(storage.Has("foo"));
    EXPECT_TRUE(storage.Has("1"));
}

TYPED_TEST(DataStorageTest, DuplicateEntries) {
    TypeParam storage;
    storage.Set("key", bytes("old"));
    ASSERT_THROW(storage.Set("key", bytes("new")), ::TailProduce::StorageOverwriteNotAllowedException);
}

TYPED_TEST(DataStorageTest, SetsOverwritesAndGets) {
    TypeParam storage;
    storage.Set("key", bytes("first"));
    EXPECT_EQ(bytes("first"), storage.Get("key"));
    storage.SetAllowingOverwrite("key", bytes("second"));
    EXPECT_EQ(bytes("second"), storage.Get("key"));
}

TYPED_TEST(DataStorageTest, BasicExceptions) {
    TypeParam storage;
    ASSERT_THROW(storage.Set("", bytes("foo")), ::TailProduce::StorageEmptyKeyException);
    ASSERT_THROW(storage.Set("bar", bytes("")), ::TailProduce::StorageEmptyValueException);
    ASSERT_THROW(storage.Get("baz"), ::TailProduce::StorageNoDataException);
}

TYPED_TEST(DataStorageTest, SimpleIterator) {
    TypeParam storage;
    storage.Set("foo:1", bytes("one"));
    storage.Set("foo:2", bytes("two"));
    storage.Set("zzz:", bytes("will not get here"));
    auto iterator = storage.CreateStorageIterator("foo:", "foo:\xff");
    ASSERT_FALSE(iterator.Done());
    EXPECT_EQ("foo:1", iterator.Key());
    EXPECT_EQ("one", antibytes(iterator.Value()));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    EXPECT_EQ("foo:2", iterator.Key());
    EXPECT_EQ("two", antibytes(iterator.Value()));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    ASSERT_THROW(iterator.Next(), ::TailProduce::StorageIteratorOutOfBoundsException);
}

TYPED_TEST(DataStorageTest, BoundedRangeIterator) {
    TypeParam storage;
    storage.Set("001", bytes("one"));
    storage.Set("002", bytes("two"));
    storage.Set("003", bytes("three"));
    storage.Set("004", bytes("four"));
    storage.Set("005", bytes("five"));
    typename TypeParam::StorageIterator iterator = storage.CreateStorageIterator("002", "004");
    ASSERT_FALSE(iterator.Done());
    EXPECT_EQ("002", iterator.Key());
    EXPECT_EQ("two", antibytes(iterator.Value()));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    EXPECT_EQ("003", iterator.Key());
    EXPECT_EQ("three", antibytes(iterator.Value()));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
}

TYPED_TEST(DataStorageTest, SemiBoundedRangeIterator) {
    TypeParam storage;
    storage.Set("1", bytes("one"));
    storage.Set("2", bytes("two"));
    storage.Set("3", bytes("three"));
    storage.Set("4", bytes("four"));
    storage.Set("5", bytes("five"));
    typename TypeParam::StorageIterator iterator = storage.CreateStorageIterator("3");
    ASSERT_FALSE(iterator.Done());
    EXPECT_EQ("3", iterator.Key());
    EXPECT_EQ("three", antibytes(iterator.Value()));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    EXPECT_EQ("4", iterator.Key());
    EXPECT_EQ("four", antibytes(iterator.Value()));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    EXPECT_EQ("5", iterator.Key());
    EXPECT_EQ("five", antibytes(iterator.Value()));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
}

TYPED_TEST(DataStorageTest, BoundedIteratorOutOfBounds) {
    TypeParam storage;
    storage.Set("1", bytes("one"));
    storage.Set("2", bytes("two"));
    storage.Set("3", bytes("three"));
    storage.Set("4", bytes("four"));
    storage.Set("5", bytes("five"));
    // Test `auto` with `storage.CreateStorageIterator()` as well.
    auto iterator = storage.CreateStorageIterator("2", "4");
    ASSERT_FALSE(iterator.Done());
    EXPECT_EQ("2", iterator.Key());
    EXPECT_EQ("two", antibytes(iterator.Value()));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    EXPECT_EQ("3", iterator.Key());
    EXPECT_EQ("three", antibytes(iterator.Value()));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    ASSERT_THROW(iterator.Next(), ::TailProduce::StorageIteratorOutOfBoundsException);
}

TYPED_TEST(DataStorageTest, UnboundedIteratorOutOfBounds) {
    TypeParam storage;
    storage.Set("1", bytes("one"));
    storage.Set("2", bytes("two"));
    auto iterator = storage.CreateStorageIterator("", "2");
    ASSERT_FALSE(iterator.Done());
    EXPECT_EQ("1", iterator.Key());
    EXPECT_EQ("one", antibytes(iterator.Value()));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    ASSERT_THROW(iterator.Next(), ::TailProduce::StorageIteratorOutOfBoundsException);
}
