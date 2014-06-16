#ifndef _DB_MODULE_H
#define _DB_MODULE_H

#include <string>
#include <functional>
#include "DbMStatus.h"

namespace TailProduce {
    typedef std::function<void(std::string const &)> RangeCallback;

    class DbModule {
    public:
        virtual void getRange(std::string const& startKey, 
                              std::string const& endKey,
                              RangeCallback cb) = 0;
        virtual DbMStatus getRecord(std::string const& key, std::string& value) = 0;
        virtual DbMStatus putRecord(std::string const& key, std::string const& value) = 0;
        virtual DbMStatus deleteRecord(std::string const& key) = 0;
    };
};

#endif
