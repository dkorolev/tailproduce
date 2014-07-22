#ifndef _DBM_LEVELDB_H
#define _DBM_LEVELDB_H

#include <memory>
#include "leveldb/db.h"
#include "storage.h"
#include "dbm_leveldb_iterator.h"
#include "dbm_iterator.h"

namespace TailProduce {
    class DbMLevelDb {
        class Iterator;

      public:
        DbMLevelDb(std::string const& dbname = "/tmp/tailproducedb");
        ::TailProduce::Storage::VALUE_TYPE GetRecord(::TailProduce::Storage::KEY_TYPE const& key);
        void AdminPutRecord(::TailProduce::Storage::KEY_TYPE const& key,
                            ::TailProduce::Storage::VALUE_TYPE const& value);
        void PutRecord(::TailProduce::Storage::KEY_TYPE const& key, ::TailProduce::Storage::VALUE_TYPE const& value);
        void DeleteRecord(::TailProduce::Storage::KEY_TYPE const& key);

        DbMIterator<std::shared_ptr<DbMLevelDbIterator>> GetIterator(
            ::TailProduce::Storage::KEY_TYPE const& keyPrefix,
            ::TailProduce::Storage::KEY_TYPE const& startKey = ::TailProduce::Storage::KEY_TYPE(),
            ::TailProduce::Storage::KEY_TYPE const& endKey = ::TailProduce::Storage::KEY_TYPE()) {
            std::shared_ptr<DbMLevelDbIterator> it(new DbMLevelDbIterator(db_, keyPrefix, startKey, endKey));
            return DbMIterator<std::shared_ptr<DbMLevelDbIterator>>(it);
        }

      private:
        std::shared_ptr<leveldb::DB> db_;
        std::string dbname_;
    };
};

#endif
