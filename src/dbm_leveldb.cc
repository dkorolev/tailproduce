#include <cassert>
#include <exception>

#include "storage.h"
#include "dbm_leveldb.h"
#include "tpexceptions.h"

TailProduce::DbMLevelDb::DbMLevelDb(std::string const& dbname) {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::DB* db;
    leveldb::Status status = leveldb::DB::Open(options, dbname, &db);
    assert(status.ok());
    db_.reset(db);
};

TailProduce::Storage::VALUE_TYPE TailProduce::DbMLevelDb::GetRecord(::TailProduce::Storage::KEY_TYPE const& key) {
    std::string v_get;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), key, &v_get);
    if (s.IsNotFound()) {
        throw ::TailProduce::StorageNoDataException();
    } else {
        if (!s.ok()) throw std::domain_error(s.ToString());
        ::TailProduce::Storage::VALUE_TYPE v_ret(v_get.begin(), v_get.end());
        return v_ret;
    }
}

bool TailProduce::DbMLevelDb::HasRecord(::TailProduce::Storage::KEY_TYPE const& key) {
    std::string v_get;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), key, &v_get);
    return !s.IsNotFound();
}

void TailProduce::DbMLevelDb::PutRecord(::TailProduce::Storage::KEY_TYPE const& key,
                                        ::TailProduce::Storage::VALUE_TYPE const& value) {
    std::string v_put(value.begin(), value.end());
    leveldb::Status s = db_->Put(leveldb::WriteOptions(), key, v_put);
    if (!s.ok()) throw std::domain_error(s.ToString());
}

void TailProduce::DbMLevelDb::AdminPutRecord(::TailProduce::Storage::KEY_TYPE const& key,
                                             ::TailProduce::Storage::VALUE_TYPE const& value) {
    PutRecord(key, value);
}

void TailProduce::DbMLevelDb::UNUSED_DeleteRecord(::TailProduce::Storage::KEY_TYPE const& key) {
    leveldb::Status s = db_->Delete(leveldb::WriteOptions(), key);
    if (!s.ok()) throw std::domain_error(s.ToString());
}
