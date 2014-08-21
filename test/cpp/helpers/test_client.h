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

// The SimpleEntry keeps the key/value pairs to test on.
// For this test they are { uint32, string } pairs.
struct SimpleEntry : ::TailProduce::CerealJSONSerializable<SimpleEntry> {
    SimpleEntry() = default;
    SimpleEntry(uint32_t key, std::string const& data) : ikey(key), data(data) {
    }

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

#endif  // TAILPRODUCE_MOCKS_TEST_CLIENT_H
