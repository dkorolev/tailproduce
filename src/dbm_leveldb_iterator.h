#ifndef _DBM_LEVELDB_ITERATOR_H
#define _DBM_LEVELDB_ITERATOR_H

#include <string>
#include <memory>
#include "leveldb/db.h"

#include "db_module.h"

namespace TailProduce {
    class DbMLevelDbIterator {
    public:
        DbMLevelDbIterator(std::shared_ptr<leveldb::DB> db, 
                           Key_Type const& keyPrefix,
                           Key_Type const& startKey,
                           Key_Type const& endKey);
        void Next();
        Key_Type Key() const;
        Value_Type Value() const;
        bool Done();
    private:
        bool firstRead_;
        Key_Type keyPrefix_;
        Key_Type lastKey_;
        Key_Type endKey_;
        std::shared_ptr<leveldb::DB> db_;
        std::unique_ptr<leveldb::Iterator> it_;

        DbMLevelDbIterator() = delete;
        DbMLevelDbIterator(DbMLevelDbIterator const&) = delete;
        DbMLevelDbIterator& operator=(DbMLevelDbIterator const&) = delete;
        void CreateIterator_(Key_Type const& key, bool advance);
    };
};
#endif
