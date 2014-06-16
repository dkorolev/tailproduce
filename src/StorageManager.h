#ifndef _STORAGE_MANAGER_H
#define _STORAGE_MANAGER_H

#include <memory>
#include "DbModule.h"
#include "Notifier.h"


namespace TailProduce {
    class StorageManager {
    public:
        StorageManager(DbModule* db);
        bool getRange(std::string const& streamId, 
                      std::string const& startKey,
                      std::string const& stopKey);
        bool getRecord(std::string const& streamId, std::string const& key);
        bool putRecord(std::string const& streamId, 
                       std::string const& key, 
                       std::string const& value);
        bool deleteRecord(std::string const& streamId, std::string const& key);

        void getListener(std::string const& streamId, NotifierCallback cb);
        void putListener(std::string const& streamId, NotifierCallback cb);
        void deleteListener(std::string const& streamId, NotifierCallback cb);
    private:
        std::unique_ptr<DbModule> dbm_;
        Notifier notifier_;

        std::string lastWriteKey_(std::string const& streamId);
        std::string lastWriteValue_(std::string const& streamId);
        void eventListener_(std::string const& event, 
                            std::string const& streamId, 
                            NotifierCallback cb) {
            notifier_.registerCallback(event, streamId, cb);
        };
    };
};

#endif

