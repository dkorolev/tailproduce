#include <gtest/gtest.h>
#include <exception>
#include <chrono>
#include <memory>
#include <boost/filesystem.hpp>

#include "../../src/tailproduce.h"
#include "../../src/helpers.h"

#include "../../src/storage_manager.h"
#include "../../src/dbm_leveldb.h"
#include "../../src/dbm_leveldb_iterator.h"

using ::TailProduce::bytes;

uint64_t date_now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

std::string makeKey(uint64_t millis, std::string const& streamId, std::string const& value = std::string()) {
    // We want to add a time element to the key so it will be unique per run.
    std::ostringstream os;
    os << streamId << "-";
    if (!value.empty()) os << value << "-";
    os << millis;
    return os.str();
}

TEST(StorageManagerTest, AddsEntries) {
    // Simple test adds a couple of entries and verifies we can retrieve them.
    const uint64_t millis = date_now();
    boost::filesystem::remove_all("../leveldbTest");
    auto dbm = TailProduce::DbMLevelDb("../leveldbTest");
    auto storage = TailProduce::StorageManager<TailProduce::DbMLevelDb>(dbm);
    storage.Set(makeKey(millis, "foo"), bytes("bar"));
    storage.Set(makeKey(millis, "1"), bytes(42));
    EXPECT_EQ(storage.Get(makeKey(millis, "foo")), bytes("bar"));
    EXPECT_EQ(storage.Get(makeKey(millis, "1")), bytes(42));
}

TEST(StorageManageTest, BoundedRangeIterator) {
    // Add some records to the db and verify that we can iterate over a subset of the records.
    const uint64_t millis = date_now();
    boost::filesystem::remove_all("../leveldbTest");
    auto dbm = TailProduce::DbMLevelDb("../leveldbTest");
    auto storage = TailProduce::StorageManager<TailProduce::DbMLevelDb>(dbm);

    auto key1 = makeKey(millis, "BoundedRangeIterator", "001");
    auto key2 = makeKey(millis, "BoundedRangeIterator", "002");
    auto key3 = makeKey(millis, "BoundedRangeIterator", "003");
    auto key4 = makeKey(millis, "BoundedRangeIterator", "004");
    auto key5 = makeKey(millis, "BoundedRangeIterator", "005");

    storage.Set(key1, bytes("one"));
    storage.Set(key2, bytes("two"));
    storage.Set(key3, bytes("three"));
    storage.Set(key4, bytes("four"));
    storage.Set(key5, bytes("five"));

    auto iterator = storage.GetIterator("BoundedRangeIterator-", "002", key4);
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == key2);
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == key3);
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
}

TEST(StorageManageTest, BoundedRangeDynamicIterator) {
    // Create an iterator that goes beyond actual data.  Verify records added later will be accessed.
    const uint64_t millis = date_now();
    boost::filesystem::remove_all("../leveldbTest");
    auto dbm = TailProduce::DbMLevelDb("../leveldbTest");
    auto storage = TailProduce::StorageManager<TailProduce::DbMLevelDb>(dbm);

    auto key1 = makeKey(millis, "BoundedRangeDynamicIterator", "x1");
    auto key2 = makeKey(millis, "BoundedRangeDynamicIterator", "x2");
    auto key3 = makeKey(millis, "BoundedRangeDynamicIterator", "x3");
    auto key4 = makeKey(millis, "BoundedRangeDynamicIterator", "x4");
    auto key5 = makeKey(millis, "BoundedRangeDynamicIterator", "x5");
    auto key6 = makeKey(millis, "BoundedRangeDynamicIterator", "x6");
    storage.Set(key1, bytes("one"));
    storage.Set(key2, bytes("two"));

    auto iterator = storage.GetIterator("BoundedRangeDynamicIterator-", "x2", key4);
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == key2);
    ASSERT_FALSE(iterator.Done());
    storage.Set(key3, bytes("three"));
    storage.Set(key4, bytes("four"));
    storage.Set(key5, bytes("five"));
    iterator.Next();  // Now at key3.
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == key3);
    ASSERT_FALSE(iterator.Done());
    iterator.Next();  // Now one past key 3.
    ASSERT_TRUE(iterator.Done());
    storage.Set(key6, bytes("six"));
    ASSERT_TRUE(iterator.Done());
}

TEST(StorageManageTest, SemiBoundedRangeIterator) {
    // Create an iterator that does not have a terminating end.  Verify that we can
    // retrieve the entire stream.
    const uint64_t millis = date_now();
    boost::filesystem::remove_all("../leveldbTest");
    auto dbm = TailProduce::DbMLevelDb("../leveldbTest");
    auto storage = TailProduce::StorageManager<TailProduce::DbMLevelDb>(dbm);

    auto key1 = makeKey(millis, "SemiBoundedRangeIterator", "1");
    auto key2 = makeKey(millis, "SemiBoundedRangeIterator", "2");
    auto key3 = makeKey(millis, "SemiBoundedRangeIterator", "3");
    auto key4 = makeKey(millis, "SemiBoundedRangeIterator", "4");
    auto key5 = makeKey(millis, "SemiBoundedRangeIterator", "5");
    auto key6 = makeKey(millis, "NextStreamId", "6");
    storage.Set(key1, bytes("one"));
    storage.Set(key2, bytes("two"));
    storage.Set(key3, bytes("three"));
    storage.Set(key4, bytes("four"));
    storage.Set(key5, bytes("five"));
    storage.Set(key6, bytes("NextStream"));

    {
        // This will be the entire stream.
        auto iterator = storage.GetIterator("SemiBoundedRangeIterator-");
        ASSERT_FALSE(iterator.Done());
        ASSERT_TRUE(iterator.Value() == bytes("one"));
        ASSERT_TRUE(iterator.Key() == key1);
        iterator.Next();
        ASSERT_FALSE(iterator.Done());
        ASSERT_TRUE(iterator.Value() == bytes("two"));
        ASSERT_TRUE(iterator.Key() == key2);
        iterator.Next();
        ASSERT_FALSE(iterator.Done());
        ASSERT_TRUE(iterator.Value() == bytes("three"));
        ASSERT_TRUE(iterator.Key() == key3);
        iterator.Next();
        ASSERT_FALSE(iterator.Done());
        ASSERT_TRUE(iterator.Value() == bytes("four"));
        ASSERT_TRUE(iterator.Key() == key4);
        iterator.Next();
        ASSERT_FALSE(iterator.Done());
        ASSERT_TRUE(iterator.Value() == bytes("five"));
        ASSERT_TRUE(iterator.Key() == key5);
        iterator.Next();
        ASSERT_TRUE(iterator.Done());
    }

    {
        // This will be the entire stream from some starting point
        auto iterator = storage.GetIterator("SemiBoundedRangeIterator-", "3");
        ASSERT_FALSE(iterator.Done());
        ASSERT_TRUE(iterator.Value() == bytes("three"));
        ASSERT_TRUE(iterator.Key() == key3);
        iterator.Next();
        ASSERT_FALSE(iterator.Done());
        ASSERT_TRUE(iterator.Value() == bytes("four"));
        ASSERT_TRUE(iterator.Key() == key4);
        iterator.Next();
        ASSERT_FALSE(iterator.Done());
        ASSERT_TRUE(iterator.Value() == bytes("five"));
        ASSERT_TRUE(iterator.Key() == key5);
        iterator.Next();
        ASSERT_TRUE(iterator.Done());
    }
}

TEST(StorageManageTest, UnboundedRangeDynamicIterator) {
    const uint64_t millis = date_now();
    boost::filesystem::remove_all("../leveldbTest");
    auto dbm = TailProduce::DbMLevelDb("../leveldbTest");
    auto storage = TailProduce::StorageManager<TailProduce::DbMLevelDb>(dbm);

    {
        // This is demonstrating a very special case.  When creating an unbounded iterator with a completely empty
        // db then the iterator is not Valid.  Calling Value() or Key() functions should throw.
        auto iterator = storage.GetIterator(makeKey(millis, "UnboundedRangeDynamicIterator-"));
        ASSERT_TRUE(iterator.Done());
        ASSERT_THROW(iterator.Key(), std::logic_error);
        ASSERT_THROW(iterator.Value(), std::logic_error);
    }

    auto key1 = makeKey(millis, "UnboundedRangeDynamicIterator", "101");
    auto key2 = makeKey(millis, "UnboundedRangeDynamicIterator", "102");
    auto key3 = makeKey(millis, "UnboundedRangeDynamicIterator", "103");
    auto key4 = makeKey(millis, "UnboundedRangeDynamicIterator", "104");
    storage.Set(key1, bytes("this"));
    // Now this is a more typical case.  Create an unbounded iterator when there is 1 or more records in the db.
    auto iterator = storage.GetIterator("UnboundedRangeDynamicIterator-", "101");
    ASSERT_FALSE(iterator.Done());

    storage.Set(key2, bytes("too"));

    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("this"));
    ASSERT_TRUE(iterator.Key() == key1);
    iterator.Next();  // Advances to 102 key.
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("too"));
    ASSERT_TRUE(iterator.Key() == key2);

    iterator.Next();  // Goes one past the end.  What should happen in that case???
    ASSERT_TRUE(iterator.Done());
    storage.Set(key3, bytes("shall"));
    storage.Set(key4, bytes("pass"));
    ASSERT_FALSE(iterator.Done());  // Rebuilds the iterator.
    ASSERT_TRUE(iterator.Value() == bytes("shall"));
    ASSERT_TRUE(iterator.Key() == key3);
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("pass"));
    ASSERT_TRUE(iterator.Key() == key4);
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
}

TEST(StorageManageTest, BoundedIteratorOutOfBoundsDeathTest) {
    // Verify that an error is thrown after advancing beyond the last record in the stream.
    const uint64_t millis = date_now();
    boost::filesystem::remove_all("../leveldbTest");
    auto dbm = TailProduce::DbMLevelDb("../leveldbTest");
    auto storage = TailProduce::StorageManager<TailProduce::DbMLevelDb>(dbm);

    auto key1 = makeKey(millis, "BoundedIteratorOutOfBoundsDeathTest", "1");
    auto key2 = makeKey(millis, "BoundedIteratorOutOfBoundsDeathTest", "2");
    auto key3 = makeKey(millis, "BoundedIteratorOutOfBoundsDeathTest", "3");
    auto key4 = makeKey(millis, "BoundedIteratorOutOfBoundsDeathTest", "4");
    auto key5 = makeKey(millis, "BoundedIteratorOutOfBoundsDeathTest", "5");
    storage.Set(key1, bytes("one"));

    auto iterator = storage.GetIterator("BoundedIteratorOutOfBoundsDeathTest-", "2", key4);
    ASSERT_TRUE(iterator.Done());
    storage.Set(key2, bytes("two"));
    storage.Set(key3, bytes("three"));
    storage.Set(key4, bytes("four"));
    storage.Set(key5, bytes("five"));
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == key2);
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == key3);
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    ASSERT_THROW(iterator.Next(), std::logic_error);
}

TEST(StorageManageTest, UnboundedIteratorOutOfBoundsDeathTest) {
    // Create unbounded iterator and verify we throw when advancing beyond end of records.
    const uint64_t millis = date_now();
    boost::filesystem::remove_all("../leveldbTest");
    auto dbm = TailProduce::DbMLevelDb("../leveldbTest");
    auto storage = TailProduce::StorageManager<TailProduce::DbMLevelDb>(dbm);

    auto key1 = makeKey(millis, "UnboundedIteratorOutOfBoundsDeathTest", "1");
    auto key2 = makeKey(millis, "UnboundedIteratorOutOfBoundsDeathTest", "2");
    auto key3 = makeKey(millis, "UnboundedIteratorOutOfBoundsDeathTest", "3");
    auto key4 = makeKey(millis, "UnboundedIteratorOutOfBoundsDeathTest", "4");

    auto iterator = storage.GetIterator("UnboundedIteratorOutOfBoundsDeathTest-", "2");  // Iterator is on key 2.
    ASSERT_TRUE(iterator.Done());
    storage.Set(key1, bytes("one"));
    storage.Set(key2, bytes("two"));
    ASSERT_FALSE(iterator.Done());  // Iterator gets rebuilt and positioned at key2
    ASSERT_TRUE(iterator.Value() == bytes("two"));
    ASSERT_TRUE(iterator.Key() == key2);
    iterator.Next();  // Iterator now at key3.
    ASSERT_TRUE(iterator.Done());
    storage.Set(key3, bytes("three"));
    storage.Set(key4, bytes("four"));
    ASSERT_FALSE(iterator.Done());  // Iterator gets rebuilt and positioned at key3
    ASSERT_TRUE(iterator.Value() == bytes("three"));
    ASSERT_TRUE(iterator.Key() == key3);
    iterator.Next();
    ASSERT_FALSE(iterator.Done());
    ASSERT_TRUE(iterator.Value() == bytes("four"));
    ASSERT_TRUE(iterator.Key() == key4);
    iterator.Next();
    ASSERT_TRUE(iterator.Done());
    ASSERT_THROW(iterator.Next(), std::logic_error);
}
