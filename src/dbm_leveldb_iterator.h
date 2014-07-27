#ifndef _DBM_LEVELDB_ITERATOR_H
#define _DBM_LEVELDB_ITERATOR_H

#include <string>
#include <memory>

#include "leveldb/db.h"

#include "storage.h"

namespace TailProduce {
    class DbMLevelDbIterator {
      public:
        DbMLevelDbIterator(leveldb::DB& db,
                           ::TailProduce::Storage::KEY_TYPE const& startKey,
                           ::TailProduce::Storage::KEY_TYPE const& endKey);
        void Next();
        ::TailProduce::Storage::KEY_TYPE Key() const;
        ::TailProduce::Storage::VALUE_TYPE Value() const;
        bool HasData() const;
        bool Done() const {
            return !HasData();
        }

      private:
        ::TailProduce::Storage::KEY_TYPE endKey_;
        // `db_` is owned by the creator of the iterator. The iterator is invalidated if DB gets deleted.
        leveldb::DB& db_;
        std::unique_ptr<leveldb::Iterator> it_;

        DbMLevelDbIterator() = delete;
        DbMLevelDbIterator(DbMLevelDbIterator const&) = delete;
        DbMLevelDbIterator& operator=(DbMLevelDbIterator const&) = delete;
    };
};

#endif
