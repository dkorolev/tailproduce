#ifndef _STORAGE_MANAGER_H
#define _STORAGE_MANAGER_H

#include <memory>
#include "db_module.h"
#include "notifier.h"


namespace TailProduce {
    class StorageManager {
    public:
        StorageManager(DbModule* db);
        DbMStatus Set(std::string const& streamId,
                      std::string const& key, 
                      std::string const& value, 
                      bool allow_overwrite = false);
        
        DbMStatus SetAllowingOverwite(std::string const& streamId, 
                                      std::string const& key, 
                                      std::string const& value);
        
        DbMStatus Get(std::string const& streamId, 
                      std::string const& key, 
                      std::string& value);

        void GetRange(std::string const& streamId, 
                      std::string const& startKey,
                      std::string const& stopKey);
        DbMStatus GetRecord(std::string const& streamId, std::string const& key);
        DbMStatus PutRecord(std::string const& streamId, 
                            std::string const& key, 
                            std::string const& value, 
                            bool allow_overwrites = false);
        DbMStatus DeleteRecord(std::string const& streamId, std::string const& key);

        void GetListener(std::string const& streamId, NotifierCallback cb);
        void PutListener(std::string const& streamId, NotifierCallback cb);
        void DeleteListener(std::string const& streamId, NotifierCallback cb);
    private:
        std::unique_ptr<DbModule> dbm_;
        Notifier notifier_;

        std::string lastWriteKey_(std::string const& streamId);
        std::string lastWriteValue_(std::string const& streamId);
        void eventListener_(std::string const& event, 
                            std::string const& streamId, 
                            NotifierCallback cb) {
            notifier_.RegisterCallback(event, streamId, cb);
        };
    };
};

#endif

