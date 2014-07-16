#ifndef STREAMS_REGISTRY_H
#define STREAMS_REGISTRY_H

#include <string>
#include <vector>
#include <set>
#include <glog/logging.h>

namespace TailProduce {
    class StreamBase;
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
            TailProduce::StreamBase const* impl;
            std::string name;
            std::string entry_type;
            std::string order_key_type;
        };
        std::vector<StreamsRegistryEntry> streams;
        std::set<std::string> names;

        void Add(TailProduce::StreamBase* impl,
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
};

#endif
