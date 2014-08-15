#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "dispatcher.h"

#include "cereal/archives/json.hpp"
#include "cereal/archives/binary.hpp"

namespace TailProduce {
    // Cereal-based fixed type serialization.
    template <typename ENTRY> struct CerealJSONSerializable {
        typedef ENTRY T_ENTRY;
        static void SerializeEntry(std::ostream& os, const T_ENTRY& entry) {
            (cereal::JSONOutputArchive(os))(entry);
            os << std::endl;
        }

        template <typename PROCESSOR>
        static void DeSerializeAndProcessEntry(std::istream& is, PROCESSOR& processor) {
            T_ENTRY entry;
            cereal::JSONInputArchive ar(is);
            ar(entry);
            processor(entry);
        }
    };
    // TODO(dkorolev): This copy-pasted code for Binary vs. JSON is worth eliminating some day.
    template <typename ENTRY> struct CerealBinarySerializable {
        typedef ENTRY T_ENTRY;
        static void SerializeEntry(std::ostream& os, const T_ENTRY& entry) {
            (cereal::BinaryOutputArchive(os))(entry);
        }

        template <typename PROCESSOR>
        static void DeSerializeAndProcessEntry(std::istream& is, PROCESSOR& processor) {
            T_ENTRY entry;
            cereal::BinaryInputArchive ar(is);
            ar(entry);
            processor(entry);
        }
    };

    // Cereal-based polymorphic type serialization.
    template <typename BASE_TYPE> struct SerializerImplJSON {
        typedef BASE_TYPE T_BASE_TYPE;
        explicit SerializerImplJSON(std::ostream& os) : os_(os) {
        }
        template <typename ENTRY> void operator()(const ENTRY& entry) {
            std::shared_ptr<T_BASE_TYPE> p_entry(new ENTRY(entry));  // TODO(dkorolev): Eliminate this copy!
            (cereal::JSONOutputArchive(os_))(p_entry);
            os_ << std::endl;
        }
        std::ostream& os_;
    };
    template <typename BASE_TYPE, typename PROCESSOR> struct DeSerializerImplJSON {
        typedef BASE_TYPE T_BASE_TYPE;
        typedef PROCESSOR T_PROCESSOR;
        explicit DeSerializerImplJSON(T_PROCESSOR& processor) : processor_(processor) {
        }
        template <typename ENTRY> void operator()(const ENTRY& entry) {
            processor_(entry);
        }
        T_PROCESSOR& processor_;
    };
    template <typename BASE_TYPE, typename... TYPES> struct PolymorphicCerealJSONSerializable {
        typedef BASE_TYPE T_BASE_TYPE;
        static void SerializeEntry(std::ostream& os, const T_BASE_TYPE& entry) {
            RuntimeDispatcher<T_BASE_TYPE, TYPES...>::DispatchCall(entry, SerializerImplJSON<T_BASE_TYPE>(os));
        }

        template <typename PROCESSOR>
        static void DeSerializeAndProcessEntry(std::istream& is, PROCESSOR& processor) {
            std::shared_ptr<T_BASE_TYPE> p_entry;
            cereal::JSONInputArchive ar(is);
            ar(p_entry);
            if (!p_entry.get()) {
                throw UnrecognizedPolymorphicType();
            } else {
                RuntimeDispatcher<T_BASE_TYPE, TYPES...>::DispatchCall(
                    *p_entry.get(), DeSerializerImplJSON<T_BASE_TYPE, PROCESSOR>(processor));
            }
        }
    };
    // TODO(dkorolev): This copy-pasted code for Binary vs. JSON is worth eliminating some day.
    template <typename BASE_TYPE> struct SerializerImplBinary {
        typedef BASE_TYPE T_BASE_TYPE;
        explicit SerializerImplBinary(std::ostream& os) : os_(os) {
        }
        template <typename ENTRY> void operator()(const ENTRY& entry) {
            std::shared_ptr<T_BASE_TYPE> p_entry(new ENTRY(entry));  // TODO(dkorolev): Eliminate this copy!
            (cereal::BinaryOutputArchive(os_))(p_entry);
            os_ << std::endl;
        }
        std::ostream& os_;
    };
    template <typename BASE_TYPE, typename PROCESSOR> struct DeSerializerImplBinary {
        typedef BASE_TYPE T_BASE_TYPE;
        typedef PROCESSOR T_PROCESSOR;
        explicit DeSerializerImplBinary(T_PROCESSOR& processor) : processor_(processor) {
        }
        template <typename ENTRY> void operator()(const ENTRY& entry) {
            processor_(entry);
        }
        T_PROCESSOR& processor_;
    };
    template <typename BASE_TYPE, typename... TYPES> struct PolymorphicCerealBinarySerializable {
        typedef BASE_TYPE T_BASE_TYPE;
        static void SerializeEntry(std::ostream& os, const T_BASE_TYPE& entry) {
            RuntimeDispatcher<T_BASE_TYPE, TYPES...>::DispatchCall(entry, SerializerImplBinary<T_BASE_TYPE>(os));
        }

        template <typename PROCESSOR>
        static void DeSerializeAndProcessEntry(std::istream& is, PROCESSOR& processor) {
            std::shared_ptr<T_BASE_TYPE> p_entry;
            cereal::BinaryInputArchive ar(is);
            ar(p_entry);
            if (!p_entry.get()) {
                throw UnrecognizedPolymorphicType();
            } else {
                RuntimeDispatcher<T_BASE_TYPE, TYPES...>::DispatchCall(
                    *p_entry.get(), DeSerializerImplBinary<T_BASE_TYPE, PROCESSOR>(processor));
            }
        }
    };
};

#endif
