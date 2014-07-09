#ifndef STORAGEKEYBUILDER_H
#define STORAGEKEYBUILDER_H

namespace TailProduce {
    // StorageKeyBuilder implements the BuildStorageKey function to convert
    // { stream name, typed order key, secondary key } into ::TailProduce::Storage::KEY_TYPE-s.
    template<typename T> struct StorageKeyBuilder {
        typedef typename T::order_key_type order_key_type;
        typedef typename T::head_pair_type head_pair_type;
        explicit StorageKeyBuilder(const std::string& stream_name)
          : head_storage_key("s:" + stream_name),
            prefix("d:" + stream_name + ":"),
            end_stream_key("d:" + stream_name + ":\xff") {
            using TOK = ::TailProduce::OrderKey;
            static_assert(std::is_base_of<TOK, order_key_type>::value,
                          "StorageKeyBuilder: T::order_key_type should be derived from Stream.");
        }
        ::TailProduce::Storage::KEY_TYPE BuildStorageKey(const head_pair_type& key) const {
            ::TailProduce::Storage::KEY_TYPE storage_key = prefix;
            OrderKey::template StaticAppendAsStorageKey<order_key_type>(key.first, key.second, storage_key);
            return storage_key;
        }
        static head_pair_type ParseStorageKey(::TailProduce::Storage::KEY_TYPE const& storage_key) {
            // TODO(dkorolev): This secondary key implementation as fixed 10 bytes is not final.
            const size_t expected_size = order_key_type::size_in_bytes + 1 + 10;
            if (storage_key.size() != expected_size) {
                VLOG(2)
                    << "Malformed key: input length " << storage_key.size()
                    << ", expected length " << expected_size << ".";
                VLOG(3) << "throw MalformedStorageHeadException();";
                throw MalformedStorageHeadException();
            }
            head_pair_type key;
            key.first.DeSerializeOrderKey(storage_key);
            //const char* p = reinterpret_cast<const char*>(&storage_key[order_key_type::size_in_bytes + 1]);
            const char *p = storage_key.substr(storage_key.length() - 10).data();
            if (sscanf(p, "%u", &key.second) != 1) {
                VLOG(2)
                    << "Malformed key: cannot parse the secondary key '"
                    << std::string(p, reinterpret_cast<const char*>(&storage_key[0]) + storage_key.size()) << "'"
                    << " from '" << storage_key << ".'";
                VLOG(3) << "throw MalformedStorageHeadException();";
                throw MalformedStorageHeadException();
            }
            const ::TailProduce::Storage::KEY_TYPE golden = 
                OrderKey::template StaticSerializeAsStorageKey<typename T::order_key_type>(key.first,
                                                                                           key.second);
            
            if (storage_key != golden) {
                VLOG(2)
                    << "Malformed key: input '" << storage_key << "', "
                    << "re-generated: '" << golden << "'.";
                VLOG(3) << "throw MalformedStorageHeadException();";
                throw MalformedStorageHeadException();
            }
            return key;
        }
        StorageKeyBuilder() = delete;
        //StorageKeyBuilder(const StorageKeyBuilder&) = delete;  TODO(dkorolev): Uncomment this line.
        //StorageKeyBuilder(StorageKeyBuilder&&) = delete;  TODO(dkorolev): Uncomment this line.
        //void operator=(const StorageKeyBuilder&) = delete;  TODO(dkorolev): Uncomment this line.
        ::TailProduce::Storage::KEY_TYPE const head_storage_key;
        ::TailProduce::Storage::KEY_TYPE const prefix;
        ::TailProduce::Storage::KEY_TYPE const end_stream_key;
    };
};
#endif
