#include "leveldb/db.h"
#include "dbm_leveldb_iterator.h"

TailProduce::DbMLevelDbIterator::DbMLevelDbIterator(std::shared_ptr<leveldb::DB> db, 
                                                    std::string const& startKey, 
                                                    std::string const& endKey) : db_(db) {
//    if (endKey.empty()) {
//        endKey_ = infinityKey(startKey);
//    }
    it_.reset(db_->NewIterator(leveldb::ReadOptions()));
    it_->Seek(startKey);
    lastKey_ = it_->key().ToString();
}

void
TailProduce::DbMLevelDbIterator::Next() {
    if (Done()) exit(1);
    it_->Next();
    lastKey_ = it_->key().ToString();
}

std::string 
TailProduce::DbMLevelDbIterator::Key() {
    return it_->Valid() ? it_->key().ToString() : std::string();
}

std::string
TailProduce::DbMLevelDbIterator::Value() {
    return it_->Valid() ? it_->value().ToString() : std::string();
}

bool
TailProduce::DbMLevelDbIterator::Done() {
    if (!it_->Valid() || it_->key().ToString() >= endKey_) {
        // we have reached the end.  Create a new iterator iterating from the lastKey (+1)
        // to the end. 
        auto it = db_->NewIterator(leveldb::ReadOptions());
        it_.reset(it);
        it_->Seek(lastKey_);
        it_->Next();
    }
    return (it_->key().ToString() >= endKey_ || !it_->Valid());
}

