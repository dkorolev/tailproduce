#include <cassert>
#include <exception>

#include "leveldb/db.h"

#include "storage_leveldb.h"
#include "tp_exceptions.h"

TailProduce::StorageLevelDB::StorageLevelDB(std::string const& dbname) {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::DB* db;
    leveldb::Status status = leveldb::DB::Open(options, dbname, &db);
    assert(status.ok());
    db_.reset(db);
}

TailProduce::Storage::STORAGE_VALUE_TYPE TailProduce::StorageLevelDB::Get(
    ::TailProduce::Storage::STORAGE_KEY_TYPE const& key) const {
    if (key.empty()) {
        VLOG(3) << "Attempted to Get() an entry with an empty key.";
        VLOG(3) << "throw ::TailProduce::StorageEmptyKeyException();";
        throw ::TailProduce::StorageEmptyKeyException();
    }
    std::string v_get;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), key, &v_get);
    if (s.IsNotFound()) {
        throw ::TailProduce::StorageNoDataException();
    } else {
        if (!s.ok()) throw std::domain_error(s.ToString());
        ::TailProduce::Storage::STORAGE_VALUE_TYPE v_ret(v_get.begin(), v_get.end());
        return v_ret;
    }
}

bool TailProduce::StorageLevelDB::Has(::TailProduce::Storage::STORAGE_KEY_TYPE const& key) const {
    if (key.empty()) {
        VLOG(3) << "Attempted to Has() with an empty key.";
        VLOG(3) << "throw ::TailProduce::StorageEmptyKeyException();";
        throw ::TailProduce::StorageEmptyKeyException();
    }
    std::string v_get;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), key, &v_get);
    return !s.IsNotFound();
}

void TailProduce::StorageLevelDB::InternalSet(::TailProduce::Storage::STORAGE_KEY_TYPE const& key,
                                              ::TailProduce::Storage::STORAGE_VALUE_TYPE const& value,
                                              bool allow_overwrite) {
    if (key.empty()) {
        VLOG(3) << "Attempted to Set() an entry with an empty key.";
        VLOG(3) << "throw ::TailProduce::StorageEmptyKeyException();";
        throw ::TailProduce::StorageEmptyKeyException();
    }
    if (value.empty()) {
        VLOG(3) << "Attempted to Set() an entry with an empty value.";
        VLOG(3) << "throw ::TailProduce::StorageEmptyValueException();";
        throw ::TailProduce::StorageEmptyValueException();
    }
    if (!allow_overwrite) {
        if (Has(key)) {
            VLOG(3) << "'" << key << "', that is attempted to be set to '" << std::string(value.begin(), value.end())
                    << "', has already been set.";
            VLOG(3) << "throw ::TailProduce::StorageOverwriteNotAllowedException();";
            throw ::TailProduce::StorageOverwriteNotAllowedException();
        }
    }
    std::string v_put(value.begin(), value.end());
    leveldb::Status s = db_->Put(leveldb::WriteOptions(), key, v_put);
    if (!s.ok()) throw std::domain_error(s.ToString());
}

void TailProduce::StorageLevelDB::UNUSED_Delete(::TailProduce::Storage::STORAGE_KEY_TYPE const& key) {
    leveldb::Status s = db_->Delete(leveldb::WriteOptions(), key);
    if (!s.ok()) throw std::domain_error(s.ToString());
}

TailProduce::StorageLevelDB::StorageIteratorImpl::StorageIteratorImpl(
    leveldb::DB* p_db,
    ::TailProduce::Storage::STORAGE_KEY_TYPE const& startKey,
    ::TailProduce::Storage::STORAGE_KEY_TYPE const& endKey)
    : p_db_(p_db), endKey_(endKey) {
    it_.reset(p_db_->NewIterator(leveldb::ReadOptions()));
    it_->Seek(startKey);
}

void TailProduce::StorageLevelDB::StorageIteratorImpl::Next() {
    if (Done()) {
        VLOG(3) << "Attempted to Next() an iterator for which Done() is true.";
        VLOG(3) << "throw ::TailProduce::StorageIteratorOutOfBoundsException();";
        throw ::TailProduce::StorageIteratorOutOfBoundsException();
    }
    it_->Next();
}

::TailProduce::Storage::STORAGE_KEY_TYPE TailProduce::StorageLevelDB::StorageIteratorImpl::Key() const {
    if (it_->Valid()) return it_->key().ToString();
    throw std::out_of_range("Can not obtain a Key() from a non valid iterator.");
}

TailProduce::Storage::STORAGE_VALUE_TYPE TailProduce::StorageLevelDB::StorageIteratorImpl::Value() const {
    if (it_->Valid()) {
        ::TailProduce::Storage::STORAGE_VALUE_TYPE r_vec(it_->value().size());
        int elemSize = sizeof(decltype(*(it_->value().data())));
        memcpy(r_vec.data(), it_->value().data(), elemSize * it_->value().size());
        return r_vec;
    }
    // TODO(dkorolev): TailProduce-specific exception instead?
    throw std::out_of_range("Can not obtain a Value() from a non valid iterator.");
}

bool TailProduce::StorageLevelDB::StorageIteratorImpl::HasData() const {
    return it_->Valid() && (endKey_.empty() || it_->key().ToString() < endKey_);
}
