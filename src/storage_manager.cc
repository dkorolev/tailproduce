#include "db_module.h"
#include "storage_manager.h"

TailProduce::StorageManager::StorageManager(DbModule* db) {
    dbm_.reset(db);
}

std::string
TailProduce::StorageManager::lastWriteKey_(std::string const& streamId) {
    return std::string("LASTWRITEHEAD\0") + streamId;
}

std::string
TailProduce::StorageManager::lastWriteValue_(std::string const& streamId) {
    auto key = lastWriteKey_(streamId);
    std::string value;
    TailProduce::DbMStatus s = dbm_->GetRecord(key, value);
    if (!s.Ok()) {
        dbm_->PutRecord(key, "\0");
    }
    return value;
}

TailProduce::DbMStatus
TailProduce::StorageManager::Set(std::string const& streamId,
                                 std::string const& key, 
                                 std::string const& value, 
                                 bool allow_overwrite) {
    return PutRecord(streamId, key, value, allow_overwrite);
}

TailProduce::DbMStatus
TailProduce::StorageManager::SetAllowingOverwite(std::string const& streamId, 
                                                 std::string const& key, 
                                                 std::string const& value){
    return PutRecord(streamId, key, value, true);
}

TailProduce::DbMStatus
TailProduce::StorageManager::Get(std::string const& streamId, 
                                 std::string const& key, 
                                 std::string& v) {
    std::string value;
    TailProduce::DbMStatus s = dbm_->GetRecord(key, value);
    v = value;
    return s;
}

void
TailProduce::StorageManager::GetRange(std::string const& streamId,
                                      std::string const& startKey,
                                      std::string const& stopKey) {
    auto fnctn = std::bind(&Notifier::Notify, 
                           &notifier_, 
                           "GET", 
                           streamId, 
                           std::placeholders::_1);
    dbm_->GetRange(startKey, stopKey, fnctn);
}

TailProduce::DbMStatus
TailProduce::StorageManager::GetRecord(std::string const& streamId, std::string const& key) {
    std::string value;
    TailProduce::DbMStatus s = dbm_->GetRecord(key, value);
    if (s.Ok()) {
        notifier_.Notify("GET", streamId, value);
    }
    return s;
}

TailProduce::DbMStatus
TailProduce::StorageManager::PutRecord(std::string const& streamId, 
                                       std::string const& key, 
                                       std::string const& value,
                                       bool allow_overwrites) {
    if (allow_overwrites || key.compare(lastWriteValue_(streamId)) > 0) {
        dbm_->PutRecord(lastWriteKey_(streamId), key);
        TailProduce::DbMStatus s = dbm_->PutRecord(key, value);
        if (s.Ok()) {
            notifier_.Notify("PUT", streamId, value);
        }
        return s;
    }
}

TailProduce::DbMStatus
TailProduce::StorageManager::DeleteRecord(std::string const& streamId, 
                                          std::string const& key) {
    TailProduce::DbMStatus s = dbm_->DeleteRecord(key);
    if (s.Ok()) {
        notifier_.Notify("DELETE", streamId, key);
    }
    return s;
}

void 
TailProduce::StorageManager::GetListener(std::string const& streamId, 
                                         TailProduce::NotifierCallback cb) {
    eventListener_("GET", streamId, cb);
};

void 
TailProduce::StorageManager::PutListener(std::string const& streamId, 
                                         TailProduce::NotifierCallback cb) {
    eventListener_("PUT", streamId, cb);
};

void
TailProduce::StorageManager::DeleteListener(std::string const& streamId, 
                                            TailProduce::NotifierCallback cb) {
    eventListener_("DELETE", streamId, cb);
};
