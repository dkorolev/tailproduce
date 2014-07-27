#ifndef _STORAGE_MANAGER_H
#define _STORAGE_MANAGER_H

#include <utility>
#include "storage.h"
#include "dbm_iterator.h"

namespace TailProduce {
    template <typename dbmodule> class StorageManager : public Storage {
      private:
        dbmodule& dbm_;

      public:
        explicit StorageManager(dbmodule& dbm) : dbm_(dbm) {
        }

        typedef typename dbmodule::StorageIterator StorageIterator;

        void Set(::TailProduce::Storage::KEY_TYPE const& key, VALUE_TYPE const& value) {
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
            if (Has(key)) {
                VLOG(3) << "throw ::TailProduce::StorageOverwriteNotAllowedException();";
                throw ::TailProduce::StorageOverwriteNotAllowedException();
            }
            dbm_.PutRecord(key, value);
        }

        // TODO(dkorolev): Discuss with Brian whether this is still necessary, and, if yes, in which form.
        void SetAllowingOverwrite(::TailProduce::Storage::KEY_TYPE const& key, VALUE_TYPE const& value) {
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
            dbm_.PutRecord(key, value);
        }

        void UNUSED_UNIMPLEMENTED_AdminSet(::TailProduce::Storage::KEY_TYPE const& key, VALUE_TYPE const& value) {
            dbm_.AdminPutRecord(key, value);
        }

        VALUE_TYPE Get(KEY_TYPE const& key) const {
            if (key.empty()) {
                VLOG(3) << "Attempted to Get() an entry with an empty key.";
                VLOG(3) << "throw ::TailProduce::StorageEmptyKeyException();";
                throw ::TailProduce::StorageEmptyKeyException();
            }
            return dbm_.GetRecord(key);
        }

        bool Has(KEY_TYPE const& key) const {
            if (key.empty()) {
                VLOG(3) << "Attempted to Has() an entry with an empty key.";
                VLOG(3) << "throw ::TailProduce::StorageEmptyKeyException();";
                throw ::TailProduce::StorageEmptyKeyException();
            }
            return dbm_.HasRecord(key);
        }

        StorageIterator CreateStorageIterator(const KEY_TYPE& begin = KEY_TYPE(), const KEY_TYPE& end = KEY_TYPE()) {
            return dbm_.CreateStorageIterator(begin, end);
        }
    };
};

#endif
