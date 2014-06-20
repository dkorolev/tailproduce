#ifndef _DB_MODULE_H
#define _DB_MODULE_H

#include <string>
#include <functional>
#include <memory>

namespace TailProduce {
    typedef std::string Key_Type;
    typedef std::string Value_Type;

    struct DbMStatus {
        DbMStatus(int val, std::string const& description) : 
            status_(static_cast<Status>(val)), description_(description) {}
        std::string const& Description() { return description_; }
        bool Ok() { return status_ == Status::OK; };

        enum Status { OK, NotFound, Corruption, NotSupported, InvalidArgument, IOError, UnknownError} status_;
        std::string description_;
    };



    class DbModule {
    public:

        virtual DbMStatus GetRecord(Key_Type const& key, Value_Type& value) = 0;
        virtual DbMStatus PutRecord(Key_Type const& key, Value_Type const& value) = 0;
        virtual DbMStatus DeleteRecord(Key_Type const& key) = 0;
    };
};

#endif

