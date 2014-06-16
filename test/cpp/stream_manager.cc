#include <gtest/gtest.h>

#include "mocks/data_storage.h"

// Test plan:
// * One basic stream, that is being populated "externally".
//   In other words, mocked, since each entry will be added independently.
// * A few derived streams:
//    - The one that listens to an input stream and outputs only the prime numbers from it.
//    - The one that listens to an input stream and outputs aggregates each N=10 entires.

// The SimpleIntegerEntry keeps the key/value pairs to test on. For this test they are { uint32, string } pairs.
struct SimpleIntegerEntry {
    // Used as order key, 1, 2, 3, etc.
    uint32_t key;
    // To test data extraction, "one", "two", "three", or anything else for the sake of this test.
    const std::string data;

    // TODO(dkorolev): Serialization / deserialization.
    // TOOD(dkorolev): Extracting the order key as uint32_t,
    //                 failing to extract it as any other type at compine time.
};

// EphemeralMarkerEntryType is "fired" every N=10 entries, so that TailProduce jobs don't have to keep their counters.
struct EphemeralMarkerEntryType {
};

// An example TailProduce job, effectively acting as a filter.
// TODO(dkorolev): TailProduceFilter<T_PREDICATE> should probably be a part of the standard TailProduce library.
// TODO(dkorolev): Standardize typing convention to be able to extract order key types from T_OUTPUT.
template<typename T_OUTPUT> struct PrimeNumbersTailProduce {
    void Consume(const SimpleIntegerEntry& entry, T_OUTPUT& output) {
        if (is_prime(entry.key)) {
            output.Produce(entry);
        }
    }

  private:
    static bool is_prime(const uint32_t& x) {
        for (uint32_t i = 2; i * i <= x; ++i) {
            if ((x % i) == 0) {
                return false;
            }
        }
        return true;
    }
};

// An example TailProduce job, effectively acting as an aggregator.
// TODO(dkorolev): TailProduceAggregator<T_MARKER> should probably be a part of the standard TailProduce library.
template<typename T_OUTPUT> struct PrimesAggregatorTailProduce {
    void Consume(const SimpleIntegerEntry& entry, T_OUTPUT& output) {
        intermediate_.push_back(entry.data);
    }
    void Consume(const EphemeralMarkerEntryType&) {
        std::ostringstream os;
        for (size_t i = 0; i < intermediate_.size(); ++i) {
            if (i) {
                os << ',';
            }
            os << intermediate_[i];
        }
        intermediate_.clear();
    }
    std::vector<std::string> intermediate_;
};

template<typename T> class StreamManagerTest : public ::testing::Test {};

// TODO(dkorolev): Add more stream managers and data storages here.
typedef ::testing::Types<MockStreamManager<MockDataStorage>> DataStorageImplementations;
TYPED_TEST_CASE(StreamManagerTest, DataStorageImplementations);

TYPED_TEST(StreamManagerTest, Dummy) {
    EXPECT_EQ(4, 2 + 2)
}
