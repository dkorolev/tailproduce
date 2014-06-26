#ifndef _DB_MODULE_H
#define _DB_MODULE_H

#include <string>
#include <vector>

namespace TailProduce {
    typedef std::string Key_Type;
    typedef std::vector<uint8_t> Value_Type;

    template <typename dbmodule, typename moduletype>
    auto 
    CreateIterator(dbmodule dbm, 
                   Key_Type const& startKey, 
                   Value_Type const& endKey) -> decltype(std::declval<moduletype>().GetIterator(startKey,endKey)) {
        return dbm->GetIterator(startKey, endKey);
    }

};

#endif

