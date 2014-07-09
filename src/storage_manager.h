#ifndef _STORAGE_MANAGER_H
#define _STORAGE_MANAGER_H

#include <utility>
#include "storage.h"
#include "dbm_iterator.h"

namespace TailProduce {
    template <typename dbmodule>
    class StorageManager {
    private:
        dbmodule &dbm_;
    public:
        StorageManager(dbmodule& dbm) : dbm_(dbm) {}

        auto 
        GetIterator(::TailProduce::Storage::KEY_TYPE const& keyPrefix,
                    ::TailProduce::Storage::KEY_TYPE const& startKey = ::TailProduce::Storage::KEY_TYPE(),
                    ::TailProduce::Storage::KEY_TYPE const& endKey = ::TailProduce::Storage::KEY_TYPE()) -> 
            decltype(std::declval<dbmodule>().GetIterator(startKey,endKey)) 
        {
            return dbm_.GetIterator(keyPrefix, startKey, endKey);
        }

        void Set(::TailProduce::Storage::KEY_TYPE const& key, ::TailProduce::Storage::VALUE_TYPE const& value) {
            dbm_.PutRecord(key, value);
        }

        ::TailProduce::Storage::VALUE_TYPE
        Get(::TailProduce::Storage::KEY_TYPE const& key) {
            return dbm_.GetRecord(key);
        }
    };
};

#endif

