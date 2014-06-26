#ifndef _STORAGE_MANAGER_H
#define _STORAGE_MANAGER_H

#include <utility>
#include "db_module.h"
#include "dbm_iterator.h"

namespace TailProduce {
    template <typename dbmodule>
    class StorageManager {
    private:
        dbmodule &dbm_;
    public:
        StorageManager(dbmodule& dbm) : dbm_(dbm) {}

        auto 
        GetIterator(Key_Type const& keyPrefix,
                    Key_Type const& startKey = Key_Type(),
                    Key_Type const& endKey = Key_Type()) -> 
            decltype(std::declval<dbmodule>().GetIterator(startKey,endKey)) 
        {
            return dbm_.GetIterator(keyPrefix, startKey, endKey);
        }

        void Set(Key_Type const& key, Value_Type const& value) {
            dbm_.PutRecord(key, value);
        }

        Value_Type Get(Key_Type const& key) {
            return dbm_.GetRecord(key);
        }
    };
};

#endif

