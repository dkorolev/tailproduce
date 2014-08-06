#ifndef STREAM_H
#define STREAM_H

#include <string>
#include <memory>
#include <mutex>

#include "config_values.h"

namespace TailProduce {
    // The stream base is only a syntatic convenience.
    struct StreamBase {
        StreamBase(std::string const& streamName) : streamName_(streamName), lock_mutex_(new std::mutex()) {
        }
        std::string const& GetId() {
            return streamName_;
        }
        std::mutex& lock_mutex() const {
            return *lock_mutex_;
        }

      private:
        std::string streamName_;
        mutable std::unique_ptr<std::mutex> lock_mutex_;
    };

    template <typename ORDER_KEY> struct Stream : StreamBase {
        Stream(ConfigValues const& cv,
               std::string const& stream_name,
               std::string const& entry_type_name,
               std::string const& order_key_type_name)
            : StreamBase(stream_name), orderKey_(stream_name, cv) {
        }

        ORDER_KEY& GetOrderKey() {
            return orderKey_;
        }

      private:
        ORDER_KEY orderKey_;
    };
};

#endif
