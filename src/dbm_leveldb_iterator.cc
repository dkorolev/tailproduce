#include <exception>
#include <cstring>
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

TailProduce::DbMLevelDbIterator::DbMLevelDbIterator(std::shared_ptr<leveldb::DB> db,
                                                    ::TailProduce::Storage::KEY_TYPE const& keyPrefix,
                                                    ::TailProduce::Storage::KEY_TYPE const& startKey,
                                                    ::TailProduce::Storage::KEY_TYPE const& endKey)
    : db_(db), keyPrefix_(keyPrefix), endKey_(endKey) {

    ::TailProduce::Storage::KEY_TYPE key;

    if (startKey.substr(0, keyPrefix.length()) != keyPrefix) {
        key = keyPrefix + startKey;
    } else {
        key = startKey;
    }

    it_.reset(db_->NewIterator(leveldb::ReadOptions()));
    it_->Seek(key);
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

bool TailProduce::DbMLevelDbIterator::IsValid() const {
    return it_->Valid();
}

bool TailProduce::DbMLevelDbIterator::Done() {
    if (!it_->Valid()) return true;  // We are done if the iterator is not valid.

    ::TailProduce::Storage::KEY_TYPE key = it_->key().ToString();

    return ((!endKey_.empty() && key >= endKey_) ||  // We are done if we have progressed beyond the last specified
            (key.compare(0, keyPrefix_.length(), keyPrefix_) != 0));  // Or we are done if the keyPrefix is changing
}
