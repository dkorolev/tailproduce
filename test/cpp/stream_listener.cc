// The smoke test to illustrate the functionality of the stream manager.
// Please refer to stream_manager.cc for a more complete test.

#include <string>
#include <sstream>

#include <gtest/gtest.h>

#include "../../src/tailproduce.h"

#include "helpers/storages.h"
#include "helpers/test_client.h"

using ::TailProduce::bytes;
using ::TailProduce::StreamManagerParams;

struct StatsAggregator {
    size_t count = 0;
    std::string data = "";
    void operator()(const SimpleEntry& entry) {
        std::ostringstream os;
        os << '[' << count << "]:{" << entry.ikey << ",'" << entry.data << "'}";
        if (count) {
            data.append(",");
        }
        data.append(os.str());
        ++count;
    }
};

TEST(StreamManagerSmokeTest, SmokeTest) {
    TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(Impl, ::TailProduce::StreamManager<InMemoryTestStorage>);
    TAILPRODUCE_STREAM(test, SimpleEntry, uint32_t, uint32_t);
    TAILPRODUCE_PUBLISHER(test);
    TAILPRODUCE_STATIC_FRAMEWORK_END();

    InMemoryTestStorage storage;

    // A variable to hold the lambda is required in order to pass a reference to ProcessEntrySync().
    std::function<void(const SimpleEntry&)> lambda;

    {
        // Mimic the 1st run with the command line flag set to initialize the stream in the storage.
        Impl streams_manager(storage, StreamManagerParams().CreateStream("test", uint32_t(0), uint32_t(0)));
    }

    {
        // Mimic the consecutive run(s) that rely on the fact that the stream exists.
        Impl streams_manager(storage, StreamManagerParams());

        StatsAggregator stats;
        auto test_listener_existence_scope = streams_manager.new_scoped_test_listener(stats);

        SimpleEntry entry;

        typename Impl::test_publisher_type& publisher = streams_manager.test_publisher;
        typename Impl::test_type::INTERNAL_unsafe_listener_type listener_all(streams_manager.test);
        typename Impl::test_type::INTERNAL_unsafe_listener_type listener_from_three(streams_manager.test,
                                                                                    uint32_t(3));
        typename Impl::test_type::INTERNAL_unsafe_listener_type listener_from_three_to_five_not_inclusive(
            streams_manager.test, uint32_t(3), uint32_t(5));

        ASSERT_FALSE(listener_all.HasData());
        ASSERT_FALSE(listener_all.ReachedEnd());

        ASSERT_FALSE(listener_from_three.HasData());
        ASSERT_FALSE(listener_from_three.ReachedEnd());

        ASSERT_FALSE(listener_from_three_to_five_not_inclusive.HasData());
        ASSERT_FALSE(listener_from_three_to_five_not_inclusive.ReachedEnd());

        publisher.Push(SimpleEntry(1, "one"));
        publisher.Push(SimpleEntry(2, "two"));

        ASSERT_TRUE(listener_all.HasData());
        ASSERT_FALSE(listener_all.ReachedEnd());
        listener_all.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(1, entry.ikey);
            EXPECT_EQ("one", entry.data);
        });
        listener_all.AdvanceToNextEntry();
        listener_all.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(2, entry.ikey);
            EXPECT_EQ("two", entry.data);
        });
        listener_all.AdvanceToNextEntry();
        ASSERT_FALSE(listener_all.HasData());
        ASSERT_FALSE(listener_from_three.HasData());
        ASSERT_FALSE(listener_from_three.ReachedEnd());
        ASSERT_FALSE(listener_from_three_to_five_not_inclusive.ReachedEnd());
        ASSERT_FALSE(listener_from_three_to_five_not_inclusive.HasData());

        publisher.Push(SimpleEntry(3, "three"));
        publisher.Push(SimpleEntry(4, "four"));

        ASSERT_TRUE(listener_all.HasData());
        ASSERT_FALSE(listener_all.ReachedEnd());
        listener_all.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(3, entry.ikey);
            EXPECT_EQ("three", entry.data);
        });
        listener_all.AdvanceToNextEntry();
        listener_all.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(4, entry.ikey);
            EXPECT_EQ("four", entry.data);
        });
        listener_all.AdvanceToNextEntry();
        ASSERT_FALSE(listener_all.HasData());

        ASSERT_TRUE(listener_from_three.HasData());
        ASSERT_FALSE(listener_from_three.ReachedEnd());
        listener_from_three.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(3, entry.ikey);
            EXPECT_EQ("three", entry.data);
        });
        listener_from_three.AdvanceToNextEntry();
        listener_from_three.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(4, entry.ikey);
            EXPECT_EQ("four", entry.data);
        });
        listener_from_three.AdvanceToNextEntry();
        ASSERT_FALSE(listener_from_three.HasData());

        ASSERT_TRUE(listener_from_three_to_five_not_inclusive.HasData());
        ASSERT_FALSE(listener_from_three_to_five_not_inclusive.ReachedEnd());
        listener_from_three_to_five_not_inclusive.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(3, entry.ikey);
            EXPECT_EQ("three", entry.data);
        });
        listener_from_three_to_five_not_inclusive.AdvanceToNextEntry();
        listener_from_three_to_five_not_inclusive.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(4, entry.ikey);
            EXPECT_EQ("four", entry.data);
        });
        listener_from_three_to_five_not_inclusive.AdvanceToNextEntry();
        ASSERT_FALSE(listener_from_three_to_five_not_inclusive.HasData());

        publisher.Push(SimpleEntry(5, "five"));
        publisher.Push(SimpleEntry(6, "six"));
        publisher.Push(SimpleEntry(7, "seven"));

        ASSERT_TRUE(listener_all.HasData());
        ASSERT_FALSE(listener_all.ReachedEnd());
        listener_all.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(5, entry.ikey);
            EXPECT_EQ("five", entry.data);
        });
        listener_all.AdvanceToNextEntry();
        listener_all.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(6, entry.ikey);
            EXPECT_EQ("six", entry.data);
        });
        listener_all.AdvanceToNextEntry();
        listener_all.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(7, entry.ikey);
            EXPECT_EQ(("seven"), entry.data);
        });
        listener_all.AdvanceToNextEntry();
        ASSERT_FALSE(listener_all.HasData());

        ASSERT_TRUE(listener_from_three.HasData());
        ASSERT_FALSE(listener_from_three.ReachedEnd());
        listener_from_three.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(5, entry.ikey);
            EXPECT_EQ(("five"), entry.data);
        });
        listener_from_three.AdvanceToNextEntry();
        listener_from_three.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(6, entry.ikey);
            EXPECT_EQ(("six"), entry.data);
        });
        listener_from_three.AdvanceToNextEntry();
        listener_from_three.ProcessEntrySync(lambda = [](const SimpleEntry& entry) {
            EXPECT_EQ(7, entry.ikey);
            EXPECT_EQ(("seven"), entry.data);
        });
        listener_from_three.AdvanceToNextEntry();
        ASSERT_FALSE(listener_from_three.HasData());

        ASSERT_FALSE(listener_from_three_to_five_not_inclusive.HasData());
        ASSERT_TRUE(listener_from_three_to_five_not_inclusive.ReachedEnd());

        test_listener_existence_scope->WaitUntilCurrent();
        std::string golden =
            "[0]:{1,'one'},[1]:{2,'two'},[2]:{3,'three'},[3]:{4,'four'},[4]:{5,'five'},[5]:{6,'six'},[6]:{7,'seven'"
            "}";
        ASSERT_EQ(golden, stats.data);
    }
}

TEST(StreamManagerSmokeTest, DataInjected) {
    TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(Impl, ::TailProduce::StreamManager<InMemoryTestStorage>);
    TAILPRODUCE_STREAM(foo, SimpleEntry, uint32_t, uint32_t);
    TAILPRODUCE_PUBLISHER(foo);
    TAILPRODUCE_STATIC_FRAMEWORK_END();

    InMemoryTestStorage storage;

    {
        // Mimic the 1st run with the command line flag set to initialize the stream in the storage.
        Impl streams_manager(storage, StreamManagerParams().CreateStream("foo", uint32_t(100), uint32_t(0)));
    }

    // Add one data entry.
    storage.Set("d:foo:0000000042:0000000000", bytes("{\"value0\":{\"ikey\":42,\"data\":\"Yay!\"}}"));

    {
        // Mimic the consecutive run(s) that rely on the fact that the stream exists.
        Impl streams_manager(storage, StreamManagerParams());
        StatsAggregator stats;
        EXPECT_EQ(0, stats.count);
        auto foo_listener_existence_scope = streams_manager.new_scoped_foo_listener(stats);
        foo_listener_existence_scope->WaitUntilCurrent();
        EXPECT_EQ(1, stats.count);
        EXPECT_EQ("[0]:{42,'Yay!'}", stats.data);
    }
}

TEST(StreamManagerSmokeTest, ListenerRunsInSeparateThread) {
    TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(Impl, ::TailProduce::StreamManager<InMemoryTestStorage>);
    TAILPRODUCE_STREAM(test, SimpleEntry, uint32_t, uint32_t);
    TAILPRODUCE_PUBLISHER(test);
    TAILPRODUCE_STATIC_FRAMEWORK_END();

    InMemoryTestStorage storage;

    // A variable to hold the lambda is required in order to pass a reference to ProcessEntrySync().
    std::function<void(const SimpleEntry&)> lambda;

    Impl streams_manager(storage, StreamManagerParams().CreateStream("test", uint32_t(0), uint32_t(0)));

    SimpleEntry entry;

    typename Impl::test_publisher_type& publisher = streams_manager.test_publisher;

    std::ostringstream os;

    auto async_listener =
        streams_manager.new_scoped_test_listener(lambda = [&os, &publisher](const SimpleEntry& entry) {
            static int safely_avoid_infinite_loop = 0;
            if (entry.data == "ping") {
                ASSERT_EQ(0, safely_avoid_infinite_loop);
                ++safely_avoid_infinite_loop;
                publisher.Push(SimpleEntry(entry.ikey + 1000, "pong"));
            }
            os << entry.ikey << ' ' << entry.data << std::endl;
        });

    publisher.Push(SimpleEntry(1, "one"));
    async_listener->WaitUntilCurrent();
    EXPECT_EQ("1 one\n", os.str());

    publisher.Push(SimpleEntry(2, "ping"));
    async_listener->WaitUntilCurrent();
    EXPECT_EQ("1 one\n2 ping\n1002 pong\n", os.str());
}
