#ifndef TAILPRODUCE_TEST_HELPERS_STORAGES_H
#define TAILPRODUCE_TEST_HELPERS_STORAGES_H

#include "storage_inmemory.h"
#include "storage_leveldb.h"

typedef ::testing::Types<InMemoryTestStorage, LevelDBTestStorage> TestDataStorageImplementationsTypeList;

typedef ::testing::Types<::TailProduce::StreamManager<InMemoryTestStorage>,
                         ::TailProduce::StreamManager<LevelDBTestStorage>> TestStreamManagerImplementationsTypeList;

#endif  // TAILPRODUCE_TEST_HELPERS_STORAGES_H
