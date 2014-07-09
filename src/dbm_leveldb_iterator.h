#ifndef _DBM_LEVELDB_ITERATOR_H
#define _DBM_LEVELDB_ITERATOR_H

#include <string>
#include <memory>
#include "leveldb/db.h"
#include "storage.h"

namespace TailProduce {
    class DbMLevelDbIterator {
    public:
        DbMLevelDbIterator(std::shared_ptr<leveldb::DB> db, 
                           ::TailProduce::Storage::KEY_TYPE const& keyPrefix,
                           ::TailProduce::Storage::KEY_TYPE const& startKey,
                           ::TailProduce::Storage::KEY_TYPE const& endKey);
        void Next();
        ::TailProduce::Storage::KEY_TYPE Key() const;
        ::TailProduce::Storage::VALUE_TYPE Value() const;
        bool Done();
    private:
        bool firstRead_;
        ::TailProduce::Storage::KEY_TYPE keyPrefix_;
        ::TailProduce::Storage::KEY_TYPE lastKey_;
        ::TailProduce::Storage::KEY_TYPE endKey_;
        std::shared_ptr<leveldb::DB> db_;
        std::unique_ptr<leveldb::Iterator> it_;

        DbMLevelDbIterator() = delete;
        DbMLevelDbIterator(DbMLevelDbIterator const&) = delete;
        DbMLevelDbIterator& operator=(DbMLevelDbIterator const&) = delete;
        void CreateIterator_(::TailProduce::Storage::KEY_TYPE const& key, bool advance);
    };
};
#endif
