#ifndef UNSAFEPUBLISHER_H
#define UNSAFEPUBLISHER_H

namespace TailProduce {
    // UnsafePublisher contains the logic of appending data to the streams and updating their HEAD order keys.
    template<typename T> 
    struct UnsafePublisher {
        UnsafePublisher() = delete;
        explicit UnsafePublisher(T& stream) : stream(stream) {
        }

        UnsafePublisher(T& stream, const typename T::order_key_type& order_key) : stream(stream) {
            PushHead(order_key);
        }

        void Push(const typename T::entry_type& entry) {
            typedef ::TailProduce::OrderKeyExtractorImpl<typename T::order_key_type, typename T::entry_type> impl;
            PushHead(impl::ExtractOrderKey(entry));
            std::ostringstream value_output_stream;
            T::entry_type::SerializeEntry(value_output_stream, entry);
            auto k = stream.key_builder.BuildStorageKey(stream.head);
            stream.manager->storage.Set(stream.key_builder.BuildStorageKey(stream.head), bytes(value_output_stream.str()));
        }

        void PushHead(const typename T::order_key_type& order_key) {
            typename T::head_pair_type new_head(order_key, 0);
            if (new_head.first < stream.head.first) {
                // Order keys should only be increasing.
                VLOG(3) << "throw ::TailProduce::OrderKeysGoBackwardsException();";
                throw ::TailProduce::OrderKeysGoBackwardsException();
            }
            if (!(stream.head.first < new_head.first)) {
                new_head.second = stream.head.second + 1;
            }
            // TODO(dkorolev): Perhaps more checks here?
            //auto v = 123;
            auto v = OrderKey::template 
                StaticSerializeAsStorageKey<typename T::order_key_type>(new_head.first,
                                                                        new_head.second);
            stream.manager->storage.SetAllowingOverwrite(
                stream.key_builder.head_storage_key,
                bytes(v));

            stream.head = new_head;
        }

        // TODO: PushSecondaryKey for merge usecases.

        const typename T::head_pair_type& GetHead() const {
            return stream.head;
        }

      private:
        UnsafePublisher(const UnsafePublisher&) = delete;
        void operator=(const UnsafePublisher&) = delete;
        T& stream;
    };
};
#endif
