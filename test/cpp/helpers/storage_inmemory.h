#ifndef TAILPRODUCE_TEST_HELPERS_STORAGE_INMEMORY_H
#define TAILPRODUCE_TEST_HELPERS_STORAGE_INMEMORY_H

#include <vector>
#include <map>
#include <string>

#include <gtest/gtest.h>
#include <glog/logging.h>

#include "../../../src/tailproduce.h"

// InMemoryTestStorage supports the following functionality:
//
// 1) Store data as binary key-value pairs.
//    The design decision is to use std::string-s for keys and std::vector<uint8_t>-s for values.
//    They are available as ::TailProduce::Storage::{KEY,VALUE}_TYPE respectively.
//    Both key and value should not be empty.
//
// 2) Provide read access iterators.
//    Given the range [from, to), or indefinitely from [from, ...).
//    Allows providing a non-existing key as `from`, uses std::map::lower_bound().
//
// 3) Die on attempting to overwrite the value for an already existing key.
//    Unless explicitly instructed to.

using ::TailProduce::Storage::KEY_TYPE;
using ::TailProduce::Storage::VALUE_TYPE;

class InMemoryTestStorage : ::TailProduce::Storage::Impl<InMemoryTestStorage> {
  public:
    typedef std::map<KEY_TYPE, VALUE_TYPE> MAP_TYPE;

    void Set(const KEY_TYPE& key, const VALUE_TYPE& value, bool allow_overwrite = false) {
        VLOG(3) << "InMemoryTestStorage::Set('" << key << "', '" << ::TailProduce::antibytes(value)
                << (allow_overwrite ? "');" : "', allow_overwrite=true);");
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
        std::vector<uint8_t>& placeholder = data_[key];
        if (!allow_overwrite) {
            if (!placeholder.empty()) {
                VLOG(3) << "'" << key << "', that is attempted to be set to '"
                        << std::string(value.begin(), value.end()) << "', has already been set to '"
                        << std::string(placeholder.begin(), placeholder.end()) << "'.";
                VLOG(3) << "throw ::TailProduce::StorageOverwriteNotAllowedException();";
                throw ::TailProduce::StorageOverwriteNotAllowedException();
            }
        }
        placeholder = value;
    }

    void SetAllowingOverwrite(const KEY_TYPE& key, const VALUE_TYPE& value) {
        Set(key, value, true);
    }

    bool Has(const KEY_TYPE& key) const {
        if (key.empty()) {
            VLOG(3) << "Attempted to Has() with an empty key.";
            VLOG(3) << "throw ::TailProduce::StorageEmptyKeyException();";
            throw ::TailProduce::StorageEmptyKeyException();
        }
        const auto cit = data_.find(key);
        return cit != data_.end();
    }

    VALUE_TYPE Get(const KEY_TYPE& key) const {
        if (key.empty()) {
            VLOG(3) << "Attempted to Get() an entry with an empty key.";
            VLOG(3) << "throw ::TailProduce::StorageEmptyKeyException();";
            throw ::TailProduce::StorageEmptyKeyException();
        }
        const auto cit = data_.find(key);
        if (cit != data_.end()) {
            VLOG(3) << "InMemoryTestStorage::Get('" << ::TailProduce::antibytes(key) << ") == '"
                    << ::TailProduce::antibytes(cit->second) << "'.";
            return cit->second;
        } else {
            VLOG(3) << "throw ::TailProduce::StorageNoDataException();";
            throw ::TailProduce::StorageNoDataException();
        }
    }

    struct StorageIteratorImpl {
        StorageIteratorImpl(InMemoryTestStorage& master,
                            const KEY_TYPE& begin = KEY_TYPE(),
                            const KEY_TYPE& end = KEY_TYPE())
            : data_(master.data_), end_(end), cit_(data_.lower_bound(begin)) {
        }
        StorageIteratorImpl(StorageIteratorImpl&&) = default;

        bool Valid() const {
            return cit_ != data_.end() && (end_.empty() || cit_->first < end_);
        }

        bool Done() const {
            return !Valid();
        }

        void Next() {
            if (Done()) {
                VLOG(3) << "Attempted to Next() an iterator for which Done() is true.";
                VLOG(3) << "throw ::TailProduce::StorageIteratorOutOfBoundsException();";
                throw ::TailProduce::StorageIteratorOutOfBoundsException();
            }
            ++cit_;
        }

        const KEY_TYPE& Key() const {
            EXPECT_FALSE(Done());
            return cit_->first;
        }

        const VALUE_TYPE& Value() const {
            EXPECT_FALSE(Done());
            return cit_->second;
        }

      private:
        const MAP_TYPE& data_;
        KEY_TYPE end_;
        typename MAP_TYPE::const_iterator cit_;

        StorageIteratorImpl() = delete;
        StorageIteratorImpl(const StorageIteratorImpl&) = delete;
        void operator=(const StorageIteratorImpl&) = delete;
    };

    typedef std::unique_ptr<StorageIteratorImpl> StorageIterator;

    StorageIterator CreateStorageIterator(const KEY_TYPE& begin = KEY_TYPE(), const KEY_TYPE& end = KEY_TYPE()) {
        return StorageIterator(new StorageIteratorImpl(*this, begin, end));
    }

  private:
    MAP_TYPE data_;
};

#endif  // TAILPRODUCE_TEST_HELPERS_STORAGE_INMEMORY_H
