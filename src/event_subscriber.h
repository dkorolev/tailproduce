#ifndef EVENT_SUBSCRIBER_H
#define EVENT_SUBSCRIBER_H

#include <cassert>
#include <set>

namespace TailProduce {
    struct Subscriber {
        virtual ~Subscriber() {
        }
        virtual void Poke() = 0;
    };

    struct SubscriptionsManager {
        std::set<Subscriber*> subscribers;
        void RegisterSubscriber(Subscriber* s) {
            // TODO(dkorolev): Throw an exception here.
            assert(!subscribers.count(s));
            subscribers.insert(s);
        }
        void UnregisterSubscriber(Subscriber* s) {
            // TODO(dkorolev): Throw an exception here.
            assert(subscribers.count(s));
            subscribers.erase(s);
        }
        void PokeAll() {
            for (auto it : subscribers) {
                it->Poke();
            }
        }
    };

    template <typename EVENT> struct SubscribeWhileInScope {
        typedef EVENT T_EVENT;
        Subscriber* s;
        T_EVENT& m;
        SubscribeWhileInScope(Subscriber* s, T_EVENT& m) : s(s), m(m) {
            m.RegisterSubscriber(s);
        }
        ~SubscribeWhileInScope() {
            m.UnregisterSubscriber(s);
        }
        SubscribeWhileInScope() = delete;
        SubscribeWhileInScope(const SubscribeWhileInScope&) = delete;
        void operator=(const SubscribeWhileInScope&) = delete;
    };
};

#endif
