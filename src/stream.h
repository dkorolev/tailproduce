#ifndef STREAM_H
#define STREAM_H

#include <string>
#include <memory>
#include <mutex>

#include <glog/logging.h>

#include "config_values.h"
#include "magic_order_key.h"
#include "entry.h"
#include "tp_exceptions.h"

namespace TailProduce {
    // The stream base is only a syntatic convenience.
    struct StreamBase {
        ConfigValues const& cv;
        explicit StreamBase(ConfigValues const& cv) : cv(cv) {
        }

        /*
        StreamBase() = delete;
        StreamBase(const StreamBase&) = delete;
        void operator=(const StreamBase&) = delete;
        */

        /*
        StreamBase(std::string const& streamName) : streamName_(streamName), lock_mutex_(new std::mutex()) {
        }
        std::string const& GetId() {
            return streamName_;
        }
        */
        std::mutex& lock_mutex() const {
            return *lock_mutex_;
        }

      private:
        // std::string streamName_;
        mutable std::unique_ptr<std::mutex> lock_mutex_;
    };

    template <typename TRAITS, typename ENTRY, typename ORDER_KEY> struct Stream : StreamBase, TRAITS {
        typedef TRAITS T_TRAITS;
        typedef ENTRY T_ENTRY;
        typedef ORDER_KEY T_ORDER_KEY;

        Stream(TailProduce::ConfigValues& cv, const typename T_TRAITS::T_STORAGE& storage)
            : StreamBase(cv), TRAITS(cv) {
            //                       const std::string& stream_name,
            //                       const std::string& entry_type_name,
            //                       const std::string& order_key_type_name)
            //, stream_name, entry_type_name, order_key_type_name)
            using TO = ::TailProduce::MagicGenericOrderKey;
            static_assert(std::is_base_of<TO, T_ORDER_KEY>::value,
                          "Stream::T_ORDER_KEY should be derived from MagicGenericOrderKey.");
            using TE = ::TailProduce::Entry;
            static_assert(std::is_base_of<TE, T_ENTRY>::value, "Stream::T_ENTRY should be derived from Entry.");

            try {
                // return STORAGE_KEY_BUILDER::ParseStorageKey(antibytes(storage.Get(key_builder.head_storage_key)));
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
            /*
            using TOK = ::TailProduce::OrderKey;
            static_assert(std::is_base_of<TOK, T_ORDER_KEY>::value,
                          "Stream::T_ORDER_KEY should be derived from OrderKey.");
            static_assert(T_ORDER_KEY::size_in_bytes > 0,
                          "Stream::T_ORDER_KEY::size_in_bytes should be positive.");
            */
        }

        T_ORDER_KEY head;
    };
};

#endif
