// Defines the schema for storage keys.

#ifndef TAILPRODUCE_KEY_SCHEMA_H
#define TAILPRODUCE_KEY_SCHEMA_H

#include <vector>

#include "bytes.h"

namespace TailProduce {
    struct KeySchema {
        static std::vector<uint8_t> StreamMeta(const std::string& stream) {
            return bytes("s\0" + stream);
        }
    };
}  // namespace TailProduce

#endif  // TAILPRODUCE_KEY_SCHEMA_H
