#include <cassert>
#include "DbMLevelDb.h"

namespace DbMLevelDbStatus {
    TailProduce::DbMStatus
    StatusCreator(leveldb::Status const& ldbstatus) {
        int code = TailProduce::DbMStatus::OK;
        if (ldbstatus.ok()) return TailProduce::DbMStatus(code, "");
        if (ldbstatus.IsNotFound()) code = TailProduce::DbMStatus::NotFound;
        if (ldbstatus.IsCorruption()) code = TailProduce::DbMStatus::Corruption;
        if (ldbstatus.IsIOError()) code = TailProduce::DbMStatus::IOError;
        if (ldbstatus.IsNotFound()) code = TailProduce::DbMStatus::NotFound;
        return TailProduce::DbMStatus(code, ldbstatus.ToString());
    };
};

TailProduce::DbMLevelDb::DbMLevelDb(std::string const& dbname) : dbname_(dbname) {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::DB *db;
    leveldb::Status status = leveldb::DB::Open(options, dbname_, &db);
    assert(status.ok());
    db_.reset(db);
};

void
TailProduce::DbMLevelDb::getRange(std::string const& startKey, 
                                  std::string const& endKey,
                                  RangeCallback cb) {
    auto *it = db_->NewIterator(leveldb::ReadOptions());
    
    for (it->Seek(startKey);
         it->Valid() && it->key().ToString() < endKey;
         it->Next()) {
        if (cb) {
            cb(it->value().ToString());
        }
    }
}

TailProduce::DbMStatus 
TailProduce::DbMLevelDb::getRecord(std::string const& key, std::string& value) {
    std::string v2;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), key, &v2);
    value = v2;
    return DbMLevelDbStatus::StatusCreator(s);
}

TailProduce::DbMStatus 
TailProduce::DbMLevelDb::putRecord(std::string const& key, std::string const& value) {
    leveldb::Status s = db_->Put(leveldb::WriteOptions(), key, value);
    return DbMLevelDbStatus::StatusCreator(s);
}

TailProduce::DbMStatus 
TailProduce::DbMLevelDb::deleteRecord(std::string const& key) {
    leveldb::Status s = db_->Delete(leveldb::WriteOptions(), key);
    return DbMLevelDbStatus::StatusCreator(s);
}

