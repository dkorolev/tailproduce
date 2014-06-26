#include <cassert>
#include <exception>
#include "dbm_leveldb.h"

TailProduce::DbMLevelDb::DbMLevelDb(std::string const& dbname) : dbname_(dbname) {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::DB *db;
    leveldb::Status status = leveldb::DB::Open(options, dbname_, &db);
    assert(status.ok());
    db_.reset(db);
};

TailProduce::Value_Type
TailProduce::DbMLevelDb::GetRecord(Key_Type const& key) {
    std::string v_get;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), key, &v_get);
    if (!s.ok()) throw std::domain_error(s.ToString());
    Value_Type v_ret(v_get.begin(), v_get.end());
    return v_ret;
}

void
TailProduce::DbMLevelDb::PutRecord(Key_Type const& key, Value_Type const& value) {
    std::string v_put(value.begin(), value.end());
    leveldb::Status s = db_->Put(leveldb::WriteOptions(), key, v_put);
    if (!s.ok()) throw std::domain_error(s.ToString());
}

void
TailProduce::DbMLevelDb::DeleteRecord(Key_Type const& key) {
    leveldb::Status s = db_->Delete(leveldb::WriteOptions(), key);
    if (!s.ok()) throw std::domain_error(s.ToString());
}


