#ifndef TAILPRODUCE_H
#define TAILPRODUCE_H

#include <vector>
#include <set>
#include <string>

#include <glog/logging.h>

namespace TailProduce {
    class Stream;

    class StreamsRegistry {
      public:
        struct StreamsRegistryEntry {
            // The pointer to an instance of the stream is owner by TailProduce framework.
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
    struct OrderKey {};

    // TODO(dkorolev): Do ensure that user-defined objects inherit from the right base classes.
    template<typename T_ENTRY, typename T_ORDER_KEY> class StreamInstance : public TailProduce::Stream {
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
        }
    };
};

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

#endif  // TAILPRODUCE_H
