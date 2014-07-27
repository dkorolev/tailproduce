#include <gtest/gtest.h>

// TODO(dkorolev): Complete this test. Remaining:
//                 * iterating over data via the StreamManager, going around the Db.
//                 * merging input streams.
//                 * producing data.
//                 * starting jobs.
//                 * serialization.

#include "../../src/tailproduce.h"

//#include "mocks/stream_manager.h"
#include "mocks/data_storage.h"

/*

// TODO(dkorolev): Entry implementations should inherit from their base class.
// TODO(dkorolev): Current serialization is a prototype, move to a more robust version.

// Test plan:
// * One basic stream, that is being populated "externally".
//   In other words, mocked, since each entry will be added independently.
// * A few derived streams:
//    - The one that listens to an input stream and outputs only the prime numbers from it.
//    - The one that listens to an input stream and outputs aggregates each N=10 entires.

// Ordering key.
struct SimpleIntegerOrderKey : TailProduce::OrderKey {
    SimpleIntegerOrderKey(uint32_t key) : key(key) {
    }
    uint32_t key;
    // TODO(dkorolev): Methods to serialize / deserialize the key.
    // TODO(dkorolev): A static function to extract the key w/o parsing the binary format.
};

// The SimpleIntegerEntry keeps the key/value pairs to test on. For this test they are { uint32, string } pairs.
struct SimpleIntegerEntry {
    // Used as order key, 1, 2, 3, etc.
    uint32_t key;
    // To test data extraction, "one", "two", "three", or anything else for the sake of this test.
    const std::string data;

    template<typename T> T OrderKey() const;
    // TODO(dkorolev): Serialization / deserialization.
    // TOOD(dkorolev): Extracting the order key as uint32_t,
    //                 failing to extract it as any other type at compine time.
};
template<> SimpleIntegerOrderKey SimpleIntegerEntry::OrderKey<SimpleIntegerOrderKey>() const {
    return SimpleIntegerOrderKey(key);
}

// EphemeralMarkerEntryType is fired every N=10 entries, so that TailProduce jobs don't have to keep their counters.
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
    typedef SimpleStringEntry OUTPUT_TYPE;
    // T_OUTPUT is the template parameter for this instance of TailProduce job,
    // it exposes one method, T_OUTPUT::Output(const OUTPUT_TYPE& entry),
    // where OUTPUT_TYPE is defined within this class.
    void Consume(const SimpleIntegerEntry& entry, T_OUTPUT&) {
        intermediate_.push_back(entry.data);
    }
    void Consume(const EphemeralMarkerEntryType& marker, T_OUTPUT& output) {
        std::ostringstream os;
        for (size_t i = 0; i < intermediate_.size(); ++i) {
            if (i) {
                os << ',';
            }
            os << intermediate_[i];
        }
        intermediate_.clear();

        SimpleStringEntry entry;
        entry.setOrderKey(marker.key);
        entry.setValue(os.str());
        output.Output(entry);
    }
    std::vector<std::string> intermediate_;
};

template<typename T> class StreamManagerTest : public ::testing::Test {};

// Unit test for TailProduce static framework.
// TODO(dkorolev): Add more stream managers and data storages here.
typedef ::testing::Types<MockStreamManager<MockDataStorage>> DataStorageImplementations;
TYPED_TEST_CASE(StreamManagerTest, DataStorageImplementations);

// Tests stream creation macros.
TYPED_TEST(StreamManagerTest, UserFriendlySyntaxCompiles) {
    TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(StreamManagerImpl, TypeParam);
    TAILPRODUCE_INPUT_STREAM(test, SimpleIntegerEntry, SimpleIntegerOrderKey);
    TAILPRODUCE_DERIVED_STREAM(test2,
                               PrimesAggregatorTailProduce<..., ...>::type,
                               INPUTS(INPUT(test, SimpleIntegerEntry, 1000),
                                      EPHEMERAL_INPUT(IntervalEventEmitter(100),
                                      REGISTERED_EVENT_TYPE(Foo)));  // Test this too.
    TAILPRODUCE_STATIC_FRAMEWORK_END();

    StreamManagerImpl impl;
    ASSERT_EQ(1, impl.registry().streams.size());
    EXPECT_EQ("test", impl.registry().streams[0].name);
    EXPECT_EQ("SimpleIntegerEntry", impl.registry().streams[0].entry_type);
    EXPECT_EQ("SimpleIntegerOrderKey", impl.registry().streams[0].order_key_type);
    EXPECT_TRUE(impl.registry().streams[0].impl == static_cast<TailProduce::Stream*>(&impl.test));
    EXPECT_TRUE((std::is_same<SimpleIntegerEntry, typename StreamManagerImpl::STREAM_TYPE_test::ENTRY_TYPE>::value));
    EXPECT_TRUE((std::is_same<SimpleIntegerOrderKey,
                 typename StreamManagerImpl::STREAM_TYPE_test::ORDER_KEY_TYPE>::value));
}

// This test explicitly lists what stream definition macros expand into.
// Used as a reference point, as well as to ensure the macros do what they have been intended to.
TYPED_TEST(StreamManagerTest, ExpandedMacroSyntaxCompiles) {
    class StreamManagerImpl : public TypeParam {
      private:
        ::TailProduce::StreamsRegistry registry_;
      public:
        const ::TailProduce::StreamsRegistry& registry() const { return registry_; }
        typedef ::TailProduce::StreamInstance<SimpleIntegerEntry, SimpleIntegerOrderKey> STREAM_TYPE_test;
        STREAM_TYPE_test test = STREAM_TYPE_test(registry_, "test", "SimpleIntegerEntry", "SimpleIntegerOrderKey");
    };

    StreamManagerImpl impl;
    ASSERT_EQ(1, impl.registry().streams.size());
    EXPECT_EQ("test", impl.registry().streams[0].name);
    EXPECT_EQ("SimpleIntegerEntry", impl.registry().streams[0].entry_type);
    EXPECT_EQ("SimpleIntegerOrderKey", impl.registry().streams[0].order_key_type);
    EXPECT_TRUE(impl.registry().streams[0].impl == static_cast<TailProduce::Stream*>(&impl.test));
    EXPECT_TRUE((std::is_same<SimpleIntegerEntry, typename StreamManagerImpl::STREAM_TYPE_test::ENTRY_TYPE>::value));
    EXPECT_TRUE((std::is_same<SimpleIntegerOrderKey,
                 typename StreamManagerImpl::STREAM_TYPE_test::ORDER_KEY_TYPE>::value));

    ReadIterator primes = read_primes();  // for the output of the primes filter TailProduce job.

    size_t current = 0;
    while (++current <= 10) {
        test.MockAddInput(current);
    }

    EXPECT(!primes.Done());
    EXPECT_EQ(bytes("2"), primes.Key());
    primes.Next();
    EXPECT(!primes.Done());
    EXPECT_EQ(bytes("3"), primes.Key());
    primes.Next();
    EXPECT(!primes.Done());
    EXPECT_EQ(bytes("5"), primes.Key());
    primes.Next();
    EXPECT(!primes.Done());
    EXPECT_EQ(bytes("7"), primes.Key());
    primes.Next();
    EXPECT(primes.Done());

    while (++current <= 300) {
        test.MockAddInput(current);
    }

    EXPECT(!other_it.Done());
    EXPECT_EQ(bytes(100), other_it.Key());
    EXPECT_EQ(bytes("2,3,5,7,...,97"), other_it.Value());
    other_it.Next();
    EXPECT(!other_it.Done());
    EXPECT_EQ(bytes(200), other_it.Key());
    EXPECT_EQ(bytes("101,...,"), other_it.Value());
    other_it.Next();
    EXPECT(!other_it.Done());
    EXPECT_EQ(bytes(300), other_it.Key());
    EXPECT_EQ(bytes("211,...,"), other_it.Value());
    other_it.Next();
    EXPECT(other_it.Done());

    test.MockAddInput(1);
    test.MockAddInput(1);
}

*/
