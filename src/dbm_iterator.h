#ifndef _DBM_ITERATOR_H
#define _DBM_ITERATOR_H

// Iterator class that wraps around what ever Data Base Module Iterator we have

namespace TailProduce {
    template <typename It>  // type must be a (unique) ptr
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
