#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "cereal/archives/json.hpp"

namespace TailProduce {
    // Cereal-based serialization.
    template <typename T> struct CerealJSONSerializable {
        static void SerializeEntry(std::ostream& os, const T& entry) {
            (cereal::JSONOutputArchive(os))(entry);
            os << std::endl;
        }
        template <typename T_PROCESSOR> static void DeSerializeAndProcessEntry(std::istream& is, T_PROCESSOR& processor) {
            T entry;
            cereal::JSONInputArchive ar(is);
            ar(entry);
            processor(entry);
        }
    };
};

#endif
