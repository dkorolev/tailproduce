#ifndef STORAGE_LEVELDB_H
#define STORAGE_LEVELDB_H

#include <memory>
#include <string>

#include <glog/logging.h>

#include "leveldb/db.h"

#include "storage.h"
#include "tp_exceptions.h"

namespace TailProduce {
    class StorageLevelDB : ::TailProduce::Storage::Impl<StorageLevelDB> {
      private:
        using STORAGE_KEY_TYPE = ::TailProduce::Storage::STORAGE_KEY_TYPE;
        using STORAGE_VALUE_TYPE = ::TailProduce::Storage::STORAGE_VALUE_TYPE;

      public:
        class StorageIteratorImpl {
          public:
            StorageIteratorImpl(leveldb::DB* p_db, STORAGE_KEY_TYPE const& startKey, STORAGE_KEY_TYPE const& endKey);
            StorageIteratorImpl(StorageIteratorImpl&&) = default;
            void Next();
            STORAGE_KEY_TYPE Key() const;
            STORAGE_VALUE_TYPE Value() const;
            bool HasData() const;
            bool Done() const {
                return !HasData();
            }

          private:
            // `p_db_` is owned by the creator of the iterator. The iterator is invalidated if DB gets deleted.
            leveldb::DB* p_db_;
            std::unique_ptr<leveldb::Iterator> it_;

            STORAGE_KEY_TYPE endKey_;

            StorageIteratorImpl() = delete;
            StorageIteratorImpl(StorageIteratorImpl const&) = delete;
            StorageIteratorImpl& operator=(StorageIteratorImpl const&) = delete;
        };

        StorageLevelDB(std::string const& dbname = "/tmp/tailproducedb");
        STORAGE_VALUE_TYPE Get(STORAGE_KEY_TYPE const& key) const;
        void InternalSet(STORAGE_KEY_TYPE const& key, STORAGE_VALUE_TYPE const& value, bool allow_overwrite);
        void Set(const STORAGE_KEY_TYPE& key, const STORAGE_VALUE_TYPE& value) {
            InternalSet(key, value, false);
        }
        void SetAllowingOverwrite(const STORAGE_KEY_TYPE& key, const STORAGE_VALUE_TYPE& value) {
            InternalSet(key, value, true);
        }
        bool Has(STORAGE_KEY_TYPE const& key) const;

        // TODO(dkorolev): If needed, add Delete() to the interface and add a test for it. So far, removed it.
        void UNUSED_Delete(STORAGE_KEY_TYPE const& key);

        typedef std::unique_ptr<StorageIteratorImpl> StorageIterator;
        StorageIterator CreateStorageIterator(STORAGE_KEY_TYPE const& startKey = STORAGE_KEY_TYPE(),
                                              STORAGE_KEY_TYPE const& endKey = STORAGE_KEY_TYPE()) {
            return StorageIterator(new StorageIteratorImpl(db_.get(), startKey, endKey));
        }

      private:
        std::unique_ptr<leveldb::DB> db_;
    };
};

#endif
