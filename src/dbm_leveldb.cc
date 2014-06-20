#include <cassert>
#include "dbm_leveldb.h"

namespace DbMLevelDbStatus {
    TailProduce::DbMStatus
    StatusCreator(leveldb::Status const& ldbstatus) {
        using CODE = TailProduce::DbMStatus;
        if (ldbstatus.ok()) 
            return TailProduce::DbMStatus(CODE::OK, "");
        else {
            int code = CODE::UnknownError;
            if (ldbstatus.IsNotFound()) code = CODE::NotFound;
            if (ldbstatus.IsCorruption()) code = CODE::Corruption;
            if (ldbstatus.IsIOError()) code = CODE::IOError;
            if (ldbstatus.IsNotFound()) code = CODE::NotFound;
            return TailProduce::DbMStatus(code, ldbstatus.ToString());
        }
    }
};

TailProduce::DbMLevelDb::DbMLevelDb(std::string const& dbname) : dbname_(dbname) {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::DB *db;
    leveldb::Status status = leveldb::DB::Open(options, dbname_, &db);
    assert(status.ok());
    db_.reset(db);
};

TailProduce::DbMStatus 
TailProduce::DbMLevelDb::GetRecord(Key_Type const& key, Value_Type& value) {
    std::string v2;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), key, &v2);
    value = v2;
    return DbMLevelDbStatus::StatusCreator(s);
}

TailProduce::DbMStatus 
TailProduce::DbMLevelDb::PutRecord(Key_Type const& key, Value_Type const& value) {
    leveldb::Status s = db_->Put(leveldb::WriteOptions(), key, value);
    return DbMLevelDbStatus::StatusCreator(s);
}

TailProduce::DbMStatus 
TailProduce::DbMLevelDb::DeleteRecord(Key_Type const& key) {
    leveldb::Status s = db_->Delete(leveldb::WriteOptions(), key);
    return DbMLevelDbStatus::StatusCreator(s);
}

