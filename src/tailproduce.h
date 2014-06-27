// TODO(dkorolev): Perhaps we should move StreamsRegistry to become the part of the StreamManager base class?

#ifndef TAILPRODUCE_H
#define TAILPRODUCE_H

#include <vector>
#include <set>
#include <string>
#include <type_traits>
#include <mutex>
#include <sstream>

#include <glog/logging.h>

#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"

#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/map.hpp"

#include "cereal/types/polymorphic.hpp"

#include "helpers.h"

namespace TailProduce {
    class Stream;

    class StreamsRegistry {
      public:
        struct StreamsRegistryEntry {
            // The pointer to an instance of the stream is owned by TailProduce framework.
            // * For static frameworks (streams list is fully available at compile time),
            //   these pointers point to existing, statically initialized, members
            //   of the instance of cover TailProduce class.
            // * For dynamic frameworks, they point to dynamically allocated instances
            //   of per-stream implementations, which are also owned by the instance
            //   of the cover TailProduce class.
            const Stream* impl;
            std::string name;
            std::string entry_type;
            std::string order_key_type;
        };
        std::vector<StreamsRegistryEntry> streams;
        std::set<std::string> names;

        void Add(TailProduce::Stream* impl,
                 const std::string& name,
                 const std::string& entry_type,
                 const std::string& order_key_type) {
            if (names.find(name) != names.end()) {
                LOG(FATAL) << "Attempted to register the '" << name << "' stream more than once.";
            }
            names.insert(name);
            streams.push_back(StreamsRegistryEntry{impl, name, entry_type, order_key_type});
        }
    };

    struct Stream {
        Stream(StreamsRegistry& registry,
               const std::string& stream_name,
               const std::string& entry_type_name,
               const std::string& order_key_type_name) {
            registry.Add(this, stream_name, entry_type_name, order_key_type_name);
        }
    };

    // A serializable entry.
    template<typename T1, typename T2> struct OrderKeyExtractorImpl {};
    struct Entry {
        // Need the following fully specialized template within namespace ::TailProduce:
        //     template<> struct OrderKeyExtractorImpl<OrderKeyType, EntryType> {
        //         static OrderKeyType ExtractOrderKey(const EntryType& entry) { ... }
        //      };
        // for each desired pair of { OrderKeyType, EntryType }.
        // template<typename T> static void SerializeEntry(std::ostream& os, const T& entry);
        // template<typename T> static void DeSerializeEntry(std::istream& is, T& entry);
    };

    // An interface to extract order keys in certain types. With fixed-size serialization.
    struct OrderKey {
        // enum { size_in_bytes = 0 };  // TO GO AWAY -- D.K.
        // bool operator<(const T& rhs) const;
        // void SerializeOrderKey(uint8_t* ptr) const;
        // void DeSerializeOrderKey(const uint8_t* ptr);
        template<typename T_ORDER_KEY> static void StaticAppendAsStorageKey(const T_ORDER_KEY& primary_key,
                                                                            const uint32_t secondary_key,
                                                                            std::vector<uint8_t>& output) {
            using TOK = ::TailProduce::OrderKey;
            static_assert(std::is_base_of<TOK, T_ORDER_KEY>::value, "StreamManager::T_ORDER_KEY should be derived from OrderKey.");
            static_assert(T_ORDER_KEY::size_in_bytes > 0, "StreamManager::T_ORDER_KEY::size_in_bytes should be positive.");
            uint8_t result[T_ORDER_KEY::size_in_bytes + 1 + 11];
            primary_key.SerializeOrderKey(result);
            result[T_ORDER_KEY::size_in_bytes] = ':';
            snprintf(reinterpret_cast<char*>(result + T_ORDER_KEY::size_in_bytes + 1), 11, "%010u", secondary_key);
            std::copy(result, result + sizeof(result) - 1, std::back_inserter(output));
        }
        template<typename T_ORDER_KEY> static std::vector<uint8_t> StaticSerializeAsStorageKey(const T_ORDER_KEY& primary_key,
                                                                                               const uint32_t secondary_key) {
            std::vector<uint8_t> output;
            StaticAppendAsStorageKey<T_ORDER_KEY>(primary_key, secondary_key, output);
            return output;
        }
    };
    struct Storage {};   // Data storage proxy, originally LevelDB.
    struct Producer {};  // Client-defined job.

    // Cereal-based serialization.
    template<typename T> struct CerealJSONSerializable {
       static void SerializeEntry(std::ostream& os, const T& entry) {
           (cereal::JSONOutputArchive(os))(entry);
           os << std::endl;
       }
       static void DeSerializeEntry(std::istream& is, T& entry) {
           cereal::JSONInputArchive ar(is);
           ar(entry);
       }
    };

    template<typename T_ENTRY, typename T_ORDER_KEY> class StreamInstance : public Stream {
      public:
        typedef T_ENTRY ENTRY_TYPE;
        typedef T_ORDER_KEY ORDER_KEY_TYPE;
        StreamInstance(
            TailProduce::StreamsRegistry& registry,
            const std::string& stream_name,
            const std::string& entry_type_name,
            const std::string& order_key_type_name)
            : TailProduce::Stream(registry,
                                  stream_name,
                                  entry_type_name,
                                  order_key_type_name) {
            using TE = ::TailProduce::Entry;
            static_assert(std::is_base_of<TE, T_ENTRY>::value, "StreamInstance::T_ENTRY should be derived from Entry.");
            using TOK = ::TailProduce::OrderKey;
            static_assert(std::is_base_of<TOK, T_ORDER_KEY>::value, "StreamInstance::T_ORDER_KEY should be derived from OrderKey.");
            static_assert(T_ORDER_KEY::size_in_bytes > 0, "StreamInstance::T_ORDER_KEY::size_in_bytes should be positive.");
        }
    };

    struct StreamManager {};

    struct Exception : std::exception {};
    struct OrderKeysGoBackwardsException : Exception {};

    // UnsafeListener contains the logic of creating and re-creating storage-level read iterators,
    // presenting data in serialized format and keeping track of HEAD order keys.
    template<typename T> struct UnsafeListener {
        UnsafeListener() = delete;

        explicit UnsafeListener(const T& stream) : stream(stream) {
        }
        UnsafeListener(const T& stream, const typename T::order_key_type& begin, const typename T::order_key_type& end) : stream(stream) {
            //qwerty
        }
        UnsafeListener(UnsafeListener&&) = default;
        
        const typename T::head_pair_type& GetHead() const {
            return stream.head;
        }

        // Returns true if more data is available.
        // Can change from false to true if/when new data is available.
        bool HasData() const {
            return false;
        }

      private:
        UnsafeListener(const UnsafeListener&) = delete;
        void operator=(const UnsafeListener&) = delete;
        const T& stream;
    };

    // UnsafePublisher contains the logic of appending data to the streams and updating their HEAD order keys.
    template<typename T> struct UnsafePublisher {
        UnsafePublisher() = delete;
        explicit UnsafePublisher(T& stream) : stream(stream) {
        }

        UnsafePublisher(T& stream, const typename T::order_key_type& order_key) : stream(stream) {
            PushHead(order_key);
        }

        void Push(const typename T::entry_type& entry) {
            PushHead(::TailProduce::OrderKeyExtractorImpl<typename T::order_key_type, typename T::entry_type>::ExtractOrderKey(entry));
            std::vector<uint8_t> key = bytes("d:" + stream.name + ":");
            OrderKey::template StaticAppendAsStorageKey<typename T::order_key_type>(stream.head.first, stream.head.second, key);
            std::ostringstream value_output_stream;
            T::entry_type::SerializeEntry(value_output_stream, entry);
            stream.manager->storage.Set(key, bytes(value_output_stream.str()));
        }

        void PushHead(const typename T::order_key_type& order_key) {
            typename T::head_pair_type new_head(order_key, 0);
            if (new_head < stream.head) {
                // Order keys should only be increasing.
                throw ::TailProduce::OrderKeysGoBackwardsException();
            }
            if (!(stream.head < new_head)) {
                new_head.second = stream.head.second + 1;
            }
            // TODO(dkorolev): Perhaps more checks here?
            stream.manager->storage.SetAllowingOverwrite(
                stream.head_storage_key,
                OrderKey::template StaticSerializeAsStorageKey<typename T::order_key_type>(new_head.first,
                                                                                           new_head.second));
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

/*
// TailProduce static framework macros.
// Static framework is the one that lists all the streams in the source file,
// thus allowing all C++ template and static typing powers to come into play.
#define TAILPRODUCE_STATIC_FRAMEWORK_BEGIN(name, base) \
    class name : public base { \
      private: \
        ::TailProduce::StreamsRegistry registry_;  \
      public: \
        const ::TailProduce::StreamsRegistry& registry() const { return registry_; }

#define TAILPRODUCE_STREAM(name, entry_type, order_key_type) \
        typedef ::TailProduce::StreamInstance<entry_type, order_key_type> STREAM_TYPE_##name; \
        STREAM_TYPE_##name name = STREAM_TYPE_##name(registry_, #name, #entry_type, #order_key_type)

#define TAILPRODUCE_STATIC_FRAMEWORK_END() \
    }
*/

#endif  // TAILPRODUCE_H
