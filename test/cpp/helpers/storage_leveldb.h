#ifndef TAILPRODUCE_TEST_HELPERS_STORAGE_LEVELDB_H
#define TAILPRODUCE_TEST_HELPERS_STORAGE_LEVELDB_H

#include <string>
#include <sstream>
#include <chrono>

#include "../../src/tailproduce.h"
#include "../../src/storage_leveldb.h"
#include "../../src/bytes.h"

typedef ::TailProduce::StorageLevelDB DB;

struct DBForTestCreator : DB {
    explicit DBForTestCreator(const std::string& pathname) : DB(pathname) {
    }
};

struct LevelDBTestStorage : DB {
    LevelDBTestStorage() : DB(GenerateDBName()) {
    }
    static std::string GenerateDBName() {
        static int index = 0;
        std::ostringstream os;
        os << "../testdata-leveldb-"
           << std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::system_clock::now().time_since_epoch()).count() << "-" << ++index << "/";
        return os.str();
    }
};

#endif  // TAILPRODUCE_TEST_HELPERS_STORAGE_LEVELDB_H
