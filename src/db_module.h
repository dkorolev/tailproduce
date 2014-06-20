#ifndef _DB_MODULE_H
#define _DB_MODULE_H

#include <string>
#include <functional>
#include "dbm_status.h"

namespace TailProduce {
    typedef std::function<void(std::string const &)> RangeCallback;

    class DbModule {
    public:
        virtual void GetRange(std::string const& startKey, 
                              std::string const& endKey,
                              RangeCallback cb) = 0;
        virtual DbMStatus GetRecord(std::string const& key, std::string& value) = 0;
        virtual DbMStatus PutRecord(std::string const& key, std::string const& value) = 0;
        virtual DbMStatus DeleteRecord(std::string const& key) = 0;
    };
};

#endif
