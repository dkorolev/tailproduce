// TODO(dkorolev): Perhaps we should move StreamsRegistry to become the part of the StreamManager base class?

#ifndef TAILPRODUCE_H
#define TAILPRODUCE_H

#include <glog/logging.h>

#include "tpexceptions.h"

#include "storage.h"
#include "streams_registry.h"
#include "stream.h"
#include "entry.h"
#include "order_key.h"
#include "stream_instance.h"
#include "serialize.h"
#include "stream_manager.h"
#include "storage_key_builder.h"
#include "listeners.h"
#include "publishers.h"
#include "stream_manager_params.h"


// TailProduce static framework macros.
// Static framework is the one that lists all the streams in the source file,
// thus allowing all C++ template and static typing powers to come into play.
#define TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(NAME, BASE) \
    class NAME { \
      public: \
        typedef typename BASE::storage_type storage_type; \
        storage_type& storage; \
        static storage_type& EnsureStreamsAreCreatedDuringInitialization( \
                storage_type& storage, \
                const ::TailProduce::StreamManagerParams& params) { \
            params.Apply(storage); \
            return storage; \
        } \
        NAME(storage_type& storage, \
             const ::TailProduce::StreamManagerParams& params = ::TailProduce::StreamManagerParams::FromCommandLineFlags()) \
          : storage(EnsureStreamsAreCreatedDuringInitialization(storage, params)) {} \
        NAME(const NAME&) = delete; \
        NAME(NAME&&) = delete; \
        void operator=(const NAME&) = delete; \
      private: \
        using TSM = ::TailProduce::StreamManager; \
        static_assert(std::is_base_of<TSM, BASE>::value, \
                      "TAILPRODUCE_STATIC_FRAMEWORK_BEGIN: BASE should be derived from StreamManager."); \
        using TS = ::TailProduce::Storage; \
        static_assert(std::is_base_of<TS, storage_type>::value, \
                      "TAILPRODUCE_STATIC_FRAMEWORK_BEGIN: BASE::storage_type should be derived from Storage."); \
        ::TailProduce::StreamsRegistry registry_;  \
      public: \
        typedef BASE captured_base; \
        const ::TailProduce::StreamsRegistry& registry() const { return registry_; }

#define TAILPRODUCE_STREAM(MANAGER, NAME, ENTRY_TYPE, ORDER_KEY_TYPE) \
        struct NAME##_type { \
            typedef ENTRY_TYPE entry_type; \
            typedef ORDER_KEY_TYPE order_key_type; \
            typedef ::TailProduce::StreamInstance<entry_type, order_key_type> stream_type; \
            typedef typename captured_base::storage_type storage_type; \
            typedef ::TailProduce::UnsafeListener<NAME##_type> unsafe_listener_type; \
            typedef ::TailProduce::UnsafePublisher<NAME##_type> unsafe_publisher_type; \
            typedef std::pair<order_key_type, uint32_t> head_pair_type; \
            typedef ::TailProduce::StorageKeyBuilder<NAME##_type> key_builder_type; \
            MANAGER* manager; \
            stream_type stream; \
            const std::string name; \
            key_builder_type key_builder; \
            head_pair_type head; \
            NAME##_type( \
                MANAGER* manager, \
                const char* stream_name, \
                const char* entry_type_name, \
                const char* entry_order_key_name) \
              : manager(manager), \
                stream(manager->registry_, stream_name, entry_type_name, entry_order_key_name), \
                name(stream_name), \
                key_builder(name), \
                head(::TailProduce::StreamManager::template FetchHeadOrDie<order_key_type, key_builder_type, storage_type>(name, key_builder, manager->storage)) { \
            } \
        }; \
        NAME##_type NAME = NAME##_type(this, #NAME, #ENTRY_TYPE, #ORDER_KEY_TYPE)

#define TAILPRODUCE_STATIC_FRAMEWORK_END() \
    }

#endif  // TAILPRODUCE_H
