#ifndef _STORAGE_MANAGER_H
#define _STORAGE_MANAGER_H

#include <utility>
#include "storage.h"
#include "dbm_iterator.h"

namespace TailProduce {
    template <typename dbmodule>
    class StorageManager : public Storage {
    private:
        dbmodule &dbm_;
    public:
        StorageManager(dbmodule& dbm) : dbm_(dbm) {}

        auto 
        GetIterator(KEY_TYPE const& keyPrefix,
                    KEY_TYPE const& startKey = KEY_TYPE(),
                    KEY_TYPE const& endKey = KEY_TYPE()) -> 
            decltype(std::declval<dbmodule>().GetIterator(startKey,endKey)) 
        {
            return dbm_.GetIterator(keyPrefix, startKey, endKey);
        }

        void Set(::TailProduce::Storage::KEY_TYPE const& key, VALUE_TYPE const& value) {
            dbm_.PutRecord(key, value);
        }

        VALUE_TYPE
        Get(KEY_TYPE const& key) {
            return dbm_.GetRecord(key);
        }
    };
};

#endif

