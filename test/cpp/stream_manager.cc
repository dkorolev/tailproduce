// TODO(dkorolev): Save HEAD-s into storage and test cold starts!
// TODO(dkorolev): Add --create_streams flags and ensure that the service can not start w/o them.
// TODO(dkorolev): Add secondary keys as "A" .. "Z", "ZA", .. , "ZZ", "ZZA", ... "ZZZ", "ZZZA".

// The test for STREAM MANAGER confirms that:
//
// 1. Stream manager binds to the data storage, be it the production one or the mock one.
// 2. Stream manager can be used to instantiate a (statically defined) set of streams.
// 3. The macros to define the set of streams to use do their job right.
// 4. Serialization and de-serialization into respective types works.
// 5. Listeners and publishers can be created and behave as expected.
// 6. The storage schema is respected, all the way to requiring stream metadata to exist.
//
// The test for stream manager does NOT test:
//
// * The functionality of storage-level iterators.
//
//   They are handled at the lower level (data_storage.cc).
//
// * Making sure only one append iterator can exist per stream and that it never overwrites data.
//   Updating the "HEAD" order key per stream.
//   Merging multiple streams maintaining strongly typed entries.
//   Ephemeral entry types as markers.
//   setTimeout()-style insertion of callbacks to be invoked by the framework later.
//  
//   The above is handled at the higher level (framework.cc).

#include <gtest/gtest.h>

#include "../../src/tailproduce.h"

#include "mocks/stream_manager.h"
#include "mocks/data_storage.h"
#include "mocks/test_client.h"

// TODO(dkorolev): Entry implementations should inherit from their base class.
// TODO(dkorolev): Current serialization is a prototype, move to a more robust version.

// TODO(dkorolev): Add a death test for the case when the storage is not yet initalized.
// TODO(dkorolev): Add a death test prohibiting creating multiple appenders for one stream.
// TODO(dkorolev): Add a death test confirming that overwriting the past is not allowed.

// The actual test is a templated RUN_TESTS() function.
// It is used to test both the hand-crafted objects structure
// and the one created by a sequence of macros.
template<typename STREAM_MANAGER> void RUN_TESTS() {
    STREAM_MANAGER streams_manager;

    // Test that the test stream exists.
    ASSERT_EQ(1, streams_manager.registry().streams.size());
    EXPECT_EQ("test", streams_manager.registry().streams[0].name);
    EXPECT_EQ("SimpleEntry", streams_manager.registry().streams[0].entry_type);
    EXPECT_EQ("SimpleOrderKey", streams_manager.registry().streams[0].order_key_type);
    using Stream = ::TailProduce::Stream;
    EXPECT_TRUE(streams_manager.registry().streams[0].impl == static_cast<Stream*>(&streams_manager.test.stream));
    EXPECT_TRUE((std::is_same<SimpleEntry, typename STREAM_MANAGER::test_type::entry_type>::value));
    EXPECT_TRUE((std::is_same<SimpleOrderKey, typename STREAM_MANAGER::test_type::order_key_type>::value));

    // Test that entries can be serialized and de-serialized into C++ streams.
    {
        SimpleEntry entry(1, "Test");
        std::ostringstream os;
        SimpleEntry::SerializeEntry(os, entry);
        std::string s = os.str();
        EXPECT_EQ("{\n    \"value0\": {\n        \"key\": 1,\n        \"data\": \"Test\"\n    }\n}\n", s);
        {
            SimpleEntry restored;
            std::istringstream is(s);
            SimpleEntry::DeSerializeEntry(is, restored);
            EXPECT_EQ(1, restored.key);
            EXPECT_EQ("Test", restored.data);
        }
    }

    // Test that the order key can be serialized and de-serialized to and from fixed size byte arrays.
    {
        SimpleEntry entry(42, "The Answer");
        SimpleOrderKey order_key = entry.template GetOrderKey<SimpleOrderKey>();
        uint8_t serialized_key[SimpleOrderKey::size_in_bytes];
        order_key.SerializeOrderKey(serialized_key);
        EXPECT_EQ("0000000042", std::string(serialized_key, serialized_key + sizeof(serialized_key)));
        {
            SimpleOrderKey deserialized_order_key;
            deserialized_order_key.DeSerializeOrderKey(serialized_key);
            EXPECT_EQ(42, deserialized_order_key.key);
        }
    }

    // Test HEAD updates.
    {
        // Start from zero.
        typename STREAM_MANAGER::test_type::listener_type listener(streams_manager.test);
        typename STREAM_MANAGER::test_type::head_pair_type head;

        head = listener.GetHead();
        EXPECT_EQ(0, head.first.key);
        EXPECT_EQ(0, head.second);
    
        // Instantiating a publisher does not change HEAD.
        {
            typename STREAM_MANAGER::test_type::publisher_type publisher(streams_manager.test);
            head = publisher.GetHead();
            EXPECT_EQ(0, head.first.key);
            EXPECT_EQ(0, head.second);
            head = listener.GetHead();
            EXPECT_EQ(0, head.first.key);
            EXPECT_EQ(0, head.second);
        }

        // Push() and PushHead() change HEAD.
        // Secondary keys are incremented automatically.
        {
            typename STREAM_MANAGER::test_type::publisher_type publisher(streams_manager.test);
            publisher.Push(SimpleEntry(1, "foo"));
            head = publisher.GetHead();
            EXPECT_EQ(1, head.first.key);
            EXPECT_EQ(0, head.second);
            head = listener.GetHead();
            EXPECT_EQ(1, head.first.key);
            EXPECT_EQ(0, head.second);
            publisher.Push(SimpleEntry(1, "bar"));
            head = publisher.GetHead();
            EXPECT_EQ(1, head.first.key);
            EXPECT_EQ(1, head.second);
            head = listener.GetHead();
            EXPECT_EQ(1, head.first.key);
            EXPECT_EQ(1, head.second);
            publisher.PushHead(SimpleOrderKey(2));
            head = publisher.GetHead();
            EXPECT_EQ(2, head.first.key);
            EXPECT_EQ(0, head.second);
            head = listener.GetHead();
            EXPECT_EQ(2, head.first.key);
            EXPECT_EQ(0, head.second);
            publisher.PushHead(SimpleOrderKey(2));
            head = publisher.GetHead();
            EXPECT_EQ(2, head.first.key);
            EXPECT_EQ(1, head.second);
            head = listener.GetHead();
            EXPECT_EQ(2, head.first.key);
            EXPECT_EQ(1, head.second);
        }

        // TODO: Instantiating a publisher starting from a fixed HEAD moves HEAD there.
        {
        }

        // TODO: Death test on pushing head backwards.
    }

    // Test that the stream can be listened to.
    // Test that multiple listeners can co-exist and that listeners support ranes.
    /*
    typename STREAM_MANAGER::test_type::listener_type listener(streams_manager);
    typename STREAM_MANAGER::test_type::listener_type second_listener(streams_manager, SimpleOrderKey(50));
    typename STREAM_MANAGER::test_type::listener_type third_listener(streams_manager,
                                                                     SimpleOrderKey(150),
                                                                     SimpleOrderKey(250));

    EXPECT_TRUE(!listener.HasData());
    EXPECT_TRUE(!second_listener.HasData());
    EXPECT_TRUE(!third_listener.HasData());
    */

    // Test that the stream can be appended to.

    {
        // Explicit typing.
        typename STREAM_MANAGER::test_type::publisher_type publisher(streams_manager.test);
        SimpleEntry entry;
        entry.key = 42;
        entry.data = "The Answer";
        SimpleOrderKey order_key = entry.template GetOrderKey<SimpleOrderKey>();
        publisher.Push(entry);
    }

    /*
    {
        // Implicit typing.
        auto publisher = streams_manager.test.publisher();
        SimpleEntry entry;
        entry.key = 42;
        entry.data = "The Answer";
        SimpleOrderKey order_key = entry.template GetOrderKey<SimpleOrderKey>();
        publisher.Push(entry);
    }
    */
    
    // Test that the listeners get the updates.

    // Test that the listeners don't get the data beyond the end of the range they have subscribed to.

    // Test that updating head propagates to the listeners.
    
    // Test that updating head marks completed listeners completed.

    /*
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
    */
}



template<typename T> class StreamManagerTest : public ::testing::Test {};

// Unit test for TailProduce stream manager.

// TODO(dkorolev): Add LevelDB data storage along with the mock one.
typedef ::testing::Types<MockStreamManager<MockDataStorage>> DataStorageImplementations;
TYPED_TEST_CASE(StreamManagerTest, DataStorageImplementations);

// Tests stream creation macros.
TYPED_TEST(StreamManagerTest, UserFriendlySyntaxCompiles) {
    /*
    TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(StreamManagerImpl, TypeParam);
    TAILPRODUCE_INPUT_STREAM(test, SimpleEntry, SimpleOrderKey);
    TAILPRODUCE_DERIVED_STREAM(test2,
                               PrimesAggregatorTailProduce<..., ...>::type,
                               INPUTS(INPUT(test, SimpleEntry, 1000),
                                      EPHEMERAL_INPUT(IntervalEventEmitter(100),
                                      REGISTERED_EVENT_TYPE(Foo)));  // Test this too.
    TAILPRODUCE_STATIC_FRAMEWORK_END();

    StreamManagerImpl impl;
    ASSERT_EQ(1, impl.registry().streams.size());
    EXPECT_EQ("test", impl.registry().streams[0].name);
    EXPECT_EQ("SimpleEntry", impl.registry().streams[0].entry_type);
    EXPECT_EQ("SimpleOrderKey", impl.registry().streams[0].order_key_type);
    EXPECT_TRUE(impl.registry().streams[0].impl == static_cast<TailProduce::Stream*>(&impl.test));
    EXPECT_TRUE((std::is_same<SimpleEntry, typename StreamManagerImpl::STREAM_TYPE_test::ENTRY_TYPE>::value));
    EXPECT_TRUE((std::is_same<SimpleOrderKey,
                 typename StreamManagerImpl::STREAM_TYPE_test::ORDER_KEY_TYPE>::value));
    */
}

// This test explicitly lists what stream definition macros expand into.
// Used as a reference point, as well as to ensure the macros do what they are designed to.
namespace TailProduce {
    // StreamListener contains the logic of creating and re-creating storage-level read iterators,
    // presenting data in serialized format and keeping track of HEAD order keys.
    template<typename T> struct StreamListener {
        /*
        template<typename T_MANAGER> explicit StreamListener(const T_MANAGER& manager) {
            using TM = ::TailProduce::StreamManager;
            static_assert(std::is_base_of<TM, T_MANAGER>::value, "StreamListener(x) requires `x` to be a StreamManager.");
        }
        template<typename T_MANAGER> StreamListener(const T_MANAGER& manager,
                                                    const typename T::order_key_type& begin) {
            using TM = ::TailProduce::StreamManager;
            static_assert(std::is_base_of<TM, T_MANAGER>::value, "StreamListener(x) requires `x` to be a StreamManager.");
        }
        template<typename T_MANAGER> StreamListener(const T_MANAGER& manager,
                                                    const typename T::order_key_type& begin,
                                                    const typename T::order_key_type& end) {
            using TM = ::TailProduce::StreamManager;
            static_assert(std::is_base_of<TM, T_MANAGER>::value, "StreamListener(x) requires `x` to be a StreamManager.");
        }
        */
        StreamListener() = delete;
        explicit StreamListener(const T& stream) : stream(stream) {
        }
        StreamListener(StreamListener&&) = default;
        
        const typename T::head_pair_type& GetHead() const {
            return stream.head;
        }

        // Returns true if more data is available.
        // Can change from false to true if/when new data is available.
        bool HasData() const {
            return false;
        }

      private:
        StreamListener(const StreamListener&) = delete;
        void operator=(const StreamListener&) = delete;
        const T& stream;
    };

    // StreamPublisher contains the logic of appending data to the streams and updating their HEAD order keys.
    template<typename T> struct StreamPublisher {
        /*
        template<typename T_MANAGER> explicit StreamPublisher(const T_MANAGER& manager) {
            using TM = ::TailProduce::StreamManager;
            static_assert(std::is_base_of<TM, T_MANAGER>::value, "StreamPublisher(x) requires `x` to be a StreamManager.");
        }
        */

        StreamPublisher() = delete;
        explicit StreamPublisher(T& stream) : stream(stream) {
        }

        void Push(const typename T::entry_type& entry) {
            PushHead(entry.template GetOrderKey<typename T::order_key_type>());
            // TODO(dkorolev): Add entry to the storage.
        }

        void PushHead(const typename T::order_key_type& order_key) {
            typename T::head_pair_type new_head(order_key, 0);
            if (new_head < stream.head) {
                // TODO(dkorolev): Proper exception type.
                throw std::exception();
            }
            if (!(stream.head < new_head)) {
                // Increment the secondary key when pushing to the same primary key.
                new_head.second = stream.head.second + 1;
            }
            // TODO(dkorolev): Update the storage.
            stream.head = new_head;
        }

        // TODO: PushSecondaryKey for merge usecases.

        const typename T::head_pair_type& GetHead() const {
            return stream.head;
        }

        /*
        template<typename T> StreamPublisher(const T& manager,
                                            const T_ORDER_KEY_TYPE& begin) {
            using TM = ::TailProduce::StreamManager;
            static_assert(std::is_base_of<TM, T>::value, "StreamPublisher(x) requires `x` to be a StreamManager.");
        }
        template<typename T> StreamPublisher(const T& manager,
                                            const T_ORDER_KEY_TYPE& begin,
                                            const T_ORDER_KEY_TYPE& end) {
            using TM = ::TailProduce::StreamManager;
            static_assert(std::is_base_of<TM, T>::value, "StreamPublisher(x) requires `x` to be a StreamManager.");
        }
        // Returns true if more data is available.
        // Can change from false to true if/when new data is available.
        bool HasData() const {
            return false;
        }
        */

      private:
        StreamPublisher(const StreamPublisher&) = delete;
        void operator=(const StreamPublisher&) = delete;
        T& stream;
    };

};

TYPED_TEST(StreamManagerTest, ExpandedMacroSyntaxCompiles) {
    class StreamManagerImpl : public TypeParam {
      private:
        ::TailProduce::StreamsRegistry registry_;
      public:
        const ::TailProduce::StreamsRegistry& registry() const { return registry_; }
        struct test_type {
            typedef SimpleEntry entry_type;
            typedef SimpleOrderKey order_key_type;
            typedef ::TailProduce::StreamInstance<entry_type, order_key_type> stream_type;
            typedef ::TailProduce::StreamListener<test_type> listener_type;
            typedef ::TailProduce::StreamPublisher<test_type> publisher_type;
            typedef std::pair<order_key_type, uint32_t> head_pair_type;
            StreamManagerImpl* manager;
            stream_type stream;
            const std::string name;
            head_pair_type head;
            std::unique_ptr<std::mutex> p_mutex;
            std::mutex& mutex() {
                if (!p_mutex.get()) {
                    p_mutex.reset(new std::mutex());
                }
                return p_mutex.get();
            }
            /*
            // Gotta figure out move semantics -- D.K.
            listener_type&& listener() const {
                return listener_type(*this);
            }
            publisher_type publisher() {
                return publisher_type(*this);
            }
            */
        };
        test_type test = test_type({this, typename test_type::stream_type(registry_, "test", "SimpleEntry", "SimpleOrderKey"), "test"});
    };

    RUN_TESTS<StreamManagerImpl>();
}
