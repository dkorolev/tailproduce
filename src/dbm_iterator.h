#ifndef _DBM_ITERATOR_H
#define _DBM_ITERATOR_H

// Iterator class that wraps around what ever Data Base Module Iterator we have

#include "tpexceptions.h"
#include "storage.h"

#include <glog/logging.h>

namespace TailProduce {
    template <typename It> class DbMIterator {
      public:
        DbMIterator(DbMIterator&& rhs) = default;
        explicit DbMIterator(It&& val) {
            // Avoid duplicating the iterator object. In practice, as of now, it enforces the use of unique_ptr<>.
            // TODO(dkorolev): Do we have to keep it this way? Perhaps remove the `DbMIterator` layer of abstraction?
            using std::swap;
            swap(it_, val);
        }
        void Next() {
            if (Done()) {
                VLOG(3) << "Attempted to Next() an iterator for which Done() is true.";
                VLOG(3) << "throw ::TailProduce::StorageIteratorOutOfBoundsException();";
                throw ::TailProduce::StorageIteratorOutOfBoundsException();
            }
            it_->Next();
        }

        ::TailProduce::Storage::KEY_TYPE Key() const {
            return it_->Key();
        }

        ::TailProduce::Storage::VALUE_TYPE Value() const {
            return it_->Value();
        }

        bool Done() {
            return it_->Done();
        }

        bool IsValid() const {
            return it_->IsValid();
        }

      private:
        It it_;
        DbMIterator() = delete;
        DbMIterator(DbMIterator const&) = delete;  // Use move semantics instead.
        DbMIterator(const It& val) = delete;       // Use move semantics instead.
        void operator=(DbMIterator const&) = delete;
    };
};

#endif
