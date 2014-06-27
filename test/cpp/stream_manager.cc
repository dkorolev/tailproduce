// TODO(dkorolev): Save HEAD-s into storage and test cold starts!
// TODO(dkorolev): Add --create_streams flags and ensure that the service can not start w/o them.

// The test for STREAM MANAGER confirms that:
//
// 1. Stream manager binds to the data storage, be it the production one or the mock one.
// 2. Stream manager can be used to instantiate a (statically defined) set of streams.
// 3. The macros to define the set of streams do their job.
// 4. Serialization and de-serialization of entries of their respective types works.
// 5. Serialization and de-serialization of order keys of their respective types works.
// 6. Listeners and publishers can be created and behave as expected.
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

using ::TailProduce::bytes;

// TODO(dkorolev): Add a death test for the case when the storage is not yet initalized.

// The actual test is a templated RUN_TESTS() function.
// It is used to test both the hand-crafted objects structure and the one created by a sequence of macros.
template<typename STREAM_MANAGER> void RUN_TESTS() {
    {
        // Test stream manager setup.
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
    }

    {
       // Test that entries can be serialized and de-serialized into C++ streams.
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

    {
        // Test that the order key can be serialized and de-serialized to and from fixed size byte arrays.
        SimpleEntry entry(42, "The Answer");
        SimpleOrderKey order_key = ::TailProduce::OrderKeyExtractorImpl<SimpleOrderKey, SimpleEntry>::ExtractOrderKey(entry);
        uint8_t serialized_key[SimpleOrderKey::size_in_bytes];
        order_key.SerializeOrderKey(serialized_key);
        EXPECT_EQ("0000000042", std::string(serialized_key, serialized_key + sizeof(serialized_key)));
        {
            SimpleOrderKey deserialized_order_key;
            deserialized_order_key.DeSerializeOrderKey(serialized_key);
            EXPECT_EQ(42, deserialized_order_key.key);
        }
    }

    {
        // Test HEAD updates.
        STREAM_MANAGER streams_manager;

        // Start from zero.
        typename STREAM_MANAGER::test_type::unsafe_listener_type listener(streams_manager.test);
        typename STREAM_MANAGER::test_type::head_pair_type head;

        head = listener.GetHead();
        EXPECT_EQ(0, head.first.key);
        EXPECT_EQ(0, head.second);
    
        // Instantiating a publisher does not change HEAD.
        {
            typename STREAM_MANAGER::test_type::unsafe_publisher_type publisher(streams_manager.test);
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
            typename STREAM_MANAGER::test_type::unsafe_publisher_type publisher(streams_manager.test);

            publisher.Push(SimpleEntry(1, "foo"));
            head = publisher.GetHead();
            EXPECT_EQ(1, head.first.key);
            EXPECT_EQ(0, head.second);
            head = listener.GetHead();
            EXPECT_EQ(1, head.first.key);
            EXPECT_EQ(0, head.second);
            EXPECT_EQ(bytes("0000000001:0000000000"), streams_manager.storage.Get(bytes("s:test")));

            publisher.Push(SimpleEntry(1, "bar"));
            head = publisher.GetHead();
            EXPECT_EQ(1, head.first.key);
            EXPECT_EQ(1, head.second);
            head = listener.GetHead();
            EXPECT_EQ(1, head.first.key);
            EXPECT_EQ(1, head.second);
            EXPECT_EQ(bytes("0000000001:0000000001"), streams_manager.storage.Get(bytes("s:test")));

            publisher.PushHead(SimpleOrderKey(2));
            head = publisher.GetHead();
            EXPECT_EQ(2, head.first.key);
            EXPECT_EQ(0, head.second);
            head = listener.GetHead();
            EXPECT_EQ(2, head.first.key);
            EXPECT_EQ(0, head.second);
            EXPECT_EQ(bytes("0000000002:0000000000"), streams_manager.storage.Get(bytes("s:test")));

            publisher.PushHead(SimpleOrderKey(2));
            head = publisher.GetHead();
            EXPECT_EQ(2, head.first.key);
            EXPECT_EQ(1, head.second);
            head = listener.GetHead();
            EXPECT_EQ(2, head.first.key);
            EXPECT_EQ(1, head.second);
            EXPECT_EQ(bytes("0000000002:0000000001"), streams_manager.storage.Get(bytes("s:test")));
        }

        // Instantiating a publisher starting from a fixed HEAD moves HEAD there.
        {
            typename STREAM_MANAGER::test_type::unsafe_publisher_type publisher(streams_manager.test,
                                                                                SimpleOrderKey(10));
            head = publisher.GetHead();
            EXPECT_EQ(10, head.first.key);
            EXPECT_EQ(0, head.second);
            head = listener.GetHead();
            EXPECT_EQ(10, head.first.key);
            EXPECT_EQ(0, head.second);
            EXPECT_EQ(bytes("0000000010:0000000000"), streams_manager.storage.Get(bytes("s:test")));
        }

        // Throws an exception attempting to move the HEAD backwards when doing Push().
        {
            typename STREAM_MANAGER::test_type::unsafe_publisher_type publisher(streams_manager.test);
            ASSERT_THROW(publisher.Push(SimpleEntry(0, "boom")), ::TailProduce::OrderKeysGoBackwardsException);
        }

        // Throws an exception attempting to move the HEAD backwards when doing PushHead().
        {
            typename STREAM_MANAGER::test_type::unsafe_publisher_type publisher(streams_manager.test);
            ASSERT_THROW(publisher.PushHead(SimpleOrderKey(0)), ::TailProduce::OrderKeysGoBackwardsException);
        }

        // Throws an exception attempting to start a publisher starting on the order key before the most recent one.
        {
            typedef typename STREAM_MANAGER::test_type::unsafe_publisher_type T;
            std::unique_ptr<T> p;
            ASSERT_THROW(p.reset(new T(streams_manager.test, SimpleOrderKey(0))), ::TailProduce::OrderKeysGoBackwardsException);
        }
    }

    {
        // Test storage schema.
        STREAM_MANAGER streams_manager;

        typename STREAM_MANAGER::test_type::unsafe_publisher_type publisher(streams_manager.test);
        publisher.Push(SimpleEntry(1, "one"));
        publisher.Push(SimpleEntry(2, "two"));
        publisher.Push(SimpleEntry(3, "three"));

        EXPECT_EQ(bytes("0000000003:0000000000"), streams_manager.storage.Get(bytes("s:test")));
        EXPECT_EQ(bytes("{\n    \"value0\": {\n        \"key\": 1,\n        \"data\": \"one\"\n    }\n}\n"), streams_manager.storage.Get(bytes("d:test:0000000001:0000000000")));
        EXPECT_EQ(bytes("{\n    \"value0\": {\n        \"key\": 2,\n        \"data\": \"two\"\n    }\n}\n"), streams_manager.storage.Get(bytes("d:test:0000000002:0000000000")));
        EXPECT_EQ(bytes("{\n    \"value0\": {\n        \"key\": 3,\n        \"data\": \"three\"\n    }\n}\n"), streams_manager.storage.Get(bytes("d:test:0000000003:0000000000")));
    }

    {
        // Pre-initialized with data bounded iterator test.
        STREAM_MANAGER streams_manager;

        typename STREAM_MANAGER::test_type::unsafe_publisher_type publisher(streams_manager.test);
        publisher.Push(SimpleEntry(1, "one"));
        publisher.Push(SimpleEntry(2, "two"));
        publisher.Push(SimpleEntry(3, "three"));
        publisher.Push(SimpleEntry(4, "four"));
        publisher.Push(SimpleEntry(5, "five"));

        SimpleEntry entry;
        typename STREAM_MANAGER::test_type::unsafe_listener_type listener(streams_manager.test, SimpleOrderKey(2), SimpleOrderKey(4));
        ASSERT_TRUE(listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());
        listener.ExportEntry(entry);
        EXPECT_EQ(2, entry.key);
        EXPECT_EQ("two", entry.data);
        listener.AdvanceToNextEntry();
        ASSERT_TRUE(listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());
        listener.ExportEntry(entry);
        EXPECT_EQ(3, entry.key);
        EXPECT_EQ("three", entry.data);
        listener.AdvanceToNextEntry();
        EXPECT_FALSE(listener.HasData());
        EXPECT_TRUE(listener.ReachedEnd());
        ASSERT_THROW(listener.AdvanceToNextEntry(), ::TailProduce::AttemptedToAdvanceListenerWithNoDataAvailable);
    }

    {
        // Pre-initialized with data bounded iterator test involving secondary keys.
        STREAM_MANAGER streams_manager;

        typename STREAM_MANAGER::test_type::unsafe_publisher_type publisher(streams_manager.test);
        publisher.Push(SimpleEntry(42, "i0"));
        publisher.Push(SimpleEntry(42, "i1"));
        publisher.Push(SimpleEntry(42, "i2"));
        publisher.Push(SimpleEntry(42, "i3"));
        publisher.Push(SimpleEntry(42, "i4"));
        publisher.Push(SimpleEntry(42, "i5"));
        publisher.Push(SimpleEntry(42, "i6"));

        SimpleEntry entry;
        typename STREAM_MANAGER::test_type::unsafe_listener_type listener(
            streams_manager.test,
            std::make_pair(SimpleOrderKey(42), 2),
            std::make_pair(SimpleOrderKey(42), 5));
        ASSERT_TRUE(!listener.ReachedEnd());
        ASSERT_TRUE(listener.HasData());
        listener.ExportEntry(entry);
        EXPECT_EQ(42, entry.key);
        EXPECT_EQ("i2", entry.data);
        listener.AdvanceToNextEntry();
        ASSERT_TRUE(listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());
        listener.ExportEntry(entry);
        EXPECT_EQ(42, entry.key);
        EXPECT_EQ("i3", entry.data);
        listener.AdvanceToNextEntry();
        ASSERT_TRUE(listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());
        listener.ExportEntry(entry);
        EXPECT_EQ(42, entry.key);
        EXPECT_EQ("i4", entry.data);
        listener.AdvanceToNextEntry();
        EXPECT_FALSE(listener.HasData());
        EXPECT_TRUE(listener.ReachedEnd());
        ASSERT_THROW(listener.AdvanceToNextEntry(), ::TailProduce::AttemptedToAdvanceListenerWithNoDataAvailable);
    }

    {
        // Dynamic bounded iterator test.
        STREAM_MANAGER streams_manager;

        SimpleEntry entry;
        typename STREAM_MANAGER::test_type::unsafe_publisher_type publisher(streams_manager.test);
        typename STREAM_MANAGER::test_type::unsafe_listener_type listener(streams_manager.test, SimpleOrderKey(10), SimpleOrderKey(20));

        publisher.Push(SimpleEntry(5, "five: ignored as before the beginning of the range"));
        ASSERT_TRUE(!listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());

        publisher.Push(SimpleEntry(10, "ten"));
        ASSERT_TRUE(listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());
        listener.ExportEntry(entry);
        EXPECT_EQ(10, entry.key);
        EXPECT_EQ("ten", entry.data);
        listener.AdvanceToNextEntry();
        ASSERT_TRUE(!listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());

        publisher.Push(SimpleEntry(15, "fifteen"));
        ASSERT_TRUE(listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());
        listener.ExportEntry(entry);
        EXPECT_EQ(15, entry.key);
        EXPECT_EQ("fifteen", entry.data);
        listener.AdvanceToNextEntry();
        ASSERT_TRUE(!listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());

        publisher.Push(SimpleEntry(20, "twenty: ignored as part the non-included end the of range"));
        ASSERT_TRUE(!listener.HasData());
        ASSERT_TRUE(listener.ReachedEnd());
        ASSERT_THROW(listener.ExportEntry(entry), ::TailProduce::ListenerHasNoDataToRead);
        ASSERT_THROW(listener.AdvanceToNextEntry(), ::TailProduce::AttemptedToAdvanceListenerWithNoDataAvailable);
    }

    // Test that the stream can be listened to.
    // Test that multiple listeners can co-exist and that listeners support ranes.
    /*
    typename STREAM_MANAGER::test_type::unsafe_listener_type listener(streams_manager);
    typename STREAM_MANAGER::test_type::unsafe_listener_type second_listener(streams_manager, SimpleOrderKey(50));
    typename STREAM_MANAGER::test_type::unsafe_listener_type third_listener(streams_manager,
                                                                     SimpleOrderKey(150),
                                                                     SimpleOrderKey(250));

    EXPECT_TRUE(!listener.HasData());
    EXPECT_TRUE(!second_listener.HasData());
    EXPECT_TRUE(!third_listener.HasData());
    */

    // Test that the stream can be appended to.
    {
        STREAM_MANAGER streams_manager;
        typename STREAM_MANAGER::test_type::unsafe_publisher_type publisher(streams_manager.test);
        SimpleEntry entry;
        entry.key = 42;
        entry.data = "The Answer";
        SimpleOrderKey order_key = ::TailProduce::OrderKeyExtractorImpl<SimpleOrderKey, SimpleEntry>::ExtractOrderKey(entry);
        publisher.Push(entry);
    }

    /*
    {
        // Implicit typing, if we decide we need it.
        auto publisher = streams_manager.test.publisher();
        SimpleEntry entry;
        entry.key = 42;
        entry.data = "The Answer";
        SimpleOrderKey order_key = entry.template GetOrderKey<SimpleOrderKey>();
        publisher.Push(entry);
    }
    */
    
    // Test that the listeners get updates.

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
TYPED_TEST(StreamManagerTest, ExpandedMacroSyntaxCompiles) {
    class StreamManagerImpl : public TypeParam {
      private:
        using TSM = ::TailProduce::StreamManager;
        static_assert(std::is_base_of<TSM, TypeParam>::value, "StreamManagerImpl: TypeParam should be derived from StreamManager.");
        using TS = ::TailProduce::Storage;
        static_assert(std::is_base_of<TS, typename TypeParam::storage_type>::value, "StreamManagerImpl: TypeParam::storage_type should be derived from Storage.");
        ::TailProduce::StreamsRegistry registry_;
      public:
        const ::TailProduce::StreamsRegistry& registry() const { return registry_; }
        struct test_type {
            typedef SimpleEntry entry_type;
            typedef SimpleOrderKey order_key_type;
            typedef ::TailProduce::StreamInstance<entry_type, order_key_type> stream_type;
            typedef typename TypeParam::storage_type storage_type;
            typedef ::TailProduce::UnsafeListener<test_type> unsafe_listener_type;
            typedef ::TailProduce::UnsafePublisher<test_type> unsafe_publisher_type;
            typedef std::pair<order_key_type, uint32_t> head_pair_type;
            StreamManagerImpl* manager;
            stream_type stream;
            const std::string name;
            head_pair_type head;
            const std::vector<uint8_t> head_storage_key;
            /*
            // To enable within the framework. So far on the Storage + Streams Manager level.
            std::unique_ptr<std::mutex> p_mutex;
            std::mutex& mutex() {
                if (!p_mutex.get()) {
                    p_mutex.reset(new std::mutex());
                }
                return p_mutex.get();
            }
            */
            /*
            // Gotta figure out move semantics -- D.K.
            unsafe_listener_type&& listener() const {
                return unsafe_listener_type(*this);
            }
            unsafe_publisher_type publisher() {
                return unsafe_publisher_type(*this);
            }
            */
            test_type(StreamManagerImpl* manager, const char* stream_name, const char* entry_type_name, const char* entry_order_key_name)
              : manager(manager),
              stream(manager->registry_, stream_name, entry_type_name, entry_order_key_name),
              name(stream_name),
              head_storage_key(bytes("s:" + name)) {
            }
        };
        test_type test = test_type(this, "test", "SimpleEntry", "SimpleOrderKey");
    };
//    typedef typename StreamManagerImpl::storage_type TMP;

    RUN_TESTS<StreamManagerImpl>();
}
