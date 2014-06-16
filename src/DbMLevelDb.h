#ifndef _DBM_LEVELDB_H
#define _DBM_LEVELDB_H

#include <memory>
#include "leveldb/db.h"
#include "DbModule.h"

namespace TailProduce {
    class DbMLevelDb : public DbModule {
    public:
        DbMLevelDb(std::string const& dbname = "/tmp/tailproducedb");
        virtual void getRange(std::string const& startKey,
                              std::string const& endKey,
                              RangeCallback cb);
        virtual DbMStatus getRecord(std::string const& key, std::string& value);
        virtual DbMStatus putRecord(std::string const& key, std::string const& value);
        virtual DbMStatus deleteRecord(std::string const& key);
    private:
        std::unique_ptr<leveldb::DB> db_;
        std::string dbname_;
    };
};
#endif
