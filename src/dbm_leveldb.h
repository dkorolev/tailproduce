#ifndef _DBM_LEVELDB_H
#define _DBM_LEVELDB_H

#include <memory>
#include "leveldb/db.h"
#include "db_module.h"
#include "dbm_leveldb_iterator.h"
#include "dbm_iterator.h"

namespace TailProduce {
    class DbMLevelDb {
        class Iterator;
    public:
        DbMLevelDb(std::string const& dbname = "/tmp/tailproducedb");
        virtual DbMStatus GetRecord(Key_Type const& key, Value_Type& value);
        virtual DbMStatus PutRecord(Key_Type const& key, Value_Type const& value);
        virtual DbMStatus DeleteRecord(Key_Type const& key);

        DbMIterator<std::shared_ptr<DbMLevelDbIterator>>
        GetIterator(Key_Type const& startKey, Key_Type const &endKey) {
            std::shared_ptr<DbMLevelDbIterator> it(new DbMLevelDbIterator(db_, startKey, endKey));
            return DbMIterator<std::shared_ptr<DbMLevelDbIterator>>(it);
        }

    private:
        std::shared_ptr<leveldb::DB> db_;
        std::string dbname_;
    };
};
#endif
