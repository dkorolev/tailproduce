#include "leveldb/db.h"
#include <glog/logging.h>
#include "dbm_leveldb_iterator.h"

TailProduce::DbMLevelDbIterator::DbMLevelDbIterator(std::shared_ptr<leveldb::DB> db, 
                                                    Key_Type const& startKey, 
                                                    Value_Type const& endKey) : db_(db), endKey_(endKey) {
    it_.reset(db_->NewIterator(leveldb::ReadOptions()));
    it_->Seek(startKey);
    if (it_->Valid())
        lastKey_ = it_->key().ToString();
}

void
TailProduce::DbMLevelDbIterator::Next() {
                
    if (Done()) LOG(FATAL) << "Attempted to Next() an iterator for which Done() is true.";
    it_->Next();
    lastKey_ = it_->key().ToString();
}

std::string 
TailProduce::DbMLevelDbIterator::Key() const {
    if (it_->Valid())
        return it_->key().ToString();
    LOG(FATAL) << "Can not obtain a Key() from a non valid iterator.";
}

std::string 
TailProduce::DbMLevelDbIterator::Value() const {
    if (it_->Valid())
        return it_->value().ToString();
    LOG(FATAL) << "Can not obtain a Value() from a non valid iterator.";
}

bool
TailProduce::DbMLevelDbIterator::Done() {
    if (!it_->Valid() || it_->key().ToString() >= endKey_) {
        // we have reached the end.  Create a new iterator iterating from the lastKey (+1)
        // to the end. 
        if (!lastKey_.empty()) {
            it_.reset(db_->NewIterator(leveldb::ReadOptions()));
            it_->Seek(lastKey_);
            it_->Next();
        }
    }
    return (!it_->Valid() || it_->key().ToString() >= endKey_ );
}

