#ifndef TAILPRODUCE_MOCKS_DATA_STORAGE_H
#define TAILPRODUCE_MOCKS_DATA_STORAGE_H

#include <vector>
#include <map>

#include <glog/logging.h>

#include "../../../src/tailproduce.h"

// TODO(dkorolev): Mock data storage implementation should inherit from its base class.

// MockDataStorage supports the following functionality:
//
// 1) Store data as binary key-value pairs.
//    The design decision is to use std::string-s for keys and std::vector<uint8_t>-s for values.
//    Both key and value should not be empty.
//
// 2) Provide read access iterators.
//    Given the range [from, to], or indefinitely from [from, ...].
//    The value at key `from` should exist, the implementation would explicitly die if it does not.
//    
// 3) Die on attempting to overwrite the value for an already existing key.
//    Unless explicitly instructed to.

class MockDataStorage : ::TailProduce::Storage {
  public:
    typedef std::vector<uint8_t> KEY_TYPE;
    typedef std::vector<uint8_t> VALUE_TYPE;
    typedef std::map<KEY_TYPE, VALUE_TYPE> MAP_TYPE;

    void Set(const KEY_TYPE& key, const VALUE_TYPE& value, bool allow_overwrite = false) {
        if (key.empty()) {
            LOG(FATAL) << "Attempted to Set() an entry with an empty key.";
        }
        if (value.empty()) {
            LOG(FATAL) << "Attempted to Set() an entry with an empty value.";
        }
        std::vector<uint8_t>& placeholder = data_[key];
        if (!allow_overwrite) {
            if (!placeholder.empty()) {
                LOG(FATAL)
                    << "'"
                    << std::string(key.begin(), key.end())
                    << "', that is attempted to be set to '"
                    << std::string(value.begin(), value.end())
                    << "', has already been set to '"
                    << std::string(placeholder.begin(), placeholder.end())
                    << "'.";
            }
        }
        placeholder = value;
    }

    void SetAllowingOverwrite(const KEY_TYPE& key, const VALUE_TYPE& value) {
        Set(key, value, true);
    }

    void Get(const KEY_TYPE& key, VALUE_TYPE& value) const {
        if (key.empty()) {
            LOG(FATAL) << "Attempted to Get() an entry with an empty key.";
        }
        const auto cit = data_.find(key);
        if (cit != data_.end()) {
            value = cit->second;
        } else {
            value.clear();
        }
    }

    // TODO(dkorolev): Read more about the move semantics of C++ and eliminate a potentially unoptimized copy.
    VALUE_TYPE Get(const KEY_TYPE& key) const {
        VALUE_TYPE value;
        Get(key, value);
        return value;
    }

    struct Iterator {
        Iterator(Iterator&&) = default;

        bool Valid() const {
            return cit_ != data_.end() && (end_.empty() || cit_->first < end_);
        }

        bool Done() const {
            return !Valid();
        }

        void Next() {
            if (Done()) {
                LOG(FATAL) << "Attempted to Next() an iterator for which Done() is true.";
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
        // Allow returning Iterators from Storage's member functions w/o copying them.
        Iterator(MockDataStorage& master, const KEY_TYPE& begin = KEY_TYPE(), const KEY_TYPE& end = KEY_TYPE())
            : data_(master.data_),
              end_(end),
              cit_(data_.lower_bound(begin)) {
        }
        friend class MockDataStorage;

        const MAP_TYPE& data_;
        KEY_TYPE end_;
        typename MAP_TYPE::const_iterator cit_;

        Iterator() = delete;
        Iterator(const Iterator&) = delete;
        void operator=(const Iterator&) = delete;
    };

    Iterator GetIterator(const KEY_TYPE& begin = KEY_TYPE(), const KEY_TYPE& end = KEY_TYPE()) {
        return Iterator(*this, begin, end);
    }

  private:
    MAP_TYPE data_;
};

#endif  // TAILPRODUCE_MOCKS_DATA_STORAGE_H
