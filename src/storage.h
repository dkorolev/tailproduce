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
        typedef std::string KEY_TYPE;
        typedef std::vector<uint8_t> VALUE_TYPE;
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
                storage.Set(KEY_TYPE("key"), VALUE_TYPE(bytes("value")));
                storage.SetAllowingOverwrite(KEY_TYPE("key"), VALUE_TYPE(bytes("value")));
                VALUE_TYPE v = storage.Get("key");
                bool b = storage.Has(KEY_TYPE("key"));
                // Iterator type and its creation.
                typename T::StorageIterator it1 = storage.CreateStorageIterator();
                typename T::StorageIterator it2 = storage.CreateStorageIterator(KEY_TYPE("a"));
                typename T::StorageIterator it3 = storage.CreateStorageIterator(KEY_TYPE("a"), KEY_TYPE("b"));
                // Key(), Value(), Next(), Done() for the iterator.
                bool b1 = it1->Done();
                it2->Next();
                KEY_TYPE k3 = it3->Key();
                VALUE_TYPE v4 = it2->Value();
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
