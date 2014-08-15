#ifndef TAILPRODUCE_MOCKS_TEST_CLIENT_H
#define TAILPRODUCE_MOCKS_TEST_CLIENT_H

// Example TailProduce client code.
// For this test, it generates prime numbers and aggregates those.
// The code within thi file should not depend on framework, stream manager or data storage modules.

#include "../../../src/tailproduce.h"

#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"

#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/map.hpp"

#include "cereal/types/polymorphic.hpp"

/*
// Ordering key.
struct SimpleOrderKey : ::TailProduce::OrderKey {
    SimpleOrderKey() = default;
    SimpleOrderKey(std::string const& streamId, TailProduce::ConfigValues const& cv) : ikey(0) {
    }
    explicit SimpleOrderKey(uint32_t key) : ikey(key) {
    }
    uint32_t ikey;

    enum { size_in_bytes = 10 };

    bool operator<(const SimpleOrderKey& rhs) const {
        return ikey < rhs.ikey;
    }

    void SerializeOrderKey(::TailProduce::Storage::STORAGE_KEY_TYPE& ref) const {
        char tmp[11];
        snprintf(tmp, sizeof(tmp), "%010u", ikey);
        ref = tmp;
    }

    void DeSerializeOrderKey(::TailProduce::Storage::STORAGE_KEY_TYPE const& ref) {
        ikey = atoi(ref.c_str());
    }
};
*/

// The SimpleEntry keeps the key/value pairs to test on.
// For this test they are { uint32, string } pairs.
struct SimpleEntry : ::TailProduce::Entry, ::TailProduce::CerealJSONSerializable<SimpleEntry> {
    SimpleEntry() = default;
    SimpleEntry(uint32_t key, std::string const& data) : ikey(key), data(data) {
    }

    /*
    SimpleOrderKey ExtractSimpleOrderKey() const {
        return SimpleOrderKey(ikey);
    }
    */

    // Keep return type as paramter for easer overloading / templated usage.
    void GetOrderKey(uint32_t& output) const {
        output = ikey;
    }

    // Used as order key, 1, 2, 3, etc.
    uint32_t ikey;
    // To test data extraction, "one", "two", "three", or anything else for the sake of this test.
    std::string data;

  private:
    friend class cereal::access;
    template <class A> void serialize(A& ar) {
        ar(CEREAL_NVP(ikey), CEREAL_NVP(data));
    }
};

/*
namespace TailProduce {
    template <> struct OrderKeyExtractorImpl<SimpleOrderKey, SimpleEntry> {
        static SimpleOrderKey ExtractOrderKey(const SimpleEntry& entry) {
            return entry.ExtractSimpleOrderKey();
        }
    };
};
*/

#endif  // TAILPRODUCE_MOCKS_TEST_CLIENT_H
