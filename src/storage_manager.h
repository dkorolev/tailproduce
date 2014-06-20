#ifndef _STORAGE_MANAGER_H
#define _STORAGE_MANAGER_H

#include <memory>
#include <vector>
#include "db_module.h"
#include "dbm_iterator.h"

namespace TailProduce {
    class StorageManager {
    public:
        StorageManager(DbModule* db);
        DbMStatus Set(Key_Type const& key, 
                      Value_Type const& value, 
                      bool allow_overwrite = false);
        
        DbMStatus Get(Key_Type const& key, Value_Type& value);

    private:
        std::shared_ptr<DbModule> dbm_;
    };
};

#endif

