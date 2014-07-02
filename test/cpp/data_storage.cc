#include <gtest/gtest.h>

#include "../../src/tailproduce.h"
#include "../../src/helpers.h"

#include "mocks/data_storage.h"

using ::TailProduce::bytes;

template<typename T> class DataStorageTest : public ::testing::Test {};

// TODO(dkorolev): Add more data storages here, specifically the LevelDB one once it's ready.
typedef ::testing::Types<MockDataStorage> DataStorageImplementations;
TYPED_TEST_CASE(DataStorageTest, DataStorageImplementations);

// TODO(dkorolev): Add similar tests to other files.
TYPED_TEST(DataStorageTest, HasRightBaseClass) {
    typedef ::TailProduce::Storage S;
    static_assert(std::is_base_of<S, TypeParam>::value, "Storage must inherit from ::TailProduce::Storage.");
}

TYPED_TEST(DataStorageTest, AddsEntries) {
    TypeParam storage;
    EXPECT_FALSE(storage.Has(bytes("foo")));
    EXPECT_FALSE(storage.Has(bytes(1)));
    storage.Set(bytes("foo"), bytes("bar"));
    EXPECT_TRUE(storage.Has(bytes("foo")));
    EXPECT_FALSE(storage.Has(bytes(1)));
    storage.Set(bytes(1), bytes(42));
    EXPECT_TRUE(storage.Has(bytes("foo")));
    EXPECT_TRUE(storage.Has(bytes(1)));
}

TYPED_TEST(DataStorageTest, DuplicateEntries) {
    TypeParam storage;
    storage.Set(bytes("key"), bytes("old"));
    ASSERT_THROW(storage.Set(bytes("key"), bytes("new")), ::TailProduce::StorageOverwriteNotAllowedException);
}

TYPED_TEST(DataStorageTest, SetsOverwritesAndGets) {
    TypeParam storage;
    storage.Set(bytes("key"), bytes("first"));
    EXPECT_EQ(bytes("first"), storage.Get(bytes("key")));
    storage.SetAllowingOverwrite(bytes("key"), bytes("second"));
    EXPECT_EQ(bytes("second"), storage.Get(bytes("key")));
}

TYPED_TEST(DataStorageTest, BasicExceptions) {
    TypeParam storage;
    ASSERT_THROW(storage.Set(bytes(""), bytes("foo")), ::TailProduce::StorageEmptyKeyException);
    ASSERT_THROW(storage.Set(bytes("bar"), bytes("")), ::TailProduce::StorageEmptyValueException);
    ASSERT_THROW(storage.Get(bytes("baz")), ::TailProduce::StorageNoDataException);
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

TYPED_TEST(DataStorageTest, BoundedIteratorOutOfBounds) {
    TypeParam storage;
    storage.Set(bytes(1), bytes("one"));
    storage.Set(bytes(2), bytes("two"));
    storage.Set(bytes(3), bytes("three"));
    storage.Set(bytes(4), bytes("four"));
    storage.Set(bytes(5), bytes("five"));
    typename TypeParam::Iterator iterator(storage, bytes(2), bytes(4));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == bytes(2));
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == bytes(3));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    ASSERT_THROW(iterator.Next(), ::TailProduce::StorageIteratorOutOfBoundsException);
}

TYPED_TEST(DataStorageTest, UnboundedIteratorOutOfBounds) {
    TypeParam storage;
    storage.Set(bytes(1), bytes("one"));
    storage.Set(bytes(2), bytes("two"));
    typename TypeParam::Iterator iterator(storage, bytes(2));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == bytes(2));
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    ASSERT_THROW(iterator.Next(), ::TailProduce::StorageIteratorOutOfBoundsException);
}
