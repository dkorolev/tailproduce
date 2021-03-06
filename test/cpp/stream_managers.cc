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
//   Stream manager operates on the level of publishers and listeners.
//
// * Making sure only one publisher can exist per stream.
//   Storage-level test that that a publisher would never overwrite data.
//   Updating the "HEAD" order key per stream.
//   Merging multiple streams maintaining strongly typed entries.
//   Ephemeral entry types as markers.
//   setTimeout()-style insertion of callbacks to be invoked by the framework later.
//
//   Framework is the level where the above is handler.

#include <gtest/gtest.h>

#include "../../src/tailproduce.h"
#include "../../src/order_key.h"

#include "helpers/storages.h"
#include "helpers/test_client.h"

using ::TailProduce::bytes;
using ::TailProduce::antibytes;
using ::TailProduce::StreamManagerParams;

// The actual test is a templated RUN_TESTS() function.
// It is used to test both the hand-crafted objects structure and the one created by a sequence of macros.
template <typename STORAGE, typename STREAM_MANAGER> void RUN_TESTS() {
    // A variable to hold the lambda is required in order to pass a reference to ProcessEntrySync().
    std::function<void(const SimpleEntry&)> lambda;

    {
        VLOG(2) << "Test that STREAM_MANAGER throws an exception when attempted to be created based on the storage "
                   "that does not contain a definition of the `test` stream.";
        STORAGE local_storage;
        std::unique_ptr<STREAM_MANAGER> p;
        ASSERT_THROW(p.reset(new STREAM_MANAGER(local_storage, StreamManagerParams())),
                     ::TailProduce::StreamDoesNotExistException);
        VLOG(2) << "Done.";
    }

    {
        VLOG(2) << "Test that STREAM_MANAGER throws an exception when attempted to be created based on the storage "
                   "that contains a malformed definition of the `test` stream.";
        STORAGE local_storage;
        local_storage.Set("s:test", bytes("foo"));
        std::unique_ptr<STREAM_MANAGER> p;
        ASSERT_THROW(p.reset(new STREAM_MANAGER(local_storage, StreamManagerParams())),
                     ::TailProduce::MalformedStorageHeadException);
        VLOG(2) << "Done.";
    }

    {
        VLOG(2) << "Test that STREAM_MANAGER can be created once the storage is externally set to contain the "
                   "proper definition of the `test` stream.";
        STORAGE local_storage;
        local_storage.Set("s:test", bytes("d:test:00000000000000000000"));
        STREAM_MANAGER streams_manager(local_storage, StreamManagerParams());
        VLOG(2) << "Done.";
    }

    {
        VLOG(2) << "Test that STREAM_MANAGER initializes the storage from the parameters provided in "
                   "StreamManagerParams().";
        STORAGE local_storage;
        ASSERT_FALSE(local_storage.Has("s:test"));
        // TODO(dkorolev): Change this logic to not require precise types from the caller.
        STREAM_MANAGER streams_manager(local_storage,
                                       StreamManagerParams().CreateStream("test", uint32_t(0), uint32_t(0)));
        ASSERT_TRUE(local_storage.Has("s:test"));
        ASSERT_EQ("d:test:00000000000000000000", antibytes(local_storage.Get("s:test")));
        VLOG(2) << "Done.";
    }

    {
        VLOG(2) << "Test that entries can be serialized and de-serialized.";
        SimpleEntry entry(1, "Test");
        std::ostringstream os;
        SimpleEntry::SerializeEntry(os, entry);
        std::string s = os.str();
        EXPECT_EQ("{\n    \"value0\": {\n        \"data\": \"Test\"\n    }\n}\n", s);
        {
            auto lambda = [](const SimpleEntry& restored) {
                EXPECT_EQ(1, restored.ikey);
                EXPECT_EQ("Test", restored.data);
            };
            std::istringstream is(s);
            SimpleEntry::DeSerializeAndProcessEntry(is, 1, lambda);
        }
        VLOG(2) << "Done.";
    }

    {
        VLOG(2) << "Test that the order key can be serialized and de-serialized to and from fixed size byte arrays.";
        SimpleEntry entry(42, "The Answer");
        uint32_t order_key;
        entry.GetOrderKey(order_key);
        std::string serialized_key;
        // order_key.SerializeOrderKey(serialized_key);
        serialized_key = ::TailProduce::FixedSizeSerialization::PackToString(order_key);
        EXPECT_EQ("0000000042", serialized_key);
        {
            uint32_t deserialized_order_key;
            // deserialized_order_key.DeSerializeOrderKey(serialized_key);
            deserialized_order_key = ::TailProduce::FixedSizeSerializer<uint32_t>::UnpackFromString(serialized_key);
            EXPECT_EQ(42, deserialized_order_key);
        }
        VLOG(2) << "Done.";
    }

    {
        // IMPORTANT: Keep this `storage` scoped!
        // Otherwise it would interfere with `local_storage`, which is fine for in-memory test storage,
        // but is *NOT* fine with LevelDB-based one.

        STORAGE storage;
        storage.Set("s:test", bytes("d:test:00000000000000000000"));

        {
            VLOG(2) << "Test stream manager setup. The `test` stream should exist and be statically typed.";
            STREAM_MANAGER streams_manager(storage, StreamManagerParams());
            EXPECT_TRUE((std::is_same<SimpleEntry, typename STREAM_MANAGER::test_type::T_ENTRY>::value));
            EXPECT_TRUE(
                (std::is_same<uint32_t, typename STREAM_MANAGER::test_type::T_ORDER_KEY::T_PRIMARY_KEY>::value));
            EXPECT_TRUE(
                (std::is_same<uint32_t, typename STREAM_MANAGER::test_type::T_ORDER_KEY::T_SECONDARY_KEY>::value));
            VLOG(2) << "Done.";
        }

        {
            VLOG(2) << "Test HEAD updates.";

            STREAM_MANAGER streams_manager(storage, StreamManagerParams());

            // Start from zero.
            typename STREAM_MANAGER::test_type::INTERNAL_unsafe_listener_type listener(streams_manager.test);

            size_t seen = 0;
            std::string last_as_string;
            auto lambda = [&seen, &last_as_string](const SimpleEntry& entry) {
                ++seen;
                std::ostringstream os;
                os << entry.ikey << ':' << entry.data;
                last_as_string = os.str();
            };
            auto test_listener_existence_scope = streams_manager.new_scoped_test_listener(lambda);
            EXPECT_EQ(0, seen);
            EXPECT_EQ("", last_as_string);
            typename STREAM_MANAGER::test_type::T_ORDER_KEY head;

            head = listener.GetHeadPrimaryAndSecondary();
            EXPECT_EQ(0, head.primary);
            EXPECT_EQ(0, head.secondary);

            // Instantiating a publisher does not change HEAD.
            {
                auto& publisher = streams_manager.test_publisher;
                head = publisher.GetHeadPrimaryAndSecondary();
                EXPECT_EQ(0, head.primary);
                EXPECT_EQ(0, head.secondary);
                head = listener.GetHeadPrimaryAndSecondary();
                EXPECT_EQ(0, head.primary);
                EXPECT_EQ(0, head.secondary);
            }

            // Push() and PushHead() change HEAD.
            // Secondary keys are incremented automatically.
            {
                auto& publisher = streams_manager.test_publisher;

                publisher.Push(SimpleEntry(1, "foo"));
                head = publisher.GetHeadPrimaryAndSecondary();
                EXPECT_EQ(1, head.primary);
                EXPECT_EQ(0, head.secondary);
                head = listener.GetHeadPrimaryAndSecondary();
                EXPECT_EQ(1, head.primary);
                EXPECT_EQ(0, head.secondary);
                EXPECT_EQ(bytes("d:test:00000000010000000000"), storage.Get("s:test"));
                test_listener_existence_scope->WaitUntilCurrent();
                EXPECT_EQ(1, seen);
                EXPECT_EQ("1:foo", last_as_string);

                publisher.Push(SimpleEntry(1, "bar"));
                head = publisher.GetHeadPrimaryAndSecondary();
                EXPECT_EQ(1, head.primary);
                EXPECT_EQ(1, head.secondary);
                head = listener.GetHeadPrimaryAndSecondary();
                EXPECT_EQ(1, head.primary);
                EXPECT_EQ(1, head.secondary);
                EXPECT_EQ(bytes("d:test:00000000010000000001"), storage.Get("s:test"));
                test_listener_existence_scope->WaitUntilCurrent();
                EXPECT_EQ(2, seen);
                EXPECT_EQ("1:bar", last_as_string);

                publisher.PushHead(uint32_t(2));
                head = publisher.GetHeadPrimaryAndSecondary();
                EXPECT_EQ(2, head.primary);
                EXPECT_EQ(0, head.secondary);
                head = listener.GetHeadPrimaryAndSecondary();
                EXPECT_EQ(2, head.primary);
                EXPECT_EQ(0, head.secondary);
                EXPECT_EQ(bytes("d:test:00000000020000000000"), storage.Get("s:test"));
                test_listener_existence_scope->WaitUntilCurrent();
                EXPECT_EQ(2, seen);
                EXPECT_EQ("1:bar", last_as_string);

                publisher.PushHead(uint32_t(2));
                head = publisher.GetHeadPrimaryAndSecondary();
                EXPECT_EQ(2, head.primary);
                EXPECT_EQ(1, head.secondary);
                head = listener.GetHeadPrimaryAndSecondary();
                EXPECT_EQ(2, head.primary);
                EXPECT_EQ(1, head.secondary);
                EXPECT_EQ(bytes("d:test:00000000020000000001"), storage.Get("s:test"));
                test_listener_existence_scope->WaitUntilCurrent();
                EXPECT_EQ(2, seen);
                EXPECT_EQ("1:bar", last_as_string);

                publisher.Push(SimpleEntry(100, "async"));
                test_listener_existence_scope->WaitUntilCurrent();
                EXPECT_EQ(3, seen);
                EXPECT_EQ("100:async", last_as_string);
                publisher.Push(SimpleEntry(101, "is ok"));
                test_listener_existence_scope->WaitUntilCurrent();
                EXPECT_EQ(4, seen);
                EXPECT_EQ("101:is ok", last_as_string);
            }

            /*
            // TODO(dkorolev): Move this test to a dedicated, Publisher-centric test case.
            // Instantiating a publisher starting from a fixed HEAD moves HEAD there.
            {
                typename STREAM_MANAGER::test_type::publisher_type publisher(streams_manager.test,
            uint32_t(10));
                head = publisher.GetHeadPrimaryAndSecondary();
                EXPECT_EQ(10, head.primary.ikey);
                EXPECT_EQ(0, head.secondary);
                head = listener.GetHeadPrimaryAndSecondary();
                EXPECT_EQ(10, head.primary.ikey);
                EXPECT_EQ(0, head.secondary);
                EXPECT_EQ(bytes("0000000010:0000000000"), storage.Get("s:test"));
            }
            */

            // Throws an exception attempting to move HEAD backwards when doing Push().
            {
                auto& publisher = streams_manager.test_publisher;
                ASSERT_THROW(publisher.Push(SimpleEntry(0, "boom")), ::TailProduce::OrderKeysGoBackwardsException);
            }

            // Throws an exception attempting to move HEAD backwards when doing PushHead().
            {
                auto& publisher = streams_manager.test_publisher;
                ASSERT_THROW(publisher.PushHead(uint32_t(0)), ::TailProduce::OrderKeysGoBackwardsException);
            }

            /*
            // Throws an exception attempting to start a publisher starting on the order key before the most recent
            one.
            {
                // TODO(dkorolev): Move this test to a dedicated, Publisher-centric test case.
                typedef typename STREAM_MANAGER::test_type::publisher_type T;
                std::unique_ptr<T> p;
                ASSERT_THROW(p.reset(new T(streams_manager.test, uint32_t(0))),
                             ::TailProduce::OrderKeysGoBackwardsException);
            }
            */
            VLOG(2) << "Done.";
        }
    }

    {
        VLOG(2) << "Test storage schema.";

        STORAGE local_storage;
        STREAM_MANAGER streams_manager(local_storage,
                                       StreamManagerParams().CreateStream("test", uint32_t(0), uint32_t(0)));

        auto& publisher = streams_manager.test_publisher;
        publisher.Push(SimpleEntry(1, "one"));
        publisher.Push(SimpleEntry(2, "two"));
        publisher.Push(SimpleEntry(3, "three"));

        EXPECT_EQ(bytes("d:test:00000000030000000000"), local_storage.Get("s:test"));
        EXPECT_EQ(bytes("{\n    \"value0\": {\n        \"data\": \"one\"\n    }\n}\n"),
                  local_storage.Get("d:test:00000000010000000000"));
        EXPECT_EQ(bytes("{\n    \"value0\": {\n        \"data\": \"two\"\n    }\n}\n"),
                  local_storage.Get("d:test:00000000020000000000"));
        EXPECT_EQ(bytes("{\n    \"value0\": {\n        \"data\": \"three\"\n    }\n}\n"),
                  local_storage.Get("d:test:00000000030000000000"));
        VLOG(2) << "Done.";
    }

    {
        VLOG(2) << "Listener test: bounded, pre-initialized with data.";

        STORAGE local_storage;
        // TODO(dkorolev): Fix this CreateStream().
        STREAM_MANAGER streams_manager(local_storage,
                                       StreamManagerParams().CreateStream("test", uint32_t(0), uint32_t(0)));

        auto& publisher = streams_manager.test_publisher;
        publisher.Push(SimpleEntry(1, "one"));
        publisher.Push(SimpleEntry(2, "two"));
        publisher.Push(SimpleEntry(3, "three"));
        publisher.Push(SimpleEntry(4, "four"));
        publisher.Push(SimpleEntry(5, "five"));

        SimpleEntry entry;
        typename STREAM_MANAGER::test_type::INTERNAL_unsafe_listener_type listener(
            streams_manager.test, uint32_t(2), uint32_t(4));
        ASSERT_TRUE(listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());
        listener.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(2, entry.ikey);
            EXPECT_EQ("two", entry.data);
        });
        listener.AdvanceToNextEntry();
        ASSERT_TRUE(listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());
        listener.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(3, entry.ikey);
            EXPECT_EQ("three", entry.data);
        });
        listener.AdvanceToNextEntry();
        EXPECT_FALSE(listener.HasData());
        EXPECT_TRUE(listener.ReachedEnd());
        ASSERT_THROW(listener.AdvanceToNextEntry(), ::TailProduce::AttemptedToAdvanceListenerWithNoDataAvailable);

        VLOG(2) << "Done.";
    }

    {
        VLOG(2) << "Listener test: bounded, pre-initialized with data, involving secondary keys.";

        STORAGE local_storage;
        // TODO(dkorolev): Fix this CreateStream().
        STREAM_MANAGER streams_manager(local_storage,
                                       StreamManagerParams().CreateStream("test", uint32_t(0), uint32_t(0)));

        auto& publisher = streams_manager.test_publisher;
        publisher.Push(SimpleEntry(42, "i0"));
        publisher.Push(SimpleEntry(42, "i1"));
        publisher.Push(SimpleEntry(42, "i2"));
        publisher.Push(SimpleEntry(42, "i3"));
        publisher.Push(SimpleEntry(42, "i4"));
        publisher.Push(SimpleEntry(42, "i5"));
        publisher.Push(SimpleEntry(42, "i6"));

        SimpleEntry entry;
        typename STREAM_MANAGER::test_type::INTERNAL_unsafe_listener_type listener(
            streams_manager.test,
            typename STREAM_MANAGER::test_type::T_ORDER_KEY(uint32_t(42), 2),
            typename STREAM_MANAGER::test_type::T_ORDER_KEY(uint32_t(42), 5));
        ASSERT_TRUE(!listener.ReachedEnd());
        ASSERT_TRUE(listener.HasData());
        listener.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(42, entry.ikey);
            EXPECT_EQ("i2", entry.data);
        });
        listener.AdvanceToNextEntry();
        ASSERT_TRUE(listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());
        listener.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(42, entry.ikey);
            EXPECT_EQ("i3", entry.data);
        });
        listener.AdvanceToNextEntry();
        ASSERT_TRUE(listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());
        listener.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(42, entry.ikey);
            EXPECT_EQ("i4", entry.data);
        });
        listener.AdvanceToNextEntry();
        EXPECT_FALSE(listener.HasData());
        EXPECT_TRUE(listener.ReachedEnd());
        ASSERT_THROW(listener.AdvanceToNextEntry(), ::TailProduce::AttemptedToAdvanceListenerWithNoDataAvailable);

        VLOG(2) << "Done.";
    }

    {
        VLOG(2) << "Listener test: appended on-the-fly, bounded.";
        STORAGE local_storage;
        // TODO(dkorolev): Fix this CreateStream().
        STREAM_MANAGER streams_manager(local_storage,
                                       StreamManagerParams().CreateStream("test", uint32_t(0), uint32_t(0)));

        SimpleEntry entry;
        auto& publisher = streams_manager.test_publisher;
        typename STREAM_MANAGER::test_type::INTERNAL_unsafe_listener_type listener(
            streams_manager.test, uint32_t(10), uint32_t(20));

        publisher.Push(SimpleEntry(5, "five: ignored as before the beginning of the range"));
        ASSERT_TRUE(!listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());

        publisher.Push(SimpleEntry(10, "ten"));
        ASSERT_TRUE(listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());
        listener.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(10, entry.ikey);
            EXPECT_EQ("ten", entry.data);
        });
        listener.AdvanceToNextEntry();
        ASSERT_TRUE(!listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());

        publisher.Push(SimpleEntry(15, "fifteen"));
        ASSERT_TRUE(listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());
        listener.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(15, entry.ikey);
            EXPECT_EQ("fifteen", entry.data);
        });
        listener.AdvanceToNextEntry();
        ASSERT_TRUE(!listener.HasData());
        ASSERT_TRUE(!listener.ReachedEnd());

        publisher.Push(SimpleEntry(20, "twenty: ignored as part the non-included end the of range"));
        ASSERT_TRUE(!listener.HasData());
        ASSERT_TRUE(listener.ReachedEnd());
        ASSERT_THROW(listener.ProcessEntrySync(lambda = [](SimpleEntry) {}), ::TailProduce::ListenerHasNoDataToRead);
        ASSERT_THROW(listener.AdvanceToNextEntry(), ::TailProduce::AttemptedToAdvanceListenerWithNoDataAvailable);

        VLOG(2) << "Done.";
    }

    {
        // Listener test: appended on-the-fly, bounded, test that PushHead() makes ReachedEnd() return true.
        // TODO(dkorolev): Code it.
    }

    if (false) {
        // TCP endpoint test for the EXPORT-ed stream.
        // TODO(dkorolev): Capture the output and test it from within this unit test, not just via curl/telnet.
        STORAGE local_storage;
        // TODO(dkorolev): Fix this CreateStream().
        STREAM_MANAGER streams_manager(local_storage,
                                       StreamManagerParams().CreateStream("test", uint32_t(0), uint32_t(0)));

        auto& publisher = streams_manager.test_publisher;

        VLOG(2) << "Publishing and exporting a stream for three seconds. Run curl or telnet now to test.";
        for (int i = 100; i <= 300; ++i) {
            publisher.Push(SimpleEntry(i, "TCP Test"));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        VLOG(2) << "Three seconds have passed.";
    }
}

template <typename T> class StreamManagerTest : public ::testing::Test {};
TYPED_TEST_CASE(StreamManagerTest, TestStreamManagerImplementationsTypeList);

// Runs the tests against the static framework defined by macros.
TYPED_TEST(StreamManagerTest, UserFriendlySyntaxCompiles) {
    TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(StreamManagerImpl, TypeParam);
    TAILPRODUCE_STREAM(test, SimpleEntry, uint32_t, uint32_t);
    TAILPRODUCE_PUBLISHER(test);
    TAILPRODUCE_EXPORT_STREAM(test);
    TAILPRODUCE_STATIC_FRAMEWORK_END();

    RUN_TESTS<typename StreamManagerImpl::T_STORAGE, StreamManagerImpl>();
}

// Runs the test against explicitly defined static framework.
// Used as a reference point, as well as to ensure the macros do what they are designed for.
TYPED_TEST(StreamManagerTest, ExpandedMacroSyntaxCompiles) {
    // TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(StreamManagerImpl, TypeParam);
    struct StreamManagerImpl : ::TailProduce::StaticFramework<TypeParam> {
        typedef StreamManagerImpl T_THIS_FRAMEWORK_INSTANCE;
        typedef ::TailProduce::StaticFramework<TypeParam> T_FRAMEWORK;
        typedef typename T_FRAMEWORK::T_STORAGE T_STORAGE;
        StreamManagerImpl(T_STORAGE& storage,
                          const ::TailProduce::StreamManagerParams& params =
                              ::TailProduce::StreamManagerParams::FromCommandLineFlags())
            : T_FRAMEWORK(storage, params) {
        }

        // TAILPRODUCE_STREAM(test, SimpleEntry, uint32_t, uint32_t);
        struct test_type_params {
            struct StreamTraits {
                typedef typename T_FRAMEWORK::T_STORAGE T_STORAGE;
                const std::string name;
                const std::string storage_key_meta_prefix;
                const std::string storage_key_data_prefix;
                const std::string starting_order_key_as_string;
                explicit StreamTraits(const ::TailProduce::ConfigValues& cv)
                    : name("test"),
                      storage_key_meta_prefix(cv.GetStreamMetaPrefix(*this)),
                      storage_key_data_prefix(cv.GetStreamDataPrefix(*this)) {
                }
            };
            typedef ::TailProduce::OrderKey<StreamTraits, uint32_t, uint32_t> T_ORDER_KEY;
        };
        struct test_type : ::TailProduce::Stream<typename test_type_params::StreamTraits,
                                                 SimpleEntry,
                                                 typename test_type_params::T_ORDER_KEY> {
            typedef ::TailProduce::Stream<typename test_type_params::StreamTraits,
                                          SimpleEntry,
                                          typename test_type_params::T_ORDER_KEY> T_STREAM_INSTANCE;
            typedef test_type T_STREAM;
            typedef SimpleEntry T_ENTRY;
            typedef typename test_type_params::T_ORDER_KEY T_ORDER_KEY;
            typedef typename T_THIS_FRAMEWORK_INSTANCE::T_STORAGE T_STORAGE;
            typedef ::TailProduce::INTERNAL_UnsafeListener<test_type> INTERNAL_unsafe_listener_type;
            T_THIS_FRAMEWORK_INSTANCE* manager_;
            mutable ::TailProduce::SubscriptionsManager subscriptions_;
            test_type(T_THIS_FRAMEWORK_INSTANCE* manager,
                      const char* stream_name,
                      const char* entry_type_name,
                      const char* entry_order_key_name)
                : T_STREAM_INSTANCE(manager->cv, manager->storage), manager_(manager) {
                manager_->streams_declared_.insert("test");
            }
        };
        test_type test = test_type(this, "test", "uint32_t", "uint32_t");
        ::TailProduce::AsyncListenersFactory<test_type> new_scoped_test_listener =
            ::TailProduce::AsyncListenersFactory<test_type>(test);

        // TAILPRODUCE_PUBLISHER(test);
        struct test_publisher_type : ::TailProduce::Publisher<test_type> {
            typedef ::TailProduce::Publisher<test_type> base;
            explicit test_publisher_type(T_THIS_FRAMEWORK_INSTANCE* manager) : base(manager->test) {
                manager->stream_publishers_declared_.insert("test");
            }
        };
        test_publisher_type test_publisher = test_publisher_type(this);

        // TAILPRODUCE_EXPORT_STREAM(test);
        struct test_exporter_type : ::TailProduce::StreamExporter {
            T_THIS_FRAMEWORK_INSTANCE* manager_;
            explicit test_exporter_type(T_THIS_FRAMEWORK_INSTANCE* manager) : manager_(manager) {
                manager_->AddExporter("/test", this);
            }
            ~test_exporter_type() {
                manager_->RemoveExporter("/test", this);
            }
            void ListenAndStreamData(std::unique_ptr<boost::asio::ip::tcp::socket>&& socket) {
                // TODO(dkorolev): Handle shutdown correctly.
                auto lambda = [&socket](const SimpleEntry& entry) {
                    std::ostringstream os;
                    os << entry.ikey << ' ' << entry.data << '\n';
                    std::string message = os.str();
                    boost::asio::write(*socket, boost::asio::buffer(message), boost::asio::transfer_all());
                };
                auto scope = manager_->new_scoped_test_listener(lambda);
                // TODO(dkorolev): Join threads or do something else clever.
                while (true) {
                }
            }
        };
        test_exporter_type test_exporter = test_exporter_type(this);

        // TAILPRODUCE_STATIC_FRAMEWORK_END();
    };

    RUN_TESTS<typename StreamManagerImpl::T_STORAGE, StreamManagerImpl>();
}
