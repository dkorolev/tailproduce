#ifndef STREAMMANAGERPARAMS_H
#define STREAMMANAGERPARAMS_H

#include <map>
#include <vector>
#include <utility>
#include <memory>

#include <glog/logging.h>

#include "storage.h"
#include "bytes.h"
#include "tp_exceptions.h"
#include "config_values.h"
#include "magic_order_key.h"

namespace TailProduce {
    class StreamManagerParams {
      private:
        using STORAGE_KEY_TYPE = ::TailProduce::Storage::STORAGE_KEY_TYPE;
        using STORAGE_VALUE_TYPE = ::TailProduce::Storage::STORAGE_VALUE_TYPE;

        struct HeadInitializer {
            virtual STORAGE_KEY_TYPE ComposeStartingStorageKey(const std::string& name,
                                                               const ::TailProduce::ConfigValues& cv) = 0;
        };

        template <typename PRIMARY_ORDER_KEY, typename SECONDARY_ORDER_KEY>
        struct TypedHeadInitializer : HeadInitializer {
            PRIMARY_ORDER_KEY primary;
            SECONDARY_ORDER_KEY secondary;
            // TODO(dkorolev): This code should not be here. Especially the `0` part.
            TypedHeadInitializer(PRIMARY_ORDER_KEY primary, SECONDARY_ORDER_KEY secondary = 0)
                : primary(primary), secondary(secondary) {
            }
            virtual STORAGE_KEY_TYPE ComposeStartingStorageKey(const std::string& name,
                                                               const ::TailProduce::ConfigValues& cv) {
                struct Traits {
                    std::string name;
                    std::string storage_key_data_prefix;
                    Traits(const ::TailProduce::ConfigValues& cv, const std::string& name)
                        : name(name), storage_key_data_prefix(cv.GetStreamDataPrefix(*this)) {
                    }
                };
                Traits traits(cv, name);
                ::TailProduce::OrderKey<Traits, PRIMARY_ORDER_KEY, SECONDARY_ORDER_KEY> key;
                key.primary = primary;
                key.secondary = secondary;
                return key.ComposeStorageKey(traits, cv);
            }
        };

      public:
        // Using default StreamManagerParams requires non-header-only usecase,
        // since command line flags must be defined in a single compile unit.
        // NOTE: This method is not implemented yet, and this is on purpose.
        //       The default construct of static framework, that uses is, should be avoided, at least in the tests.
        // TODO(dkorolev): Implement it.
        static StreamManagerParams FromCommandLineFlags();

        // TODO(dkorolev): Important! Should not be able to CreateStream with wrong type!
        template <typename PRIMARY_ORDER_KEY, typename SECONDARY_ORDER_KEY>
        StreamManagerParams& CreateStream(const std::string& name,
                                          const PRIMARY_ORDER_KEY& primary_key,
                                          const SECONDARY_ORDER_KEY& secondary_key) {
            auto& placeholder = streams_to_create[name];
            if (placeholder) {
                // TODO(dkorolev): Add a test for it.
                VLOG(3) << "throw StreamAlreadyListedForCreationException();";
                throw StreamAlreadyListedForCreationException();
            }
            placeholder = std::shared_ptr<HeadInitializer>(
                new TypedHeadInitializer<PRIMARY_ORDER_KEY, SECONDARY_ORDER_KEY>(primary_key, secondary_key));
            VLOG(3) << "Registering stream '" << name << "'.";
            return *this;
        }

        template <typename T_STORAGE> void Apply(T_STORAGE& storage, const ::TailProduce::ConfigValues& cv) const {
            for (auto cit : streams_to_create) {
                VLOG(3) << "Populating stream '" << cit.first << "' to the storage.";
                try {
                    // TODO(dkorolev): Key generation logic should also go into ConfigValues.
                    storage.Set(
                        ::TailProduce::Storage::STORAGE_KEY_TYPE("s:" + cit.first),
                        ::TailProduce::Storage::KeyToValue(cit.second->ComposeStartingStorageKey(cit.first, cv)));
                } catch (const StorageOverwriteNotAllowedException&) {
                    VLOG(3) << "throw StreamAlreadyExistsException();";
                    throw StreamAlreadyExistsException();
                }
            }
        }

      private:
        std::map<std::string, std::shared_ptr<HeadInitializer>> streams_to_create;
    };
};

#endif
