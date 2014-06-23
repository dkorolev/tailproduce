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
        GetIterator(Key_Type const& startKey, Key_Type const& endKey) -> 
            decltype(std::declval<dbmodule>().GetIterator(startKey,endKey)) 
        {
            return dbm_.GetIterator(startKey, endKey);
        }

        DbMStatus Set(Key_Type const& key, Value_Type const& value) {
            DbMStatus s = dbm_.PutRecord(key, value);
            return s;
        }

        
        DbMStatus Get(Key_Type const& key, Value_Type& v_return) {
            Value_Type value;
            DbMStatus s = dbm_.GetRecord(key, value);
            v_return = value;
            return s;
        }
    };
};

#endif

