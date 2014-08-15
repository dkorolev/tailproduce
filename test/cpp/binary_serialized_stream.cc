#include <cstring>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "../../src/tailproduce.h"

#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/polymorphic.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

#include "helpers/test_client.h"
#include "helpers/storages.h"

using ::TailProduce::bytes;
using ::TailProduce::antibytes;
using ::TailProduce::StreamManagerParams;

struct SimpleBinaryEntry : ::TailProduce::Entry, ::TailProduce::CerealBinarySerializable<SimpleBinaryEntry> {
    SimpleBinaryEntry() = default;
    SimpleBinaryEntry(uint32_t k, std::string const& v) : k(k), v(v) {
    }

    SimpleOrderKey ExtractSimpleOrderKey() const {
        return SimpleOrderKey(k);
    }

    uint32_t k;
    std::string v;

  private:
    friend class cereal::access;
    template <class A> void serialize(A& ar) {
        ar(CEREAL_NVP(k), CEREAL_NVP(v));
    }
};

/*
namespace TailProduce {
    template <> struct OrderKeyExtractorImpl<SimpleOrderKey, SimpleBinaryEntry> {
        static SimpleOrderKey ExtractOrderKey(const SimpleBinaryEntry& entry) {
            return entry.ExtractSimpleOrderKey();
        }
    };
};
*/

template <typename STREAM_MANAGER_TYPE> struct Setup {
    TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(StreamManagerWithASingleBinaryStream, STREAM_MANAGER_TYPE);
    TAILPRODUCE_STREAM(foo, SimpleBinaryEntry, SimpleOrderKey);
    TAILPRODUCE_PUBLISHER(foo);
    TAILPRODUCE_STATIC_FRAMEWORK_END();

    typedef typename STREAM_MANAGER_TYPE::T_STORAGE Storage;
};

template <typename STREAM_MANAGER_TYPE> class BinaryStreamSerializationTest : public ::testing::Test {};
TYPED_TEST_CASE(BinaryStreamSerializationTest, TestStreamManagerImplementationsTypeList);

TYPED_TEST(BinaryStreamSerializationTest, InitializesStream) {
    typename Setup<TypeParam>::Storage storage;
    ASSERT_FALSE(storage.Has("s:foo"));
    typename Setup<TypeParam>::StreamManagerWithASingleBinaryStream streams_manager(
        storage, StreamManagerParams().CreateStream("foo", SimpleOrderKey(0)));
    ASSERT_TRUE(storage.Has("s:foo"));
    ASSERT_EQ("0000000000:0000000000", antibytes(storage.Get("s:foo")));
}

template <typename T> void PublishTestEntries(T& publisher) {
    publisher.Push(SimpleBinaryEntry(42, "Spock"));
}

TYPED_TEST(BinaryStreamSerializationTest, SerializesEntriesWithTypes) {
    typename Setup<TypeParam>::Storage storage;
    typename Setup<TypeParam>::StreamManagerWithASingleBinaryStream streams_manager(
        storage, StreamManagerParams().CreateStream("foo", SimpleOrderKey(0)));
    PublishTestEntries(streams_manager.foo_publisher);
    const char golden[] = "*\0\0\0\x5\0\0\0\0\0\0\0Spock";
    ASSERT_EQ(std::string(golden, golden + sizeof(golden) - 1),
              antibytes(storage.Get("d:foo:0000000042:0000000000")));
}

TYPED_TEST(BinaryStreamSerializationTest, DeSerializesEntriesWithTypes) {
    typename Setup<TypeParam>::Storage storage;
    typename Setup<TypeParam>::StreamManagerWithASingleBinaryStream streams_manager(
        storage, StreamManagerParams().CreateStream("foo", SimpleOrderKey(0)));
    struct Client {
        std::ostringstream os;
        void operator()(const SimpleBinaryEntry& entry) {
            os << "SimpleBinaryEntry(" << entry.k << ',' << entry.v << ")\n";
        }
    };
    Client client;
    auto foo_listener_existence_scope = streams_manager.new_scoped_foo_listener(client);
    ASSERT_EQ("", client.os.str());
    PublishTestEntries(streams_manager.foo_publisher);
    foo_listener_existence_scope->WaitUntilCurrent();
    EXPECT_EQ("SimpleBinaryEntry(42,Spock)\n", client.os.str());
}
