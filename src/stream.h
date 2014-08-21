#ifndef STREAM_H
#define STREAM_H

#include <string>
#include <memory>
#include <mutex>

#include <glog/logging.h>

#include "config_values.h"
#include "magic_order_key.h"
#include "tp_exceptions.h"

namespace TailProduce {
    // The stream base is only a syntatic convenience.
    struct StreamBase {
        ConfigValues const& cv;
        explicit StreamBase(ConfigValues const& cv) : cv(cv), lock_mutex_(new std::mutex()) {
        }

        std::mutex& lock_mutex() const {
            return *lock_mutex_;
        }

      private:
        // Stream name is part of the traits class now.
        // std::string streamName_;
        mutable std::unique_ptr<std::mutex> lock_mutex_;
    };

    template <typename TRAITS, typename ENTRY, typename ORDER_KEY> struct Stream : StreamBase, TRAITS {
        typedef TRAITS T_TRAITS;
        typedef ENTRY T_ENTRY;
        typedef ORDER_KEY T_ORDER_KEY;

        Stream(TailProduce::ConfigValues& cv, const typename T_TRAITS::T_STORAGE& storage)
            : StreamBase(cv), TRAITS(cv) {

            // TODO(dkorolev): Check with Brian that removing `Entry` is a good idea.
            // using TE = ::TailProduce::Entry;
            // static_assert(std::is_base_of<TE, T_ENTRY>::value, "Stream::T_ENTRY should be derived from Entry.");

            try {
                ::TailProduce::Storage::STORAGE_KEY_TYPE storage_key = cv.HeadStorageKey(*this);
                ::TailProduce::Storage::STORAGE_VALUE_TYPE storage_value = storage.Get(storage_key);
                VLOG(2) << "Stream::Stream('" << T_TRAITS::name << "'): '" << storage_key << "' == '"
                        << antibytes(storage_value) << "'.";
                head.DecomposeStorageKey(::TailProduce::Storage::ValueToKey(storage_value), *this, cv);
                VLOG(2) << "Stream::Stream('" << T_TRAITS::name << "'): Decomposes to { " << head.primary << ", "
                        << head.secondary << " }.";
            } catch (const ::TailProduce::StorageException&) {
                VLOG(3) << "throw StreamDoesNotExistException();";
                throw StreamDoesNotExistException();
            }
        }

        T_ORDER_KEY head;
    };
};

#endif
