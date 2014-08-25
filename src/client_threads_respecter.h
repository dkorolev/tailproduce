#ifndef CLIENT_THREADS_RESPECTER_H
#define CLIENT_THREADS_RESPECTER_H

#include <mutex>

#include "tp_exceptions.h"

namespace TailProduce {
    // An instance of ClientThreadsRespecter will wait in its destructor
    // until all the clients registered with it have successfully terminated.
    //
    // This is accomplished by waiting for the clients to finish in the destructor of ClientThreadsRespecter.
    // Effectively the destructor of ClientThreadsRespecter becomes a blocking call.
    //
    // There are two ways to run client jobs that will be waited for:
    // 1) Use `auto client = respected_instance.RegisterScopedClient();`
    // 2) Use `respected_instance.RunClientCode([](const ClientThreadsRespected::Client& client) { ... });`
    //
    // In both cases the client code should use `if (!client) { ... }` to tell whether it is time to tear down.
    //
    // Note that solution 1) may throw an exception when the instance of ClientThreadsRespecter is already being
    // destroyed.
    // (No new clients are allowed to spawn.)
    // Solution 2) will just return false w/o spawning the client.
    class ClientThreadsRespecter {
      public:
        class Client;
        Client RegisterScopedClient() {
            return Client(this);
        }

        template <typename F> inline bool RunClientCode(F& f) {
            try {
                Client impl(this);
                f(impl);
                return true;
            } catch (AlreadyInTearDownModeException& e) {
                return false;
            }
        }

        inline ~ClientThreadsRespecter() {
            // Mark this object as the one being destructed. This would prevent any new clients from starting.
            {
                std::lock_guard<std::mutex> guard(access_mutex_);
                destructing_ = true;
            }
            // Wait until all active instaces of ScopedUser are destructed.
            ref_count_is_zero_mutex_.lock();
        }

        class Client {
          public:
            inline operator bool() const {
                std::lock_guard<std::mutex> guard(parent_->access_mutex_);
                return !parent_->destructing_;
            }
            Client(Client&&) = default;

            inline ~Client() {
                std::lock_guard<std::mutex> guard(parent_->access_mutex_);
                --parent_->ref_count_;
                if (!parent_->ref_count_) {
                    parent_->ref_count_is_zero_mutex_.unlock();
                }
            }

          private:
            friend class ClientThreadsRespecter;
            inline explicit Client(ClientThreadsRespecter* parent) : parent_(parent) {
                std::lock_guard<std::mutex> guard(parent_->access_mutex_);
                if (parent_->destructing_) {
                    throw AlreadyInTearDownModeException();
                }
                if (!parent_->ref_count_) {
                    parent_->ref_count_is_zero_mutex_.lock();
                }
                ++parent_->ref_count_;
            }

            Client(const Client&) = delete;
            void operator=(const Client&) = delete;

            mutable ClientThreadsRespecter* parent_;
        };

      private:
        std::mutex access_mutex_;
        size_t ref_count_ = 0;
        bool destructing_ = false;
        std::mutex ref_count_is_zero_mutex_;
    };
};  // namespace TailProduce

#endif
