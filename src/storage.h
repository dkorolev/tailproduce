#ifndef STORAGE_H
#define STORAGE_H

// Each user-defined storage should be defined as:
// class MyStorage : ::TailProduce::Storage::Impl<MyStorage> { ... };
//
// This ensures that the storage correctly exposes the interface,
// while keeping it statically typed in the meantime.

#include <string>
#include <vector>

#include "bytes.h"

namespace TailProduce {
    namespace Storage {
        typedef std::string STORAGE_KEY_TYPE;
        typedef std::vector<uint8_t> STORAGE_VALUE_TYPE;

        // TODO(dkorolev): Add tests for these two methods.
        inline STORAGE_KEY_TYPE ValueToKey(const STORAGE_VALUE_TYPE& value) {
            return antibytes(value);
        }

        inline STORAGE_VALUE_TYPE KeyToValue(const STORAGE_KEY_TYPE& key) {
            return bytes(key);
        }

        // TailProduce::Storage::Internal::Interface is used to static_assert the inheritance
        // at the moment of attemping to use the storage as part of the static framework.
        namespace Internal {
            struct Interface {};
        };
        // Impl<T> ensures that T implements Storage interface in full.
        template <typename T> struct Impl : Internal::Interface {
            Impl() {
                if (false) {
                    EnsureTIsStorage();
                }
            }
            static void EnsureTIsStorage() {
                T storage;
                // Set(), SetAllowingOverwrite(), Get(), Has().
                storage.Set(STORAGE_KEY_TYPE("key"), STORAGE_VALUE_TYPE(bytes("value")));
                storage.SetAllowingOverwrite(STORAGE_KEY_TYPE("key"), STORAGE_VALUE_TYPE(bytes("value")));
                STORAGE_VALUE_TYPE v = storage.Get("key");
                bool b = storage.Has(STORAGE_KEY_TYPE("key"));
                // Get() and Has() should be const.
                {
                    const T& const_storage = storage;
                    STORAGE_VALUE_TYPE v = const_storage.Get("key");
                    bool b = const_storage.Has(STORAGE_KEY_TYPE("key"));
                }
                // Iterator type and its creation.
                typename T::StorageIterator it1 = storage.CreateStorageIterator();
                typename T::StorageIterator it2 = storage.CreateStorageIterator(STORAGE_KEY_TYPE("a"));
                typename T::StorageIterator it3 =
                    storage.CreateStorageIterator(STORAGE_KEY_TYPE("a"), STORAGE_KEY_TYPE("b"));
                // Key(), Value(), Next(), Done() for the iterator.
                bool b1 = it1->Done();
                it2->Next();
                STORAGE_KEY_TYPE k3 = it3->Key();
                STORAGE_VALUE_TYPE v4 = it2->Value();
                // Iterator copy and assignment should support move semantics.
                it2 = std::move(it1);
                typename T::StorageIterator it4(std::move(it3));
                // Iterators are either unique_ptr<>-s or behave as them.
                auto bare_pointer = it4.get();
                it1.reset(bare_pointer);
                it2.reset(nullptr);
            }
        };
    }
};

#endif
