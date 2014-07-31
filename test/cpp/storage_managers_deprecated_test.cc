// TODO(dkorolev): Ask Brian whether this test should be merged with the data_storage.cc one (and renamed?).

#include <gtest/gtest.h>
#include <exception>
#include <chrono>
#include <memory>
#include <boost/filesystem.hpp>

#include "../../src/tailproduce.h"
#include "../../src/bytes.h"

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
