#ifndef STREAMMANAGER_H
#define STREAMMANAGER_H

#include <glog/logging.h>

#include "storage.h"
#include "tp_exceptions.h"
#include "bytes.h"

namespace TailProduce {

    // StreamManager provides mid-level access to data in the streams.
    // It abstracts out:
    // 1) Low-level storage:
    //      Instead of raw string keys and raw byte array values,
    //      instances of StreamManager-s operates on serialized objects.
    // 2) Order keys management:
    //      StreamManager respects order keys, as well as their serialization to the storage.
    // 3) Producers and Listeners:
    //      Instead of storage-level Iterators that may hit the end and have to be re-created,
    //      StreamManager works on the scale of append-only Producers and stream-only Listeners.
    struct StreamManagerBase {
        template <typename ORDER_KEY, typename STORAGE_KEY_BUILDER, typename STORAGE>
        static std::pair<ORDER_KEY, uint32_t> FetchHeadOrDie(const std::string& name,
                                                               const STORAGE_KEY_BUILDER& key_builder,
                                                               STORAGE& storage) {
            ::TailProduce::Storage::VALUE_TYPE storage_value;
            try {
                // TODO(dkorolev): Ask Brian whether ParseStorageKey() should accept both strings and byte arrays?
                return STORAGE_KEY_BUILDER::ParseStorageKey(antibytes(storage.Get(key_builder.head_storage_key)));
            } catch (const StorageException&) {
                VLOG(3) << "throw StreamDoesNotExistException();";
                throw StreamDoesNotExistException();
            }
        }
    };

    template <typename STORAGE_MANAGER> struct StreamManager : StreamManagerBase {
        typedef STORAGE_MANAGER storage_type;
        STORAGE_MANAGER storage;
    };
};

#endif
