// TODO(dkorolev): Rethink which mutexes are necessary and which are not.

#ifndef UNSAFELISTENERS_H
#define UNSAFELISTENERS_H

#include <memory>
#include <thread>
#include <mutex>

#include <glog/logging.h>

#include "stream.h"
#include "storage.h"
#include "event_subscriber.h"
#include "tp_exceptions.h"

namespace TailProduce {
    // TODO(dkorolev): Rename this class.
    // INTERNAL_UnsafeListener contains the logic of creating and re-creating storage-level read iterators,
    // presenting data in serialized format and keeping track of HEAD order keys.
    template <typename STREAM> struct INTERNAL_UnsafeListener {
        typedef STREAM T_STREAM;

        // Unbounded.
        ~INTERNAL_UnsafeListener() {
            VLOG(3) << this << ": INTERNAL_UnsafeListener::~INTERNAL_UnsafeListener();";
        }

        INTERNAL_UnsafeListener(const T_STREAM& stream,
                                const typename T_STREAM::T_ORDER_KEY& begin = typename T_STREAM::T_ORDER_KEY())
            : stream(stream),
              storage(stream.manager_->storage),
              storage_cursor_key(begin.ComposeStorageKey(stream, stream.cv)),
              need_to_increment_cursor(false),
              has_end_key(false),
              reached_end(false) {
            VLOG(3) << this << ": INTERNAL_UnsafeListener::INTERNAL_UnsafeListener('" << stream.name << "', "
                    << "begin='" << storage_cursor_key << "');";
        }
        INTERNAL_UnsafeListener(const T_STREAM& stream,
                                const typename T_STREAM::T_ORDER_KEY::T_PRIMARY_KEY& primary_begin)
            : INTERNAL_UnsafeListener(stream, typename T_STREAM::T_ORDER_KEY(primary_begin)) {
        }

        // Bounded.
        INTERNAL_UnsafeListener(const T_STREAM& stream,
                                const typename T_STREAM::T_ORDER_KEY& begin,
                                const typename T_STREAM::T_ORDER_KEY& end)
            : stream(stream),
              storage(stream.manager_->storage),
              storage_cursor_key(begin.ComposeStorageKey(stream, stream.cv)),
              need_to_increment_cursor(false),
              has_end_key(true),
              storage_end_key(end.ComposeStorageKey(stream, stream.cv)),
              reached_end(false) {
            VLOG(3) << this << ": INTERNAL_UnsafeListener::INTERNAL_UnsafeListener('" << stream.name << "', "
                    << "begin='" << storage_cursor_key << "', end='" << storage_end_key << "');";
        }
        INTERNAL_UnsafeListener(const T_STREAM& stream,
                                const typename T_STREAM::T_ORDER_KEY::T_PRIMARY_KEY& primary_begin,
                                const typename T_STREAM::T_ORDER_KEY::T_PRIMARY_KEY& primary_end)
            : INTERNAL_UnsafeListener(stream,
                                      typename T_STREAM::T_ORDER_KEY(primary_begin),
                                      typename T_STREAM::T_ORDER_KEY(primary_end)) {
        }

        const typename T_STREAM::T_ORDER_KEY::T_PRIMARY_KEY& GetHead() const {
            std::lock_guard<std::mutex> guard(stream.lock_mutex());
            return stream.head.primary;
        }

        const typename T_STREAM::T_ORDER_KEY& GetHeadPrimaryAndSecondary() const {
            std::lock_guard<std::mutex> guard(stream.lock_mutex());
            return stream.head;
        }

        // Note that listeners expose HasData() / ReachedEnd(), and not Done().
        // This is because the listener, unlike the iterator, supports dynamically added data,
        // and, therefore, the standard `for (auto i = Iterator(); !i.Done(); i.Next())` loop is meaningless.

        // HasData() returns true if more data is available.
        // Can change from false to true if/when new data is available.
        bool HasDataUnguarded() const {
            if (reached_end) {
                VLOG(3) << this << " INTERNAL_UnsafeListener::HasData() = false, due to reached_end = true.";
                return false;
            } else {
                if (!iterator) {
                    iterator = std::move(storage.CreateStorageIterator(
                        storage_cursor_key, stream.manager_->cv.EndDataStorageKey(stream)));
                    if (need_to_increment_cursor && !iterator->Done()) {
                        iterator->Next();
                    }
                }
                if (iterator->Done()) {
                    iterator.reset(nullptr);
                    VLOG(3) << this
                            << " INTERNAL_UnsafeListener::HasData() = false, due to no data in the iterator.";
                    return false;
                }
                assert(iterator && !iterator->Done());
                if (has_end_key && iterator->Key() >= storage_end_key) {
                    VLOG(3) << this << " INTERNAL_UnsafeListener::HasData() = false, due to reaching the end.";
                    reached_end = true;
                    iterator.reset(nullptr);
                    return false;
                } else {
                    // TODO(dkorolev): Handle HEAD going beyond storage_end_key resulting in ReachedEnd().
                    VLOG(3) << this << " INTERNAL_UnsafeListener::HasData() = true.";
                    return true;
                }
            }
        }
        bool HasData() const {
            std::lock_guard<std::mutex> guard(stream.lock_mutex());
            return HasDataUnguarded();
        }

        // ReachedEnd() returns true if the end has been reached and no data may even be read from this iterator.
        // Can only happen if the iterator has a fixed `end`, it has been reached and the HEAD of this stream
        // is beyond this end.
        bool ReachedEnd() const {
            HasData();
            return reached_end;
        }

        // ProcessEntrySync() deserealizes the entry and calls the supplied method of the respective type.
        template <typename PROCESSOR> void ProcessEntrySync(PROCESSOR& processor, bool require_data = true) {
            std::string key_as_string;  // For logging purposes, should be removed in non-debug builds.
            std::string value_as_string;
            {
                std::lock_guard<std::mutex> guard(stream.lock_mutex());
                if (!HasDataUnguarded()) {
                    if (require_data) {
                        VLOG(3) << "throw ::TailProduce::ListenerHasNoDataToRead();";
                        throw ::TailProduce::ListenerHasNoDataToRead();
                    } else {
                        return;
                    }
                }
                if (!iterator) {
                    VLOG(3) << "throw ::TailProduce::InternalError();";
                    throw ::TailProduce::InternalError();
                }
                // TODO(dkorolev): Make this proof-of-concept code efficient.
                ::TailProduce::Storage::STORAGE_VALUE_TYPE const value = iterator->Value();
                value_as_string = std::string(value.begin(), value.end());

                // For logging purposes, should be removed in non-debug builds.
                ::TailProduce::Storage::STORAGE_KEY_TYPE const key = iterator->Key();
                key_as_string = std::string(key.begin(), key.end());
            }
            VLOG(3) << this << " INTERNAL_UnsafeListener::ProcessEntrySync(): ['" << key_as_string << "'] = '"
                    << value_as_string << "'";
            std::istringstream is(value_as_string);
            T_STREAM::T_ENTRY::DeSerializeAndProcessEntry(is, processor);
        }

        // AdvanceToNextEntry() advances the listener to the next available entry.
        // Will throw an exception if no further data is (yet) available.
        void AdvanceToNextEntry() {
            std::lock_guard<std::mutex> guard(stream.lock_mutex());
            if (!HasDataUnguarded()) {
                VLOG(3) << "throw ::TailProduce::AttemptedToAdvanceListenerWithNoDataAvailable();";
                throw ::TailProduce::AttemptedToAdvanceListenerWithNoDataAvailable();
            }
            if (!iterator) {
                VLOG(3) << "throw ::TailProduce::InternalError();";
                throw ::TailProduce::InternalError();
            }
            storage_cursor_key = iterator->Key();
            need_to_increment_cursor = true;
            iterator->Next();
        }

      private:
        const T_STREAM& stream;
        typename T_STREAM::T_STORAGE& storage;
        ::TailProduce::Storage::STORAGE_KEY_TYPE storage_cursor_key;
        bool need_to_increment_cursor;
        const bool has_end_key;
        ::TailProduce::Storage::STORAGE_KEY_TYPE const storage_end_key;
        mutable bool reached_end;
        mutable typename T_STREAM::T_STORAGE::StorageIterator iterator;

        INTERNAL_UnsafeListener() = delete;
        INTERNAL_UnsafeListener(const INTERNAL_UnsafeListener&) = delete;
        INTERNAL_UnsafeListener(INTERNAL_UnsafeListener&&) = delete;
        void operator=(const INTERNAL_UnsafeListener&) = delete;
    };

    // TODO(dkorolev): Add support for other listener types, not just "all" range.
    template <typename T_STREAM> struct AsyncListenersFactory {
        AsyncListenersFactory(const T_STREAM& stream) : stream(stream) {
        }

        template <typename PROCESSOR> struct AsyncListener : ::TailProduce::Subscriber {
            typedef PROCESSOR T_PROCESSOR;
            AsyncListener(const T_STREAM& stream, T_PROCESSOR& processor)
                : stream(stream),
                  processor(processor),
                  subscribe(this, stream.subscriptions_),
                  worker_thread(&AsyncListener::ThreadFunction, this) {
            }
            virtual ~AsyncListener() {
                VLOG(2) << this << " AsyncListener::~AsyncListener(): Waiting for the thread to terminate.";
                terminating = true;
                worker_thread.join();
                VLOG(2) << this << " AsyncListener::~AsyncListener(): Thread terminated.";
            }

            void ThreadFunction() {
                INTERNAL_UnsafeListener<T_STREAM> impl(stream);
                // TODO(dkorolev): This, of course, should not be based on this repeated check.
                while (!terminating) {
                    while (!terminating && !impl.ReachedEnd() && impl.HasData()) {
                        VLOG(3) << this << " AsyncListener::ThreadFunction(): Has entry.";
                        impl.ProcessEntrySync(processor);
                        VLOG(3) << this << " AsyncListener::ThreadFunction(): Processed entry.";
                        impl.AdvanceToNextEntry();
                        VLOG(3) << this << " AsyncListener::ThreadFunction(): Advanced to next entry.";
                    }
                    {
                        std::lock_guard<std::mutex> guard(mutex);
                        ++cycles;
                    }
                }
            }

            // TODO(dkorolev): This, of course, should use std::condition_variable.
            void WaitUntilCurrent() {
                int safe_cycles;
                {
                    std::lock_guard<std::mutex> guard(mutex);
                    safe_cycles = cycles;
                }
                while (true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    {
                        std::lock_guard<std::mutex> guard(mutex);
                        if (cycles != safe_cycles) {
                            return;
                        }
                    }
                }
            }

            virtual void Poke() override {
                // TODO(dkorolev): ALARM! This will fail miserably if a stream is appended to as a direct result
                // of it just being appended to. Async implementation will take care of it.
                // RunFakeEventLoop();
            }

          private:
            const T_STREAM& stream;

            T_PROCESSOR& processor;
            ::TailProduce::SubscribeWhileInScope<::TailProduce::SubscriptionsManager> subscribe;

            std::mutex mutex;
            bool terminating = false;
            size_t cycles = 0;

            std::thread worker_thread;

            AsyncListener() = delete;
            AsyncListener(const AsyncListener&) = delete;
            AsyncListener(AsyncListener&& rhs) = default;
            void operator=(const AsyncListener&) = delete;
        };

        template <typename PROCESSOR> std::unique_ptr<AsyncListener<PROCESSOR>> operator()(PROCESSOR& processor) {
            std::lock_guard<std::mutex> guard(stream.lock_mutex());
            return std::unique_ptr<AsyncListener<PROCESSOR>>(new AsyncListener<PROCESSOR>(stream, processor));
        }

      private:
        const T_STREAM& stream;
    };
};

#endif
