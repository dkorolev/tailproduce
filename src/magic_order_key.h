#ifndef TAILPRODUCE_MAGIC_ORDER_KEY_H
#define TAILPRODUCE_MAGIC_ORDER_KEY_H

#include <string>

#include <glog/logging.h>

#include "storage.h"
#include "config_values.h"
#include "fixed_size_serializer.h"
#include "tp_exceptions.h"

namespace TailProduce {
    // MagicOrderKey has several purposes.
    //
    // 1) It contains the T_PRIMARY_KEY and T_SECONDARY_KEY types.
    //
    // 2) When instantiated, it contains the `primary` and `secondary` fields,
    //    essentially acting as an std::pair.
    //
    // 3) It is tied to a specific stream, by name.
    //    Therefore, once the type of a template-instantiated MagicOrderKey is known,
    //    no objects are required to tie it to the stream in order to compose/decompose keys.
    //
    //    All this work is done by a static methods, with no MagicOrderKey object required.
    //
    // 4) It is an entry point for the logic to compose/decompose keys, increment secondary key, etc.
    //
    // Note that 3) and 4)  would still need a const reference to ConfigValues -- D.K.

    struct MagicGenericOrderKey {};

    template <typename TRAITS, typename PRIMARY_KEY, typename SECONDARY_KEY>
    struct MagicOrderKey : MagicGenericOrderKey {
        typedef TRAITS T_TRAITS;
        typedef PRIMARY_KEY T_PRIMARY_KEY;
        typedef SECONDARY_KEY T_SECONDARY_KEY;

        T_PRIMARY_KEY primary;
        T_SECONDARY_KEY secondary;

        MagicOrderKey() = default;
        // TODO(dkorolev): Change 0 to a default value for the secondary key.
        explicit MagicOrderKey(const T_PRIMARY_KEY& p) : primary(p), secondary(0) {
        }
        MagicOrderKey(const T_PRIMARY_KEY& p, const T_SECONDARY_KEY& s) : primary(p), secondary(s) {
        }

        ::TailProduce::Storage::STORAGE_KEY_TYPE ComposeStorageKey(const T_TRAITS& traits,
                                                                   const ::TailProduce::ConfigValues& cv) const {
            return traits.storage_key_data_prefix +
                   ::TailProduce::FixedSizeSerializer<T_PRIMARY_KEY>::PackToString(primary) + cv.GetConnector() +
                   ::TailProduce::FixedSizeSerializer<T_SECONDARY_KEY>::PackToString(secondary);
        }

        void DecomposeStorageKey(const ::TailProduce::Storage::STORAGE_KEY_TYPE storage_key,
                                 const T_TRAITS& traits,
                                 const ::TailProduce::ConfigValues& cv) {
            // This code is platform-dependent.
            const std::string s = ::TailProduce::antibytes(storage_key);
            const size_t expected_length = traits.storage_key_data_prefix.length() +
                                           ::TailProduce::FixedSizeSerializer<T_PRIMARY_KEY>::size_in_bytes + 1 +
                                           ::TailProduce::FixedSizeSerializer<T_SECONDARY_KEY>::size_in_bytes;
            if (s.length() != expected_length) {
                VLOG(2) << "Malformed key: input length " << s.size() << ", expected length " << expected_length
                        << ".";
                VLOG(3) << "throw MalformedStorageHeadException();";
                throw ::TailProduce::MalformedStorageHeadException();
            } else {
                const char connector =
                    s[s.length() - 1 - ::TailProduce::FixedSizeSerializer<T_SECONDARY_KEY>::size_in_bytes];
                if (connector != cv.GetConnector()) {
                    VLOG(2) << "Malformed key: connecter is '" << connector << "', while expected '"
                            << cv.GetConnector() << "'.";
                    VLOG(3) << "throw MalformedStorageHeadException();";
                    throw ::TailProduce::MalformedStorageHeadException();
                } else {
                    ::TailProduce::FixedSizeSerialization::UnpackFromString(
                        s.substr(traits.storage_key_data_prefix.length(),
                                 ::TailProduce::FixedSizeSerializer<T_PRIMARY_KEY>::size_in_bytes),
                        primary);
                    ::TailProduce::FixedSizeSerialization::UnpackFromString(
                        s.substr(traits.storage_key_data_prefix.length() + 1 +
                                     ::TailProduce::FixedSizeSerializer<T_PRIMARY_KEY>::size_in_bytes + 1,
                                 ::TailProduce::FixedSizeSerializer<T_SECONDARY_KEY>::size_in_bytes),
                        secondary);
                }
            }
        }
    };
};

#endif  // TAILPRODUCE_MAGIC_ORDER_KEY_H
