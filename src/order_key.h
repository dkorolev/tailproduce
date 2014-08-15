#ifndef ORDERKEY_H
#define ORDERKEY_H

/*
#include <string>
#include <type_traits>
#include <sstream>
#include <iomanip>
#include "storage.h"

namespace TailProduce {
    // An interface to extract order keys in certain types. With fixed-size serialization.
    struct OrderKey {
        // Needs enum { size_in_bytes = 0 };  // TO GO AWAY -- D.K.
        // Needs bool operator<(const T& rhs) const;
        // Needs void SerializeOrderKey(uint8_t* ptr) const;
        // Needs void DeSerializeOrderKey(const uint8_t* ptr);
        template <typename ORDER_KEY>
        static void StaticAppendAsStorageKey(const ORDER_KEY& primary_key,
                                             const uint32_t secondary_key,
                                             ::TailProduce::Storage::STORAGE_KEY_TYPE& output) {
            using TOK = ::TailProduce::OrderKey;
            static_assert(std::is_base_of<TOK, ORDER_KEY>::value,
                          "OrderKey::StaticAppendAsStorageKey::ORDER_KEY should be derived from OrderKey.");
            static_assert(ORDER_KEY::size_in_bytes > 0,
                          "OrderKey::StaticAppendAsStorageKey::ORDER_KEY::size_in_bytes should be positive.");
            // TODO(dkorolev): This secondary key implementation as fixed 10 bytes is not final.
            ::TailProduce::Storage::STORAGE_KEY_TYPE pkey;
            primary_key.SerializeOrderKey(pkey);
            std::ostringstream os;
            os << pkey << ':' << std::setfill('0') << std::setw(10) << secondary_key;
            output.append(os.str());
        }

        template <typename ORDER_KEY>
        static ::TailProduce::Storage::STORAGE_KEY_TYPE StaticSerializeAsStorageKey(const ORDER_KEY& primary_key,
                                                                                    const uint32_t secondary_key) {
            ::TailProduce::Storage::STORAGE_KEY_TYPE output;
            StaticAppendAsStorageKey<ORDER_KEY>(primary_key, secondary_key, output);
            return output;
        }
    };
};
*/

#endif
