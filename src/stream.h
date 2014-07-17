#ifndef STREAM_H
#define STREAM_H

#include <string>
#include "streams_registry.h"
#include "config_values.h"

namespace TailProduce {
    // The stream base is only a syntatic convenience.
    struct StreamBase {
        StreamBase(std::string const& streamName) : streamName_(streamName) {}
        std::string const& GetId() { return streamName_; }
    private:
        std::string streamName_;
    };

    template <typename ORDER_KEY>
    struct Stream : StreamBase {
        Stream(StreamsRegistry& registry,
               ConfigValues &cv,
               const std::string& stream_name,
               const std::string& entry_type_name,
               const std::string& order_key_type_name) : 
            StreamBase(stream_name),
            orderKey_(stream_name, cv) {
            registry.Add(this, stream_name, entry_type_name, order_key_type_name);
        }
        ORDER_KEY &GetOrderKey() { return orderKey_; }
    private:
        ORDER_KEY orderKey_;
    };

};

#endif
