#ifndef STORAGE_LEVELDB_H
#define STORAGE_LEVELDB_H

#include <memory>
#include <string>

#include <glog/logging.h>

#include "leveldb/db.h"

#include "storage.h"
#include "tp_exceptions.h"

using ::TailProduce::Storage::KEY_TYPE;
using ::TailProduce::Storage::VALUE_TYPE;

namespace TailProduce {
    class StorageLevelDB : ::TailProduce::Storage::Impl<StorageLevelDB> {
      public:
        class StorageIteratorImpl {
          public:
            StorageIteratorImpl(leveldb::DB* p_db, KEY_TYPE const& startKey, KEY_TYPE const& endKey);
            StorageIteratorImpl(StorageIteratorImpl&&) = default;
            void Next();
            KEY_TYPE Key() const;
            VALUE_TYPE Value() const;
            bool HasData() const;
            bool Done() const {
                return !HasData();
            }

          private:
            // `p_db_` is owned by the creator of the iterator. The iterator is invalidated if DB gets deleted.
            leveldb::DB* p_db_;
            std::unique_ptr<leveldb::Iterator> it_;

            KEY_TYPE endKey_;

            StorageIteratorImpl() = delete;
            StorageIteratorImpl(StorageIteratorImpl const&) = delete;
            StorageIteratorImpl& operator=(StorageIteratorImpl const&) = delete;
        };

        StorageLevelDB(std::string const& dbname = "/tmp/tailproducedb");
        VALUE_TYPE Get(KEY_TYPE const& key);
        void InternalSet(KEY_TYPE const& key, VALUE_TYPE const& value, bool allow_overwrite);
        void Set(const KEY_TYPE& key, const VALUE_TYPE& value) {
            InternalSet(key, value, false);
        }
        void SetAllowingOverwrite(const KEY_TYPE& key, const VALUE_TYPE& value) {
            InternalSet(key, value, true);
        }
        bool Has(KEY_TYPE const& key);

        // TODO(dkorolev): If needed, add Delete() to the interface and add a test for it. So far, removed it.
        void UNUSED_Delete(KEY_TYPE const& key);

        typedef std::unique_ptr<StorageIteratorImpl> StorageIterator;
        StorageIterator CreateStorageIterator(KEY_TYPE const& startKey = KEY_TYPE(),
                                              KEY_TYPE const& endKey = KEY_TYPE()) {
            return StorageIterator(new StorageIteratorImpl(db_.get(), startKey, endKey));
        }

      private:
        std::unique_ptr<leveldb::DB> db_;
    };
};

#endif
