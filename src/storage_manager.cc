#include "db_module.h"
#include "storage_manager.h"

TailProduce::StorageManager::StorageManager(DbModule* db) {
    dbm_.reset(db);
}

TailProduce::DbMStatus
TailProduce::StorageManager::Set(Key_Type const& key, 
                                 Value_Type const& value, 
                                 bool allow_overwrite) {
    TailProduce::DbMStatus s = dbm_->PutRecord(key, value);
    return s;
}

TailProduce::DbMStatus
TailProduce::StorageManager::Get(Key_Type const& key, Value_Type& v_return) {
    std::string value;
    TailProduce::DbMStatus s = dbm_->GetRecord(key, value);
    v_return = value;
    return s;
}

