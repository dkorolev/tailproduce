#ifndef STREAMINSTANCE_H
#define STREAMINSTANCE_H

#include <string>
#include <type_traits>

#include "entry.h"
#include "order_key.h"
#include "storage.h"
#include "stream.h"
#include "config_values.h"

namespace TailProduce {
    template <typename ENTRY, typename ORDER_KEY> class StreamInstance : public Stream<ORDER_KEY> {
      public:
        typedef ENTRY T_ENTRY;
        typedef ORDER_KEY T_ORDER_KEY;
        StreamInstance(TailProduce::ConfigValues& cv,
                       const std::string& stream_name,
                       const std::string& entry_type_name,
                       const std::string& order_key_type_name)
            : Stream<ORDER_KEY>(cv, stream_name, entry_type_name, order_key_type_name) {
            using TE = ::TailProduce::Entry;
            static_assert(std::is_base_of<TE, ENTRY>::value,
                          "StreamInstance::ENTRY should be derived from Entry.");
            using TOK = ::TailProduce::OrderKey;
            static_assert(std::is_base_of<TOK, ORDER_KEY>::value,
                          "StreamInstance::ORDER_KEY should be derived from OrderKey.");
            static_assert(ORDER_KEY::size_in_bytes > 0,
                          "StreamInstance::ORDER_KEY::size_in_bytes should be positive.");
        }
    };
};

#endif
