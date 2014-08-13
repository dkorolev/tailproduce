#include <cstring>
#include <iomanip>
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

#include "helpers/storages.h"

using ::TailProduce::bytes;
using ::TailProduce::antibytes;
using ::TailProduce::StreamManagerParams;

struct IntOrderKey : ::TailProduce::OrderKey {
    uint32_t k;
    explicit IntOrderKey(uint32_t key = 0) : k(key) {
    }
    // TODO(dkorolev): Discuss this design with Brian. Perhaps it's better to keep OrderKey implemenations clean?
    IntOrderKey(std::string const& streamId, TailProduce::ConfigValues const& cv) : k(0) {
    }

    enum { size_in_bytes = 3 };

    bool operator<(const IntOrderKey& rhs) const {
        return k < rhs.k;
    }

    void SerializeOrderKey(::TailProduce::Storage::KEY_TYPE& ref) const {
        char tmp[size_in_bytes + 1];
        snprintf(tmp, sizeof(tmp), "%03u", k);
        ref = tmp;
    }

    void DeSerializeOrderKey(::TailProduce::Storage::KEY_TYPE const& ref) {
        k = atoi(ref.c_str());
    }
};

struct DerivedEntryA;
struct DerivedEntryB;
struct BaseEntry : ::TailProduce::Entry,
                   ::TailProduce::PolymorphicCerealJSONSerializable<BaseEntry, DerivedEntryA, DerivedEntryB> {
    BaseEntry() = default;
    virtual ~BaseEntry() {
    }
    BaseEntry(uint32_t key) : k(key) {
    }

    IntOrderKey ExtractSimpleOrderKey() const {
        return IntOrderKey(k);
    }

    uint32_t k;

  protected:
    friend class cereal::access;
    template <class A> void serialize(A& ar) {
        ar(CEREAL_NVP(k));
    }
};

struct DerivedEntryA : BaseEntry {
    char c;
    DerivedEntryA() = default;
    DerivedEntryA(uint32_t k, char c) : BaseEntry(k), c(c) {
    }

  private:
    friend class cereal::access;
    template <class A> void serialize(A& ar) {
        BaseEntry::serialize(ar);
        ar(CEREAL_NVP(c));
    }
};

struct DerivedEntryB : BaseEntry {
    std::string s;
    DerivedEntryB() = default;
    DerivedEntryB(uint32_t k, const std::string& s) : BaseEntry(k), s(s) {
    }

  private:
    friend class cereal::access;
    template <class A> void serialize(A& ar) {
        BaseEntry::serialize(ar);
        ar(CEREAL_NVP(s));
    }
};

CEREAL_REGISTER_TYPE(BaseEntry);
CEREAL_REGISTER_TYPE(DerivedEntryA);
CEREAL_REGISTER_TYPE(DerivedEntryB);

namespace TailProduce {
    template <> struct OrderKeyExtractorImpl<IntOrderKey, BaseEntry> {
        static IntOrderKey ExtractOrderKey(const BaseEntry& entry) {
            return entry.ExtractSimpleOrderKey();
        }
    };
};

template <typename STREAM_MANAGER_TYPE> struct Setup {
    TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(PolymorphicStreamsManager, STREAM_MANAGER_TYPE);
    TAILPRODUCE_STREAM(polymorphic_stream, BaseEntry, IntOrderKey);
    TAILPRODUCE_PUBLISHER(polymorphic_stream);
    TAILPRODUCE_STATIC_FRAMEWORK_END();

    typedef typename STREAM_MANAGER_TYPE::storage_type Storage;
};

template <typename STREAM_MANAGER_TYPE> class PolymorphicStreamTest : public ::testing::Test {};
TYPED_TEST_CASE(PolymorphicStreamTest, TestStreamManagerImplementationsTypeList);

TYPED_TEST(PolymorphicStreamTest, InitializesStream) {
    typename Setup<TypeParam>::Storage storage;
    ASSERT_FALSE(storage.Has("s:polymorphic_stream"));
    typename Setup<TypeParam>::PolymorphicStreamsManager streams_manager(
        storage, StreamManagerParams().CreateStream("polymorphic_stream", IntOrderKey(0)));
    ASSERT_TRUE(storage.Has("s:polymorphic_stream"));
    ASSERT_EQ("000:0000000000", antibytes(storage.Get("s:polymorphic_stream")));
}

template <typename T> void PublishTestEntries(T& publisher) {
    publisher.Push(BaseEntry(1));
    publisher.Push(DerivedEntryA(2, 'A'));
    publisher.Push(DerivedEntryB(3, "foo"));
}

TYPED_TEST(PolymorphicStreamTest, SerializesEntriesWithTypes) {
    typename Setup<TypeParam>::Storage storage;
    typename Setup<TypeParam>::PolymorphicStreamsManager streams_manager(
        storage, StreamManagerParams().CreateStream("polymorphic_stream", IntOrderKey(0)));
    PublishTestEntries(streams_manager.polymorphic_stream_publisher);
    ASSERT_EQ(
        "{\n"
        "    \"value0\": {\n"
        "        \"polymorphic_id\": 1073741824,\n"
        "        \"ptr_wrapper\": {\n"
        "            \"id\": 2147483649,\n"
        "            \"data\": {\n"
        "                \"k\": 1\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}\n",
        antibytes(storage.Get("d:polymorphic_stream:001:0000000000")));
    ASSERT_EQ(
        "{\n"
        "    \"value0\": {\n"
        "        \"polymorphic_id\": 2147483649,\n"
        "        \"polymorphic_name\": \"DerivedEntryA\",\n"
        "        \"ptr_wrapper\": {\n"
        "            \"id\": 2147483649,\n"
        "            \"data\": {\n"
        "                \"k\": 2,\n"
        "                \"c\": 65\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}\n",
        antibytes(storage.Get("d:polymorphic_stream:002:0000000000")));
    ASSERT_EQ(
        "{\n"
        "    \"value0\": {\n"
        "        \"polymorphic_id\": 2147483649,\n"
        "        \"polymorphic_name\": \"DerivedEntryB\",\n"
        "        \"ptr_wrapper\": {\n"
        "            \"id\": 2147483649,\n"
        "            \"data\": {\n"
        "                \"k\": 3,\n"
        "                \"s\": \"foo\"\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}\n",
        antibytes(storage.Get("d:polymorphic_stream:003:0000000000")));
}

TYPED_TEST(PolymorphicStreamTest, DeSerializesEntriesWithTypes) {
    typename Setup<TypeParam>::Storage storage;
    typename Setup<TypeParam>::PolymorphicStreamsManager streams_manager(
        storage, StreamManagerParams().CreateStream("polymorphic_stream", IntOrderKey(0)));
    struct Client {
        std::ostringstream os;
        void operator()(const BaseEntry& entry) {
            os << "BaseEntry(" << entry.k << ")\n";
        }
        void operator()(const DerivedEntryA& entry) {
            os << "DerivedEntryA(" << entry.k << ", '" << entry.c << "')\n";
        }
        void operator()(const DerivedEntryB& entry) {
            os << "DerivedEntryB(" << entry.k << ", ''" << entry.s << "'')\n";
        }
    };
    Client client;
    auto foo_listener_existence_scope = streams_manager.new_scoped_polymorphic_stream_listener(client);
    ASSERT_EQ("", client.os.str());
    PublishTestEntries(streams_manager.polymorphic_stream_publisher);
    foo_listener_existence_scope->WaitUntilCurrent();
    EXPECT_EQ("BaseEntry(1)\nDerivedEntryA(2, 'A')\nDerivedEntryB(3, ''foo'')\n", client.os.str());
}
