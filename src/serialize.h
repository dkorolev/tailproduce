#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "cereal/archives/json.hpp"

namespace TailProduce {
    // Cereal-based fixed type serialization.
    template <typename T> struct CerealJSONSerializable {
        static void SerializeEntry(std::ostream& os, const T& entry) {
            (cereal::JSONOutputArchive(os))(entry);
            os << std::endl;
        }
        template <typename T_PROCESSOR>
        static void DeSerializeAndProcessEntry(std::istream& is, T_PROCESSOR& processor) {
            T entry;
            cereal::JSONInputArchive ar(is);
            ar(entry);
            processor(entry);
        }
    };

    // Cereal-based polymorphic type serialization.
    template <typename T> struct PolymorphicCerealJSONSerializable {
        static void SerializeEntry(std::ostream& os, const T& entry) {
            std::shared_ptr<T> p_entry(new T(entry));  // TODO(dkorolev): Eliminate this copy!
            (cereal::JSONOutputArchive(os))(p_entry);
            os << std::endl;
        }
        template <typename T_PROCESSOR>
        static void DeSerializeAndProcessEntry(std::istream& is, T_PROCESSOR& processor) {
            T entry;
            cereal::JSONInputArchive ar(is);
            ar(entry);
            processor(entry);
        }
    };
};

#endif
