#ifndef _DBM_STATUS_H
#define _DBM_STATUS_H

#include <string>

namespace TailProduce {
    struct DbMStatus {
        DbMStatus(int val, std::string const& description): status_((Status)val), description_(description) {}
        std::string const& Description() { return description_; }
        bool Ok() { return status_ == Status::OK; };

        enum Status { OK, NotFound, Corruption, NotSupported, InvalidArgument, IOError} status_;
        std::string description_;
    };
};
#endif
