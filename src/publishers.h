// TODO(dkorolev): Rethink which mutexes are necessary and which are not.

#ifndef UNSAFEPUBLISHER_H
#define UNSAFEPUBLISHER_H

#include <glog/logging.h>

#include <set>
#include <vector>
#include <algorithm>
#include <sstream>
#include <mutex>

#include "tp_exceptions.h"
#include "bytes.h"

// TODO(dkorolev): Rename INTERNAL_UnsafePublisher once the transition is completed.

namespace TailProduce {
    // INTERNAL_UnsafePublisher contains the logic of appending data to the streams
    // and updating their HEAD order keys.
    template <typename STREAM> struct INTERNAL_UnsafePublisher {
        typedef STREAM T_STREAM;
        explicit INTERNAL_UnsafePublisher(T_STREAM& stream) : stream(stream) {
        }

        INTERNAL_UnsafePublisher(INTERNAL_UnsafePublisher&&) = default;

        INTERNAL_UnsafePublisher(T_STREAM& stream, const typename T_STREAM::T_ORDER_KEY& order_key)
            : stream(stream) {
            PushHead(order_key);
        }

        void Push(const typename T_STREAM::T_ENTRY& entry) {
            std::lock_guard<std::mutex> guard(stream.lock_mutex());
            typename T_STREAM::T_ORDER_KEY::T_PRIMARY_KEY primary_order_key;
            entry.GetOrderKey(primary_order_key);
            PushHeadUnguarded(primary_order_key);
            std::ostringstream value_output_stream;
            T_STREAM::T_ENTRY::SerializeEntry(value_output_stream, entry);
            stream.manager_->storage.Set(stream.head.ComposeStorageKey(stream, stream.cv),
                                         bytes(value_output_stream.str()));
        }

        void PushHeadUnguarded(const typename T_STREAM::T_ORDER_KEY::T_PRIMARY_KEY& primary_order_key) {
            // TODO(dkorolev): Move this logic to the new keys as well.
            typename T_STREAM::T_ORDER_KEY new_head(primary_order_key, 0);
            if (new_head.primary < stream.head.primary) {
                // Order keys should only be increasing.
                VLOG(3) << "throw ::TailProduce::OrderKeysGoBackwardsException();";
                throw ::TailProduce::OrderKeysGoBackwardsException();
            }
            if (!(stream.head.primary < new_head.primary)) {
                new_head.secondary = stream.head.secondary + 1;
            }
            // TODO(dkorolev): Perhaps more checks here?
            auto v = new_head.ComposeStorageKey(stream, stream.cv);
            stream.manager_->storage.SetAllowingOverwrite(stream.manager_->cv.HeadStorageKey(stream), bytes(v));
            stream.head = new_head;
        }
        void PushHead(const typename T_STREAM::T_ORDER_KEY::T_PRIMARY_KEY& primary_order_key) {
            std::lock_guard<std::mutex> guard(stream.lock_mutex());
            PushHeadUnguarded(primary_order_key);
        }

        // TODO: PushSecondaryKey for merge usecases.

        const typename T_STREAM::T_ORDER_KEY::T_PRIMARY_KEY& GetHead() const {
            std::lock_guard<std::mutex> guard(stream.lock_mutex());
            return stream.head.primary;
        }

        const typename T_STREAM::T_ORDER_KEY& GetHeadPrimaryAndSecondary() const {
            std::lock_guard<std::mutex> guard(stream.lock_mutex());
            return stream.head;
        }

        T_STREAM& stream;

        INTERNAL_UnsafePublisher() = delete;
        INTERNAL_UnsafePublisher(const INTERNAL_UnsafePublisher&) = delete;
        void operator=(const INTERNAL_UnsafePublisher&) = delete;
    };

    // Publisher contains the logic of appending data to the streams and updating their HEAD order keys.
    // It also handles notifying all waiting listeners that new data is now available.
    // Each stream should have one and only one Publisher, regardless of whether it is appended to externally
    // or is being populated by a running TailProduce job.
    template <typename STREAM> struct Publisher {
        typedef STREAM T_STREAM;
        explicit Publisher(T_STREAM& stream) : impl(stream) {
        }
        Publisher(Publisher&&) = default;

        Publisher(T_STREAM& stream, const typename T_STREAM::T_ORDER_KEY& order_key) : impl(stream, order_key) {
        }

        Publisher(T_STREAM& stream, const typename T_STREAM::T_ORDER_KEY::T_PRIMARY_KEY& primary_order_key)
            : impl(stream, T_STREAM::T_ORDER_KEY(primary_order_key)) {
        }

        void Push(const typename T_STREAM::T_ENTRY& entry) {
            impl.Push(entry);
            impl.stream.subscriptions_.PokeAll();
        }

        void PushHead(const typename T_STREAM::T_ORDER_KEY& order_key) {
            impl.PushHead(order_key);
            impl.stream.subscriptions_.PokeAll();
        }

        void PushHead(const typename T_STREAM::T_ORDER_KEY::T_PRIMARY_KEY& primary_order_key) {
            impl.PushHead(typename T_STREAM::T_ORDER_KEY::T_PRIMARY_KEY(primary_order_key));
            impl.stream.subscriptions_.PokeAll();
        }

        // TODO: PushSecondaryKey for merge usecases.

        const typename T_STREAM::T_ORDER_KEY::T_PRIMARY_KEY& GetHead() const {
            return impl.GetHead();
        }

        const typename T_STREAM::T_ORDER_KEY& GetHeadPrimaryAndSecondary() const {
            return impl.GetHeadPrimaryAndSecondary();
        }

        Publisher() = delete;
        Publisher(const Publisher&) = delete;
        void operator=(const Publisher&) = delete;

        INTERNAL_UnsafePublisher<T_STREAM> impl;
    };
};

#endif
