#ifndef STORAGE_LEVELDB_H
#define STORAGE_LEVELDB_H

#include <memory>
#include <string>

#include <glog/logging.h>

#include "leveldb/db.h"

#include "storage.h"
#include "tp_exceptions.h"

namespace TailProduce {
    class StorageLevelDB : ::TailProduce::Storage {
      public:
        class StorageIterator {
          public:
            StorageIterator(leveldb::DB* p_db,
                            ::TailProduce::Storage::KEY_TYPE const& startKey,
                            ::TailProduce::Storage::KEY_TYPE const& endKey);
            StorageIterator(StorageIterator&&) = default;
            void Next();
            ::TailProduce::Storage::KEY_TYPE Key() const;
            ::TailProduce::Storage::VALUE_TYPE Value() const;
            bool HasData() const;
            bool Done() const {
                return !HasData();
            }

          private:
            // `p_db_` is owned by the creator of the iterator. The iterator is invalidated if DB gets deleted.
            leveldb::DB* p_db_;
            std::unique_ptr<leveldb::Iterator> it_;

            void DoNext();
            ::TailProduce::Storage::KEY_TYPE endKey_;

            StorageIterator() = delete;
            StorageIterator(StorageIterator const&) = delete;
            StorageIterator& operator=(StorageIterator const&) = delete;
        };

        StorageLevelDB(std::string const& dbname = "/tmp/tailproducedb");
        ::TailProduce::Storage::VALUE_TYPE Get(::TailProduce::Storage::KEY_TYPE const& key);
        void InternalSet(::TailProduce::Storage::KEY_TYPE const& key,
                         ::TailProduce::Storage::VALUE_TYPE const& value,
                         bool allow_overwrite);
        void Set(const KEY_TYPE& key, const VALUE_TYPE& value) {
            InternalSet(key, value, false);
        }
        void SetAllowingOverwrite(const KEY_TYPE& key, const VALUE_TYPE& value) {
            InternalSet(key, value, true);
        }
        bool Has(::TailProduce::Storage::KEY_TYPE const& key);

        // TODO(dkorolev): Add a generic test for DeleteRecord(). So far, removed it.
        void UNUSED_Delete(::TailProduce::Storage::KEY_TYPE const& key);

        std::unique_ptr<StorageIterator> CreateNewStorageIterator(
            ::TailProduce::Storage::KEY_TYPE const& startKey = ::TailProduce::Storage::KEY_TYPE(),
            ::TailProduce::Storage::KEY_TYPE const& endKey = ::TailProduce::Storage::KEY_TYPE()) {
            return std::unique_ptr<StorageIterator>(new StorageIterator(db_.get(), startKey, endKey));
        }

      private:
        std::unique_ptr<leveldb::DB> db_;
    };
};

#endif
