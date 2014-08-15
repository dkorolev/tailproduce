#ifndef STORAGEKEYBUILDER_H
#define STORAGEKEYBUILDER_H

/*
// TODO(dkorolev): Phase out this file.

#include <string>

#include <glog/logging.h>

#include "storage.h"
#include "entry.h"
#include "order_key.h"
#include "tp_exceptions.h"

namespace TailProduce {
    // StorageKeyBuilder implements the BuildStorageKey function to convert
    // { stream name, typed order key, secondary key } into ::TailProduce::Storage::STORAGE_KEY_TYPE-s.
    template <typename STREAM> struct StorageKeyBuilder {
        using STORAGE_KEY_TYPE = ::TailProduce::Storage::STORAGE_KEY_TYPE;
        using STORAGE_VALUE_TYPE = ::TailProduce::Storage::STORAGE_VALUE_TYPE;
        typedef STREAM T_STREAM;
        typedef typename T_STREAM::T_ORDER_KEY T_ORDER_KEY;
        explicit StorageKeyBuilder(const std::string& stream_name)
            : head_storage_key("s:" + stream_name),
              prefix("d:" + stream_name + ":"),
              end_stream_key("d:" + stream_name + ":\xff") {
            using TOK = ::TailProduce::OrderKey;
            static_assert(std::is_base_of<TOK, T_ORDER_KEY>::value,
                          "StorageKeyBuilder: T_STREAM::T_ORDER_KEY should be derived from OrderKey.");
        }
        STORAGE_KEY_TYPE BuildStorageKey(const T_ORDER_KEY& key) const {
            STORAGE_KEY_TYPE storage_key = prefix;
            OrderKey::template StaticAppendAsStorageKey<T_ORDER_KEY>(key.first, key.second, storage_key);
            return storage_key;
        }
        static T_ORDER_KEY ParseStorageKey(STORAGE_KEY_TYPE const& storage_key) {
            // TODO(dkorolev): This secondary key implementation as fixed 10 bytes is not final.
            const size_t expected_size = T_ORDER_KEY::size_in_bytes + 1 + 10;
            if (storage_key.size() != expected_size) {
                VLOG(2) << "Malformed key: input length " << storage_key.size() << ", expected length "
                        << expected_size << ".";
                VLOG(3) << "throw MalformedStorageHeadException();";
                throw MalformedStorageHeadException();
            }
            T_ORDER_KEY key;
            key.first.DeSerializeOrderKey(storage_key);
            // const char* p = reinterpret_cast<const char*>(&storage_key[T_ORDER_KEY::size_in_bytes + 1]);
            const char* p = storage_key.substr(storage_key.length() - 10).data();
            if (sscanf(p, "%u", &key.second) != 1) {
                VLOG(2) << "Malformed key: cannot parse the secondary key '"
                        << std::string(p, reinterpret_cast<const char*>(&storage_key[0]) + storage_key.size()) << "'"
                        << " from '" << storage_key << ".'";
                VLOG(3) << "throw MalformedStorageHeadException();";
                throw MalformedStorageHeadException();
            }
            const STORAGE_KEY_TYPE golden =
                OrderKey::template StaticSerializeAsStorageKey<T_ORDER_KEY>(key.first, key.second);

            if (storage_key != golden) {
                VLOG(2) << "Malformed key: input '" << storage_key << "', "
                        << "re-generated: '" << golden << "'.";
                VLOG(3) << "throw MalformedStorageHeadException();";
                throw MalformedStorageHeadException();
            }
            return key;
        }
        StorageKeyBuilder() = delete;
        // StorageKeyBuilder(const StorageKeyBuilder&) = delete;  TODO(dkorolev): Uncomment this line.
        // StorageKeyBuilder(StorageKeyBuilder&&) = delete;  TODO(dkorolev): Uncomment this line.
        // void operator=(const StorageKeyBuilder&) = delete;  TODO(dkorolev): Uncomment this line.
        STORAGE_KEY_TYPE const head_storage_key;
        STORAGE_KEY_TYPE const prefix;
        STORAGE_KEY_TYPE const end_stream_key;
    };
};
*/

#endif
