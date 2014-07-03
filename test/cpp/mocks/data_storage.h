#ifndef TAILPRODUCE_MOCKS_DATA_STORAGE_H
#define TAILPRODUCE_MOCKS_DATA_STORAGE_H

#include <vector>
#include <map>

#include <glog/logging.h>

#include "../../../src/tailproduce.h"

// MockDataStorage supports the following functionality:
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

class MockDataStorage : ::TailProduce::Storage {
  public:
    typedef std::map<KEY_TYPE, VALUE_TYPE> MAP_TYPE;

    void Set(const KEY_TYPE& key, const VALUE_TYPE& value, bool allow_overwrite = false) {
        VLOG(3)
            << "MockDataStorage::Set('"
            << ::TailProduce::antibytes(key)
            << "', '"
            << ::TailProduce::antibytes(value)
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
                VLOG(3)
                    << "'"
                    << std::string(key.begin(), key.end())
                    << "', that is attempted to be set to '"
                    << std::string(value.begin(), value.end())
                    << "', has already been set to '"
                    << std::string(placeholder.begin(), placeholder.end())
                    << "'.";
                VLOG(3) << "throw ::TailProduce::StorageOverwriteNotAllowedException();";
                throw ::TailProduce::StorageOverwriteNotAllowedException();
            }
        }
        placeholder = value;
    }

    void SetAllowingOverwrite(const KEY_TYPE& key, const VALUE_TYPE& value) {
        Set(key, value, true);
    }

    bool Has(const KEY_TYPE& key) {
        if (key.empty()) {
            VLOG(3) << "Attempted to Has() with an empty key.";
            VLOG(3) << "throw ::TailProduce::StorageEmptyKeyException();";
            throw ::TailProduce::StorageEmptyKeyException();
        }
        const auto cit = data_.find(key);
        return cit != data_.end();
    }

    void Get(const KEY_TYPE& key, VALUE_TYPE& value) const {
        if (key.empty()) {
            VLOG(3) << "Attempted to Get() an entry with an empty key.";
            VLOG(3) << "throw ::TailProduce::StorageEmptyKeyException();";
            throw ::TailProduce::StorageEmptyKeyException();
        }
        const auto cit = data_.find(key);
        if (cit != data_.end()) {
            value = cit->second;
        } else {
            VLOG(3) << "throw ::TailProduce::StorageNoDataException();";
            throw ::TailProduce::StorageNoDataException();
        }
        VLOG(3)
            << "MockDataStorage::Get('"
            << ::TailProduce::antibytes(key)
            << ") == '"
            << ::TailProduce::antibytes(value)
            << "'.";
    }

    // TODO(dkorolev): Read more about move semantics of C++ and eliminate a potentially unoptimized copy.
    VALUE_TYPE Get(const KEY_TYPE& key) const {
        VALUE_TYPE value;
        Get(key, value);
        return value;
    }

    struct Iterator {
        Iterator(MockDataStorage& master, const KEY_TYPE& begin = KEY_TYPE(), const KEY_TYPE& end = KEY_TYPE())
            : data_(master.data_),
              end_(end),
              cit_(data_.lower_bound(begin)) {
        }

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

        Iterator() = delete;
        Iterator(const Iterator&) = delete;
        Iterator(Iterator&&) = delete;
        void operator=(const Iterator&) = delete;
    };
    
  private:
    MAP_TYPE data_;
};

#endif  // TAILPRODUCE_MOCKS_DATA_STORAGE_H
