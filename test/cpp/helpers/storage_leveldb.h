#ifndef TAILPRODUCE_TEST_HELPERS_STORAGE_LEVELDB_H
#define TAILPRODUCE_TEST_HELPERS_STORAGE_LEVELDB_H

#include <string>

#include <boost/filesystem.hpp>

#include "../../src/tailproduce.h"
#include "../../src/storage_leveldb.h"
#include "../../src/bytes.h"

typedef ::TailProduce::StorageLevelDB DB;
const std::string LEVELDB_TEST_PATH = "../leveldbTest";

struct DBBeforeTestDeleter {
    explicit DBBeforeTestDeleter(const std::string& pathname) {
        boost::filesystem::remove_all(pathname);
    }
};

struct DBForTestCreator : DB {
    explicit DBForTestCreator(const std::string& pathname) : DB(pathname) {
    }
};

struct LevelDBTestStorage : DBBeforeTestDeleter, DBForTestCreator {
    LevelDBTestStorage() : DBBeforeTestDeleter(LEVELDB_TEST_PATH), DBForTestCreator(LEVELDB_TEST_PATH) {
    }
};

#endif  // TAILPRODUCE_TEST_HELPERS_STORAGE_LEVELDB_H
