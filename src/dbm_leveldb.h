#ifndef _DBM_LEVELDB_H
#define _DBM_LEVELDB_H

#include <memory>
#include "leveldb/db.h"
#include "storage.h"
#include "dbm_leveldb_iterator.h"
#include "dbm_iterator.h"

namespace TailProduce {
    class DbMLevelDb {
      public:
        // TODO(dkorolev): This can certainly be simplified. Chat with Brian.
        typedef std::unique_ptr<DbMLevelDbIterator> StorageIteratorInnerType;
        typedef DbMIterator<StorageIteratorInnerType> StorageIterator;

        DbMLevelDb(std::string const& dbname = "/tmp/tailproducedb");
        ::TailProduce::Storage::VALUE_TYPE GetRecord(::TailProduce::Storage::KEY_TYPE const& key);
        void AdminPutRecord(::TailProduce::Storage::KEY_TYPE const& key,
                            ::TailProduce::Storage::VALUE_TYPE const& value);
        void PutRecord(::TailProduce::Storage::KEY_TYPE const& key, ::TailProduce::Storage::VALUE_TYPE const& value);
        bool HasRecord(::TailProduce::Storage::KEY_TYPE const& key);

        // TODO(dkorolev): Add a generic test for DeleteRecord(). So far, removed it.
        void UNUSED_DeleteRecord(::TailProduce::Storage::KEY_TYPE const& key);

        StorageIterator CreateStorageIterator(
            ::TailProduce::Storage::KEY_TYPE const& startKey = ::TailProduce::Storage::KEY_TYPE(),
            ::TailProduce::Storage::KEY_TYPE const& endKey = ::TailProduce::Storage::KEY_TYPE()) {
            return StorageIterator(StorageIteratorInnerType(new DbMLevelDbIterator(*db_.get(), startKey, endKey)));
        }

      private:
        std::unique_ptr<leveldb::DB> db_;
    };
};

#endif
