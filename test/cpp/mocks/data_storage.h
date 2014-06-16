#ifndef TAILPRODUCE_MOCKS_DATA_STORAGE_H
#define TAILPRODUCE_MOCKS_DATA_STORAGE_H

#include <vector>
#include <map>

#include <glog/logging.h>

// TODO(dkorolev): Mock data storage implementation should inherit from its base class.

// MockDataStorage supports the following functionality:
// 1) Store data as bianry key-value pairs.
//    Both key and value should not be empty.
// 2) Provide read access iterators. Given the range [from, to], or indefinitely from [from, ...].
//    Read iterators are capable of resuming reading new data once it arrives.
// 3) Die on attempting to overwrite the value for an already existing key.

class MockDataStorage {
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

    // TODO(dkorolev): Read more about the move semantics of C++.
    VALUE_TYPE Get(const KEY_TYPE& key) const {
        VALUE_TYPE value;
        Get(key, value);
        return value;
    }

    struct Iterator {
        // TODO(dkorolev): Read more about the move semantics of C++.
        // Allow returning Iterators from Storage's member functions w/o copying them.
        Iterator(MockDataStorage& master, const KEY_TYPE& from = KEY_TYPE(), const KEY_TYPE& to = KEY_TYPE())
            : data_(master.data_),
              current_(from),
              upper_bound_(false),
              to_(to),
              cit_(data_.end()) {
        }
        bool Done() const {
            // Support dynamic iteration.
            // 1) Done() would check if new data has arrived and would adjust accordingly.
            // 2) Done() would be called in Key(), Value() and Next() to ensure this adjustment.
            if (cit_ == data_.end()) {
                if (!upper_bound_) {
                    // Point to the key that is current_ or above.
                    // Only used before any advacement has taken place.
                    cit_ = data_.lower_bound(current_);
                } else {
                    // Point to the key right after the current_ one.
                    // Used to resume if any advancement has taken place.
                    cit_ = data_.upper_bound(current_);
                }
            }
            return (cit_ == data_.end()) || (!to_.empty() && cit_->first > to_);
        }
        void Next() {
            if (Done()) {
                LOG(FATAL) << "Attempted to Next() an iterator for which Done() is true.";
            }
            current_ = cit_->first;
            upper_bound_ = true;
            ++cit_;
        }
        const KEY_TYPE& Key() const {
            EXPECT_FALSE(Done());
            return cit_->first;
        }
        const KEY_TYPE& Value() const {
            EXPECT_FALSE(Done());
            return cit_->second;
        }
        const MAP_TYPE& data_;
        KEY_TYPE current_;
        bool upper_bound_;
        KEY_TYPE to_;
        mutable typename MAP_TYPE::const_iterator cit_;

      private:
        Iterator() = delete;
        Iterator(const Iterator&) = delete;
        void operator=(const Iterator&) = delete;
    };

  private:
    MAP_TYPE data_;
};

#endif  // TAILPRODUCE_MOCKS_DATA_STORAGE_H
