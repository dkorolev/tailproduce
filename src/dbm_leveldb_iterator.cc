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

void
TailProduce::DbMLevelDbIterator::CreateIterator_(::TailProduce::Storage::KEY_TYPE const& key, bool advance) {
    it_.reset(db_->NewIterator(leveldb::ReadOptions()));
    it_->Seek(key);
    if (it_->Valid() && advance) {
        it_->Next();
    }
};

TailProduce::DbMLevelDbIterator::DbMLevelDbIterator(std::shared_ptr<leveldb::DB> db, 
                                                    ::TailProduce::Storage::KEY_TYPE const& keyPrefix, 
                                                    ::TailProduce::Storage::KEY_TYPE const& startKey, 
                                                    ::TailProduce::Storage::KEY_TYPE const& endKey) : 
    db_(db), keyPrefix_(keyPrefix), endKey_(endKey) {

    if (keyPrefix.empty()) 
        throw std::invalid_argument("May not start a query with an empty keyPrefix.");

    if (startKey.substr(0, keyPrefix.length()) != keyPrefix) {
        lastKey_ = keyPrefix + startKey;
    }
    else {
        lastKey_ = startKey;
    }

    CreateIterator_(lastKey_, false);
    firstRead_ = it_->Valid();
}

void
TailProduce::DbMLevelDbIterator::Next() {
    if (Done()) 
        throw std::logic_error("Attempting to Next() when at the end of iterator.");

    lastKey_ = it_->key().ToString();
    it_->Next();
    firstRead_ = true;

    if (it_->Valid()) {
        lastKey_ = it_->key().ToString();
        firstRead_ = true;
    }
    else
        CreateIterator_(lastKey_, true);
}

::TailProduce::Storage::KEY_TYPE
TailProduce::DbMLevelDbIterator::Key() const {
    if (it_->Valid())
        return it_->key().ToString();
    throw std::out_of_range("Can not obtain a Key() from a non valid iterator.");
}

TailProduce::Storage::VALUE_TYPE
TailProduce::DbMLevelDbIterator::Value() const {
    if (it_->Valid()) {
        ::TailProduce::Storage::VALUE_TYPE r_vec(it_->value().size());
        int elemSize = sizeof(decltype(*(it_->value().data())));
        memcpy(r_vec.data(), it_->value().data(), elemSize * it_->value().size());
        return r_vec;
    }
    throw std::out_of_range("Can not obtain a Value() from a non valid iterator.");
}

bool
TailProduce::DbMLevelDbIterator::Done() {

    if (!it_->Valid()) {
        // We have moved to the end of the iterator.  Recreate the iterator
        // from the last key (+1) to see if any new entries have arrived.
        CreateIterator_(lastKey_, firstRead_);
    }

    
    if (!it_->Valid()) return true;  // We are done if the iterator is not valid.

    ::TailProduce::Storage::KEY_TYPE key = it_->key().ToString();

    return ((!endKey_.empty() && key >= endKey_ ) || // We are done if we have progressed beyond the last specified
            (key.compare(0, keyPrefix_.length(), keyPrefix_) != 0));  // Or we are done if the keyPrefix is changing
}

