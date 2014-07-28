#ifndef TAILPRODUCE_TEST_HELPERS_STORAGE_LEVELDB_H
#define TAILPRODUCE_TEST_HELPERS_STORAGE_LEVELDB_H

#include <string>

#include <boost/filesystem.hpp>

#include "../../src/tailproduce.h"
#include "../../src/helpers.h"

#include "../../src/dbm_leveldb.h"
#include "../../src/dbm_leveldb_iterator.h"

const std::string LEVELDB_TEST_PATH = "../leveldbTest";

struct LevelDBBeforeTestDeleter {
    explicit LevelDBBeforeTestDeleter(const std::string& pathname) {
        boost::filesystem::remove_all(pathname);
    }
};

struct LevelDBCreator {
    explicit LevelDBCreator(const std::string& pathname) : db_(pathname) {
    }
    ::TailProduce::DbMLevelDb db_;
};

typedef ::TailProduce::StorageManager<::TailProduce::DbMLevelDb> LevelDBStorageManager;
struct LevelDBTestDataStorage : LevelDBBeforeTestDeleter, LevelDBCreator, LevelDBStorageManager {
    LevelDBTestDataStorage()
        : LevelDBBeforeTestDeleter(LEVELDB_TEST_PATH),
          LevelDBCreator(LEVELDB_TEST_PATH),
          LevelDBStorageManager(db_) {
    }
};

#endif  // TAILPRODUCE_TEST_HELPERS_STORAGE_LEVELDB_H
