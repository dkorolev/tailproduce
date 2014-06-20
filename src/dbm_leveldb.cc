#include <cassert>
#include "dbm_leveldb.h"

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
TailProduce::DbMLevelDb::GetRange(std::string const& startKey, 
                                  std::string const& endKey,
                                  RangeCallback cb) {
    std::unique_ptr<leveldb::Iterator> it(db_->NewIterator(leveldb::ReadOptions()));
    std::string lEndKey(endKey);
    
    for (it->Seek(startKey);
         it->Valid() && it->key().ToString() < lEndKey;
         it->Next()) {
        if (cb) {
            cb(it->value().ToString());
        }
    }
}

TailProduce::DbMStatus 
TailProduce::DbMLevelDb::GetRecord(std::string const& key, std::string& value) {
    std::string v2;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), key, &v2);
    value = v2;
    return DbMLevelDbStatus::StatusCreator(s);
}

TailProduce::DbMStatus 
TailProduce::DbMLevelDb::PutRecord(std::string const& key, std::string const& value) {
    leveldb::Status s = db_->Put(leveldb::WriteOptions(), key, value);
    return DbMLevelDbStatus::StatusCreator(s);
}

TailProduce::DbMStatus 
TailProduce::DbMLevelDb::DeleteRecord(std::string const& key) {
    leveldb::Status s = db_->Delete(leveldb::WriteOptions(), key);
    return DbMLevelDbStatus::StatusCreator(s);
}

