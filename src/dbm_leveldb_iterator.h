#ifndef _DBM_LEVELDB_ITERATOR_H
#define _DBM_LEVELDB_ITERATOR_H

#include <string>
#include <memory>
#include "leveldb/db.h"

namespace TailProduce {
    class DbMLevelDbIterator {
    public:
        DbMLevelDbIterator(std::shared_ptr<leveldb::DB> db, 
                           std::string const& startKey,
                           std::string const& endKey);
        void Next();
        std::string Key();
        std::string Value();
        bool Done();
    private:
        std::string lastKey_;
        std::string endKey_;
        std::unique_ptr<leveldb::Iterator> it_;
        std::shared_ptr<leveldb::DB> db_;
        DbMLevelDbIterator() = delete;
        DbMLevelDbIterator(DbMLevelDbIterator const&) = delete;
        DbMLevelDbIterator& operator=(DbMLevelDbIterator const&) = delete;
    };
};
#endif
