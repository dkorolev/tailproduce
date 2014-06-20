#ifndef _DBM_ITERATOR_H
#define _DBM_ITERATOR_H

// Iterator class that wraps around what ever Data Base Module Iterator we have

namespace TailProduce {
    template <typename dbmodule, typename moduletype>
    auto 
    CreateIterator(dbmodule dbm, 
       std::string const& startKey, 
       std::string const& endKey) -> decltype(std::declval<moduletype>().GetIterator(startKey,endKey)) {
        return dbm->GetIterator(startKey, endKey);
    }

    template <typename It>
    struct DbMIterator {
    public:
        DbMIterator(DbMIterator&&) = default;  // how does a shared_ptr respond with move dynamics??
        DbMIterator(It val) : it_(val) {}
        void Next() { it_->Next(); }
        std::string const& Key() { return it_->Key(); }
        std::string const& Value() { return it_->Value();}
        bool Done() { return it_>Done();}
    private:
        It it_;
        DbMIterator() = delete;
        DbMIterator(DbMIterator const&) = delete;
        DbMIterator& operator=(DbMIterator const&) = delete;
    };
};


#endif
