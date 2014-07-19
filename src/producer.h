#ifndef PRODUCERS_H
#define PRODUCERS_H

#include <string>
#include <stdexcept>
#include <glog/logging.h>

#include "stream.h"
#include "dbm_leveldb.h"
#include "storage_manager.h"
#include "helpers.h"

namespace TailProduce {

    // The STREAM object must be of Stream<GenericOrderKey<RangedData<XXX>>,
    //   RangedData<XXX>>>, or a type that mimics GenericOrderKey<RangedData<>>
    template <typename STORAGE_MANAGER, typename STREAM> struct Producer {
        Producer(STREAM& strm, STORAGE_MANAGER& sm, ConfigValues const& cv)
            : stream_(strm), storageManager_(sm), config_(cv) {
            // Read the last write key, or create a minimum key if the read fails.
            try {
                Storage::KEY_TYPE lastWriteKey = config_.GetStreamsLastKey(stream_.GetId());
                lastKey_ = antibytes(sm.Get(lastWriteKey));
            } catch (...) {
                // Did not find a last key.  Get the Minimum key from the order key.
                lastKey_ = stream_.GetOrderKey().GetMinKey();
            }
        }

        void Push(Storage::KEY_TYPE key, Storage::VALUE_TYPE const& value) {
            // if the incoming key is lexographically less then the previous write
            // key then die a miserable death
            if (key < lastKey_) {
                // Order keys should only be increasing.
                VLOG(3) << "throwing \"Keys must be lexographically increasing\"";
                throw std::logic_error("Keys must be lexographically increasing");
            } else if (key == lastKey_) {  // increment secondary if they are equal
                auto first = stream_.GetOrderKey().ReversePrimary(key);
                auto second = stream_.GetOrderKey().ReverseSecondary(lastKey_) + 1;
                key = stream_.GetOrderKey().CreateKey(first, second);
            }

            lastKey_ = key;
            storageManager_.AdminSet(config_.GetStreamsLastKey(stream_.GetId()), bytes(lastKey_));
            storageManager_.Set(key, value);
        }

        void Push(Storage::VALUE_TYPE const& value) {
            // if primary component of key is same as primary component of the
            // last key then recreate the key with an incremented secondary component

            auto key = stream_.GetOrderKey().CreateKey();
            auto first = stream_.GetOrderKey().ReversePrimary(key);
            if (first == stream_.GetOrderKey().ReversePrimary(lastKey_)) {
                auto second = stream_.GetOrderKey().ReverseSecondary(lastKey_) + 1;
                key = stream_.GetOrderKey().CreateKey(first, second);
            }
            lastKey_ = key;
            storageManager_.AdminSet(config_.GetStreamsLastKey(stream_.GetId()), bytes(lastKey_));
            storageManager_.Set(key, value);
        }

      private:
        STREAM& stream_;
        STORAGE_MANAGER& storageManager_;
        const ConfigValues& config_;
        Storage::KEY_TYPE lastKey_;
    };
};

#endif
