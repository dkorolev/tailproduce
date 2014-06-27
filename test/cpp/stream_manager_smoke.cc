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

TEST(StreamManagerSmokeTest, SmokeTest) {
    TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(StreamManagerImpl, MockStreamManager<MockDataStorage>);
    TAILPRODUCE_STREAM(test, SimpleEntry, SimpleOrderKey);
    TAILPRODUCE_STATIC_FRAMEWORK_END();

    StreamManagerImpl streams_manager;
    SimpleEntry entry;

    typename StreamManagerImpl::test_type::unsafe_listener_type listener_all(streams_manager.test);
    typename StreamManagerImpl::test_type::unsafe_listener_type listener_from_three(
        streams_manager.test,
        SimpleOrderKey(3));
    typename StreamManagerImpl::test_type::unsafe_listener_type listener_from_three_to_five_not_inclusive(
        streams_manager.test,
        SimpleOrderKey(3),
        SimpleOrderKey(5));

    ASSERT_TRUE(!listener_all.HasData());
    ASSERT_TRUE(!listener_from_three.HasData());
    ASSERT_TRUE(!listener_from_three_to_five_not_inclusive.HasData());
    ASSERT_TRUE(!listener_all.ReachedEnd());
    ASSERT_TRUE(!listener_from_three.ReachedEnd());
    ASSERT_TRUE(!listener_from_three_to_five_not_inclusive.ReachedEnd());

    typename StreamManagerImpl::test_type::unsafe_publisher_type publisher(streams_manager.test);

    publisher.Push(SimpleEntry(1, "one"));
    publisher.Push(SimpleEntry(2, "two"));

 //   ASSERT_TRUE(listener_all.HasData());
    ASSERT_TRUE(!listener_from_three.HasData());
    ASSERT_TRUE(!listener_from_three_to_five_not_inclusive.HasData());
//    ASSERT_TRUE(!listener_all.ReachedEnd());
    ASSERT_TRUE(!listener_from_three.ReachedEnd());
    ASSERT_TRUE(!listener_from_three_to_five_not_inclusive.ReachedEnd());

    publisher.Push(SimpleEntry(3, "three"));
    publisher.Push(SimpleEntry(4, "four"));

//    ASSERT_TRUE(listener_all.HasData());
    ASSERT_TRUE(listener_from_three.HasData());
    ASSERT_TRUE(listener_from_three_to_five_not_inclusive.HasData());
//    ASSERT_TRUE(listener_all.ReachedEnd());
    ASSERT_TRUE(!listener_from_three.ReachedEnd());
    ASSERT_TRUE(!listener_from_three_to_five_not_inclusive.ReachedEnd());

    publisher.Push(SimpleEntry(5, "five"));
    publisher.Push(SimpleEntry(6, "six"));
    publisher.Push(SimpleEntry(7, "seven"));

/*
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
*/

    /*
    {
        // Iterator test: bounded, pre-initialized with data, involving secondary keys.
        StreamManagerImpl streams_manager;

        typename StreamManagerImpl::test_type::unsafe_publisher_type publisher(streams_manager.test);
        publisher.Push(SimpleEntry(42, "i0"));
        publisher.Push(SimpleEntry(42, "i1"));
        publisher.Push(SimpleEntry(42, "i2"));
        publisher.Push(SimpleEntry(42, "i3"));
        publisher.Push(SimpleEntry(42, "i4"));
        publisher.Push(SimpleEntry(42, "i5"));
        publisher.Push(SimpleEntry(42, "i6"));

        SimpleEntry entry;
        typename StreamManagerImpl::test_type::unsafe_listener_type listener(
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
        // Iterator test: appended on-the-fly, bounded.
        StreamManagerImpl streams_manager;

        SimpleEntry entry;
        typename StreamManagerImpl::test_type::unsafe_publisher_type publisher(streams_manager.test);
        typename StreamManagerImpl::test_type::unsafe_listener_type listener(streams_manager.test,
                                                                          SimpleOrderKey(10),
                                                                          SimpleOrderKey(20));

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
    */
}
