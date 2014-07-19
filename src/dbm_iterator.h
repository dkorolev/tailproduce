#ifndef _DBM_ITERATOR_H
#define _DBM_ITERATOR_H

// Iterator class that wraps around what ever Data Base Module Iterator we have

#include "storage.h"

namespace TailProduce {
    template <typename It> struct DbMIterator {
      public:
        DbMIterator(DbMIterator&&) = default;  // TODO: understand how a shared_ptr responds with move dynamics??
        DbMIterator(It val) : it_(val) {
        }
        void Next() {
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

      private:
        It it_;
        DbMIterator() = delete;
        DbMIterator(DbMIterator const&) = delete;
        DbMIterator& operator=(DbMIterator const&) = delete;
    };
};

#endif
