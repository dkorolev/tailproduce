#include <exception>
#include <cstring>

#include <glog/logging.h>

#include "leveldb/db.h"

#include "dbm_leveldb_iterator.h"

#if 0
#include <iostream>

// A convenient function for debugging
void showValid( leveldb::Iterator* it_) {
    std::cout << "Valid " << it_->Valid() << std::endl;
    if (it_->Valid()) {
        std::cout << "Key = " << it_->key().ToString() << " Value = " << it_->value().ToString() << std::endl;
    }
}

#endif

TailProduce::DbMLevelDbIterator::DbMLevelDbIterator(leveldb::DB& db,
                                                    ::TailProduce::Storage::KEY_TYPE const& startKey,
                                                    ::TailProduce::Storage::KEY_TYPE const& endKey)
    : db_(db), endKey_(endKey) {
    it_.reset(db_.NewIterator(leveldb::ReadOptions()));
    it_->Seek(startKey);
}

void TailProduce::DbMLevelDbIterator::Next() {
    if (Done()) throw std::out_of_range("Attempting to Next() when at the end of iterator.");
    it_->Next();
}

::TailProduce::Storage::KEY_TYPE TailProduce::DbMLevelDbIterator::Key() const {
    if (it_->Valid()) return it_->key().ToString();
    throw std::out_of_range("Can not obtain a Key() from a non valid iterator.");
}

TailProduce::Storage::VALUE_TYPE TailProduce::DbMLevelDbIterator::Value() const {
    if (it_->Valid()) {
        ::TailProduce::Storage::VALUE_TYPE r_vec(it_->value().size());
        int elemSize = sizeof(decltype(*(it_->value().data())));
        memcpy(r_vec.data(), it_->value().data(), elemSize * it_->value().size());
        return r_vec;
    }
    throw std::out_of_range("Can not obtain a Value() from a non valid iterator.");
}

// TODO(dkorolev): Chat with Brian. I have kept only the HasData() method in the source file,
// to me this looks as the most readable and the least error-prone solution.
bool TailProduce::DbMLevelDbIterator::HasData() const {
    return it_->Valid() && (endKey_.empty() || it_->key().ToString() < endKey_);
}
