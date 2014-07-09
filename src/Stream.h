#ifndef STREAMSREGISTRY_H
#define STREAMSREGISTRY_H

#include <string>
#include "StreamsRegistry.h"

namespace TailProduce {

    struct Stream {
        Stream(StreamsRegistry& registry,
               const std::string& stream_name,
               const std::string& entry_type_name,
               const std::string& order_key_type_name) {
            registry.Add(this, stream_name, entry_type_name, order_key_type_name);
        }
    };

};

#endif
