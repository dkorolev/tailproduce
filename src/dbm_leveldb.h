#ifndef _DBM_LEVELDB_H
#define _DBM_LEVELDB_H

#include <memory>
#include "leveldb/db.h"
#include "db_module.h"
#include "dbm_leveldb_iterator.h"
#include "dbm_iterator.h"

namespace TailProduce {
    class DbMLevelDb : public DbModule {
        class Iterator;
    public:
        DbMLevelDb(std::string const& dbname = "/tmp/tailproducedb");
        virtual void GetRange(std::string const& startKey,
                              std::string const& endKey,
                              RangeCallback cb);
        virtual DbMStatus GetRecord(std::string const& key, std::string& value);
        virtual DbMStatus PutRecord(std::string const& key, std::string const& value);
        virtual DbMStatus DeleteRecord(std::string const& key);

        DbMIterator<std::shared_ptr<DbMLevelDbIterator>>
        GetIterator(std::string const& startKey, std::string const &endKey) {
            std::shared_ptr<DbMLevelDbIterator> it(new DbMLevelDbIterator(db_, startKey, endKey));
            return DbMIterator<std::shared_ptr<DbMLevelDbIterator>>(it);
        }

    private:
        std::shared_ptr<leveldb::DB> db_;
        std::string dbname_;
    };
};
#endif
