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

        template <typename PRIMARY_KEY, typename PROCESSOR>
        static void DeSerializeAndProcessEntry(std::istream& is,
                                               const PRIMARY_KEY& order_key,
                                               PROCESSOR& processor) {
            T_ENTRY entry;
            cereal::JSONInputArchive ar(is);
            try {
                ar(entry);
            } catch (cereal::Exception& e) {
                throw CerealDeSerializeException();
            }
            entry.SetOrderKey(order_key);
            processor(entry);
        }
    };
    // TODO(dkorolev): This copy-pasted code for Binary vs. JSON is worth eliminating some day.
    template <typename ENTRY> struct CerealBinarySerializable {
        typedef ENTRY T_ENTRY;
        static void SerializeEntry(std::ostream& os, const T_ENTRY& entry) {
            (cereal::BinaryOutputArchive(os))(entry);
        }

        template <typename PRIMARY_KEY, typename PROCESSOR>
        static void DeSerializeAndProcessEntry(std::istream& is,
                                               const PRIMARY_KEY& order_key,
                                               PROCESSOR& processor) {
            T_ENTRY entry;
            cereal::BinaryInputArchive ar(is);
            try {
                ar(entry);
            } catch (cereal::Exception& e) {
                throw CerealDeSerializeException();
            }
            entry.SetOrderKey(order_key);
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
    template <typename BASE_TYPE, typename PRIMARY_KEY, typename PROCESSOR> struct DeSerializerImplJSON {
        typedef BASE_TYPE T_BASE_TYPE;
        typedef PRIMARY_KEY T_PRIMARY_KEY;
        typedef PROCESSOR T_PROCESSOR;
        explicit DeSerializerImplJSON(const T_PRIMARY_KEY& order_key, T_PROCESSOR& processor)
            : order_key_(order_key), processor_(processor) {
        }
        template <typename ENTRY> void operator()(ENTRY& entry) {
            entry.SetOrderKey(order_key_);
            processor_(entry);
        }
        const T_PRIMARY_KEY& order_key_;
        T_PROCESSOR& processor_;
    };
    template <typename BASE_TYPE, typename... TYPES> struct PolymorphicCerealJSONSerializable {
        typedef BASE_TYPE T_BASE_TYPE;
        static void SerializeEntry(std::ostream& os, const T_BASE_TYPE& entry) {
            RuntimeDispatcher<T_BASE_TYPE, TYPES...>::DispatchCall(entry, SerializerImplJSON<T_BASE_TYPE>(os));
        }

        template <typename PRIMARY_KEY, typename PROCESSOR>
        static void DeSerializeAndProcessEntry(std::istream& is,
                                               const PRIMARY_KEY& order_key,
                                               PROCESSOR& processor) {
            std::shared_ptr<T_BASE_TYPE> p_entry;
            cereal::JSONInputArchive ar(is);
            try {
                ar(p_entry);
            } catch (cereal::Exception& e) {
                p_entry.reset();
                throw CerealDeSerializeException();
            }
            if (!p_entry.get()) {
                throw UnrecognizedPolymorphicType();
            } else {
                RuntimeDispatcher<T_BASE_TYPE, TYPES...>::DispatchCall(
                    *p_entry.get(), DeSerializerImplJSON<T_BASE_TYPE, PRIMARY_KEY, PROCESSOR>(order_key, processor));
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
    template <typename BASE_TYPE, typename PRIMARY_KEY, typename PROCESSOR> struct DeSerializerImplBinary {
        typedef BASE_TYPE T_BASE_TYPE;
        typedef PRIMARY_KEY T_PRIMARY_KEY;
        typedef PROCESSOR T_PROCESSOR;
        explicit DeSerializerImplBinary(const T_PRIMARY_KEY& order_key, T_PROCESSOR& processor)
            : order_key_(order_key), processor_(processor) {
        }
        template <typename ENTRY> void operator()(ENTRY& entry) {
            entry.SetOrderKey(order_key_);
            processor_(entry);
        }
        const T_PRIMARY_KEY& order_key_;
        T_PROCESSOR& processor_;
    };
    template <typename BASE_TYPE, typename... TYPES> struct PolymorphicCerealBinarySerializable {
        typedef BASE_TYPE T_BASE_TYPE;
        static void SerializeEntry(std::ostream& os, const T_BASE_TYPE& entry) {
            RuntimeDispatcher<T_BASE_TYPE, TYPES...>::DispatchCall(entry, SerializerImplBinary<T_BASE_TYPE>(os));
        }

        template <typename PRIMARY_KEY, typename PROCESSOR>
        static void DeSerializeAndProcessEntry(std::istream& is,
                                               const PRIMARY_KEY& order_key,
                                               PROCESSOR& processor) {
            std::shared_ptr<T_BASE_TYPE> p_entry;
            cereal::BinaryInputArchive ar(is);
            try {
                ar(p_entry);
            } catch (cereal::Exception& e) {
                p_entry.reset();
                throw CerealDeSerializeException();
            }
            if (!p_entry.get()) {
                throw UnrecognizedPolymorphicType();
            } else {
                RuntimeDispatcher<T_BASE_TYPE, TYPES...>::DispatchCall(
                    *p_entry.get(),
                    DeSerializerImplBinary<T_BASE_TYPE, PRIMARY_KEY, PROCESSOR>(order_key, processor));
            }
        }
    };
};

#endif
