#ifndef _DBM_ITERATOR_H
#define _DBM_ITERATOR_H

// Iterator class that wraps around what ever Data Base Module Iterator we have

#include "db_module.h"
namespace TailProduce {
    template <typename dbmodule, typename moduletype>
    auto 
    CreateIterator(dbmodule dbm, 
       Key_Type const& startKey, 
       Value_Type const& endKey) -> decltype(std::declval<moduletype>().GetIterator(startKey,endKey)) {
        return dbm->GetIterator(startKey, endKey);
    }

    template <typename It>
    struct DbMIterator {
    public:
        DbMIterator(DbMIterator&&) = default;  // TODO: understand how a shared_ptr responds with move dynamics??
        DbMIterator(It val) : it_(val) {}
        void Next() { it_->Next(); }
        Key_Type Key() const { return it_->Key(); }
        Value_Type Value() const { return it_->Value();}
        bool Done() { return it_->Done(); }
    private:
        It it_;
        DbMIterator() = delete;
        DbMIterator(DbMIterator const&) = delete;
        DbMIterator& operator=(DbMIterator const&) = delete;
    };
};


#endif
