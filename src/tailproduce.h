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
    struct Entry {
       // template<typename T_ORDER_KEY> T_ORDER_KEY GetOrderKey() const;
       // template<typename T> static void SerializeEntry(std::ostream& os, const T& entry);
       // template<typename T> static void DeSerializeEntry(std::istream& is, T& entry);
    };

    // An interface to extract order keys in certain types. With fixed-size serialization.
    struct OrderKey {
        // enum { size_in_bytes = 0 };  // TO GO AWAY -- D.K.
        // bool operator<(const T& rhs) const;
        // void SerializeOrderKey(uint8_t* ptr) const;
        // void DeSerializeOrderKey(const uint8_t* ptr);
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
            static_assert(std::is_base_of<TE, T_ENTRY>::value, "StreamManager::T_ENTRY should be derived from Entry.");
            using TOK = ::TailProduce::OrderKey;
            static_assert(std::is_base_of<TOK, T_ORDER_KEY>::value, "StreamManager::T_ORDER_KEY should be derived from OrderKey.");
            static_assert(T_ORDER_KEY::size_in_bytes > 0, "StreamManager::T_ORDER_KEY::size_in_bytes should be positive.");
        }
    };

    struct StreamManager {};

    struct Exception : std::exception {};
    struct OrderKeysGoBackwardsException : Exception {};
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
