#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "dispatcher.h"

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
    template <typename BASE_TYPE, typename... TYPES> struct PolymorphicCerealJSONSerializable {
        struct SerializerImpl {
            explicit SerializerImpl(std::ostream& os) : os_(os) {
            }
            template <typename T> void operator()(const T& entry) {
                std::shared_ptr<BASE_TYPE> p_entry(new T(entry));  // TODO(dkorolev): Eliminate this copy!
                (cereal::JSONOutputArchive(os_))(p_entry);
                os_ << std::endl;
            }
            std::ostream& os_;
        };
        static void SerializeEntry(std::ostream& os, const BASE_TYPE& entry) {
            RuntimeDispatcher<BASE_TYPE, TYPES...>::DispatchCall(entry, SerializerImpl(os));
        }

        template <typename T_PROCESSOR>
        static void DeSerializeAndProcessEntry(std::istream& is, T_PROCESSOR& processor) {
            BASE_TYPE entry;
            cereal::JSONInputArchive ar(is);
            ar(entry);
            processor(entry);
        }
    };
};

#endif
