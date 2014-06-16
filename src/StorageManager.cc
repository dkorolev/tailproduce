#include <iostream>
#include "DbModule.h"
#include "StorageManager.h"

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
    TailProduce::DbMStatus s = dbm_->getRecord(key, value);
    if (!s.Ok()) {
        dbm_->putRecord(key, "\0");
    }
    return value;
}

bool
TailProduce::StorageManager::getRange(std::string const& streamId,
                                      std::string const& startKey,
                                      std::string const& stopKey) {
    auto fnctn = std::bind(&Notifier::notify, 
                           &notifier_, 
                           "GET", 
                           streamId, 
                           std::placeholders::_1);
    dbm_->getRange(startKey, stopKey, fnctn);
}

bool
TailProduce::StorageManager::getRecord(std::string const& streamId, std::string const& key) {
    std::string value;
    TailProduce::DbMStatus s = dbm_->getRecord(key, value);
    if (s.Ok()) {
        notifier_.notify("GET", streamId, value);
    }
    else 
        std::cerr << "StorageManager::getRecord() " 
                  << s.status_ << " "
                  << s.Description() << std::endl;
}

bool
TailProduce::StorageManager::putRecord(std::string const& streamId, 
                                       std::string const& key, 
                                       std::string const& value) {
    if (key.compare(lastWriteValue_(streamId)) > 0) {
        dbm_->putRecord(lastWriteKey_(streamId), key);
        TailProduce::DbMStatus s = dbm_->putRecord(key, value);
        if (s.Ok()) {
            notifier_.notify("PUT", streamId, value);
        }
        else 
            std::cerr << "StorageManager::putRecord() " 
                      << s.status_ << " " 
                      << s.Description() << std::endl;
    }
}

bool
TailProduce::StorageManager::deleteRecord(std::string const& streamId, 
                                          std::string const& key) {
    TailProduce::DbMStatus s = dbm_->deleteRecord(key);
    if (s.Ok()) {
        notifier_.notify("DELETE", streamId, key);
    }
    else 
        std::cerr << "StorageManager::deleteRecord() " 
                  << s.status_ << " " 
                  << s.Description() << std::endl;
}

void 
TailProduce::StorageManager::getListener(std::string const& streamId, 
                                         TailProduce::NotifierCallback cb) {
    eventListener_("GET", streamId, cb);
};

void 
TailProduce::StorageManager::putListener(std::string const& streamId, 
                                         TailProduce::NotifierCallback cb) {
    eventListener_("PUT", streamId, cb);
};

void
TailProduce::StorageManager::deleteListener(std::string const& streamId, 
                                            TailProduce::NotifierCallback cb) {
    eventListener_("DELETE", streamId, cb);
};
