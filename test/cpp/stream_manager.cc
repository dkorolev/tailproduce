#include <type_traits>  // std::is_same<>.

#include <gtest/gtest.h>

#include "../../src/tailproduce.h"

#include "mocks/stream_manager.h"
#include "mocks/data_storage.h"

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

// Unit test for TailProduce static framework.
// TODO(dkorolev): Add more stream managers and data storages here.

template<typename T> class StreamManagerTest : public ::testing::Test {};
typedef ::testing::Types<MockStreamManager<MockDataStorage>> DataStorageImplementations;
TYPED_TEST_CASE(StreamManagerTest, DataStorageImplementations);

// Tests stream creation macros.
TYPED_TEST(StreamManagerTest, UserFriendlySyntaxCompiles) {
    TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(StreamManagerImpl, TypeParam);
    TAILPRODUCE_STREAM(test, SimpleIntegerEntry, SimpleIntegerOrderKey);
    TAILPRODUCE_STATIC_FRAMEWORK_END();

    StreamManagerImpl impl;
    ASSERT_EQ(1, impl.registry().streams.size());
    EXPECT_EQ("test", impl.registry().streams[0].name);
    EXPECT_EQ("SimpleIntegerEntry", impl.registry().streams[0].entry_type);
    EXPECT_EQ("SimpleIntegerOrderKey", impl.registry().streams[0].order_key_type);
    EXPECT_TRUE(impl.registry().streams[0].impl == static_cast<TailProduce::Stream*>(&impl.test));
    EXPECT_TRUE((std::is_same<SimpleIntegerEntry, typename StreamManagerImpl::STREAM_TYPE_test::ENTRY_TYPE>::value));
    EXPECT_TRUE((std::is_same<SimpleIntegerOrderKey, typename StreamManagerImpl::STREAM_TYPE_test::ORDER_KEY_TYPE>::value));
}

// This test explicitly lists what stream definition macros expand into.
// Used as a reference point, as well as to ensure the macros do what they have been intended to.
TYPED_TEST(StreamManagerTest, ExpandedMacroSyntaxCompiles) {
    class StreamManagerImpl : public TypeParam {
      private:
        ::TailProduce::StreamsRegistry registry_;

      public:
        const ::TailProduce::StreamsRegistry& registry() { return registry_; }
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
    EXPECT_TRUE((std::is_same<SimpleIntegerOrderKey, typename StreamManagerImpl::STREAM_TYPE_test::ORDER_KEY_TYPE>::value));
}
