#ifndef TAILPRODUCE_TEST_HELPERS_STORAGES_H
#define TAILPRODUCE_TEST_HELPERS_STORAGES_H

#include "storage_inmemory.h"
#include "storage_leveldb.h"

typedef ::testing::Types<InMemoryTestStorageManager, LevelDBTestStorageManager>
    TestDataStorageImplementationsTypeList;

typedef ::testing::Types<::TailProduce::StreamManager<InMemoryTestStorageManager>,
                         ::TailProduce::StreamManager<LevelDBTestStorageManager>>
    TestStreamManagerImplementationsTypeList;

#endif  // TAILPRODUCE_TEST_HELPERS_STORAGES_H
