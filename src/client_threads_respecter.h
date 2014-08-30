#ifndef CLIENT_THREADS_RESPECTER_H
#define CLIENT_THREADS_RESPECTER_H

#include <atomic>
#include <mutex>

#include "tp_exceptions.h"

namespace TailProduce {
    // An instance of ClientThreadsRespecter will wait in its destructor
    // until all the clients registered with it have successfully terminated.
    //
    // This is accomplished by waiting for the clients to finish in the destructor of ClientThreadsRespecter.
    // Effectively, the destructor of ClientThreadsRespecter becomes a blocking call.
    //
    // There are two ways to run client jobs that will be waited for:
    // 1) Use `{ auto client = respected_instance.RegisterScopedClient(); ... }`.
    // 2) Use `respected_instance.RunClientCode([](const ClientThreadsRespected::Client& client) { ... });`.
    //
    // In both cases the client code should use `if (!client) { ... }` to tell whether it is time to tear down.
    // Once `if (client)` becomes false, no new clients should be attempted to be created,
    // since the general instance of ClientThreadsRespected will be destroyed as the last alive client is done.
    //
    // Note that solution 1) may throw an exception when the instance of ClientThreadsRespecter
    // is already in the process of being destroyed (and no new clients are allowed to spawn).
    // Solution 2) will just return false w/o spawning the client.
    class ClientThreadsRespecter {
      public:
        class Client;
        Client RegisterScopedClient() {
            return Client(this);
        }

        template <typename F> inline bool RunClientCode(F& f) {
            try {
                f(RegisterScopedClient());
                return true;
            } catch (AlreadyInTearDownModeException& e) {
                return false;
            }
        }

        inline ClientThreadsRespecter() : ref_count_(0), destructing_(false), client_(this) {
        }

        inline ~ClientThreadsRespecter() {
            // Mark this object as the one being destructed. This would prevent any new clients from starting.
            destructing_ = true;
            {
                // Release the instance of the client that used to boost the ref count base from 0 to 1.
                Client delete_client = std::move(client_);
            }
            // Wait until all active instaces of ScopedUser are destructed.
            ref_count_is_zero_mutex_.lock();
        }

        class Client {
          public:
            inline operator bool() const {
                return parent_ && !parent_->destructing_;
            }

            Client(Client&& rhs) {
                // Move constructor should be implemented manually to nullify `rhs.parent_`.
                if (!rhs.parent_) {
                    throw AttemptedToCreateScopedClientForNullParent();
                }
                parent_ = rhs.parent_;
                rhs.parent_ = nullptr;
            }

            inline ~Client() {
                if (parent_ && !--parent_->ref_count_) {
                    parent_->ref_count_is_zero_mutex_.unlock();
                }
            }

          private:
            friend class ClientThreadsRespecter;
            inline explicit Client(ClientThreadsRespecter* parent) : parent_(parent) {
                if (!parent) {
                    throw AttemptedToCreateScopedClientForNullParent();
                }
                if (parent_->destructing_) {
                    throw AlreadyInTearDownModeException();
                }
                if (!parent_->ref_count_++) {
                    parent_->ref_count_is_zero_mutex_.lock();
                }
            }

            Client(const Client&) = delete;
            void operator=(const Client&) = delete;

            mutable ClientThreadsRespecter* parent_;
        };

      private:
        std::atomic_size_t ref_count_;
        std::atomic_bool destructing_;
        std::mutex ref_count_is_zero_mutex_;
        Client client_;
    };
};  // namespace TailProduce

#endif
