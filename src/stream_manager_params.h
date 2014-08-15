#ifndef STREAMMANAGERPARAMS_H
#define STREAMMANAGERPARAMS_H

#include <map>
#include <vector>
#include <utility>
#include <memory>

#include <glog/logging.h>

#include "storage.h"
#include "order_key.h"
#include "bytes.h"
#include "tp_exceptions.h"
#include "config_values.h"

namespace TailProduce {
    class StreamManagerParams {
      private:
        using STORAGE_KEY_TYPE = ::TailProduce::Storage::STORAGE_KEY_TYPE;
        using STORAGE_VALUE_TYPE = ::TailProduce::Storage::STORAGE_VALUE_TYPE;

        struct HeadInitializer {
            virtual STORAGE_KEY_TYPE ComposeStartingStorageKey(const ::TailProduce::ConfigValues& cv) = 0;
        };

        template <typename ORDER_KEY> struct TypedHeadInitializer : HeadInitializer {
            ORDER_KEY key;
            TypedHeadInitializer(ORDER_KEY key) : key(key) {
            }
            virtual STORAGE_KEY_TYPE ComposeStartingStorageKey(const ::TailProduce::ConfigValues& cv) {
                return key.ComposeStorageKey(cv);
            }
        };

      public:
        // Using default StreamManagerParams requires non-header-only usecase,
        // since command line flags must be defined in a single compile unit.
        // NOTE: This method is not implemented yet, and this is on purpose.
        //       The default construct of static framework, that uses is, should be avoided, at least in the tests.
        // TODO(dkorolev): Implement it.
        static StreamManagerParams FromCommandLineFlags();

        // TODO(dkorolev): Fix secondary key type and value here.
        // TODO(dkorolev): Important! Should not be able to CreateStream with wrong type!
        template <typename ORDER_KEY>
        StreamManagerParams& CreateStream(const std::string& name, const ORDER_KEY& key) {
            auto& placeholder = streams_to_create[name];
            if (placeholder) {
                // TODO(dkorolev): Add a test for it.
                VLOG(3) << "throw StreamAlreadyListedForCreationException();";
                throw StreamAlreadyListedForCreationException();
            }
            placeholder = std::shared_ptr<HeadInitializer>(new TypedHeadInitializer<ORDER_KEY>(key));
            VLOG(3) << "Registering stream '" << name << "'.";
            return *this;
        }

        template <typename PRIMARY_ORDER_KEY, typename SECONDARY_ORDER_KEY>
        StreamManagerParams& CreateStream(const std::string& name, const PRIMARY_ORDER_KEY& primary_key, const SECONDARY_ORDER_KEY& secondary_key) {
            auto& placeholder = streams_to_create[name];
            if (placeholder) {
                // TODO(dkorolev): Add a test for it.
                VLOG(3) << "throw StreamAlreadyListedForCreationException();";
                throw StreamAlreadyListedForCreationException();
            }
            placeholder = std::shared_ptr<HeadInitializer>(new TypedHeadInitializer<ORDER_KEY>(key));
            VLOG(3) << "Registering stream '" << name << "'.";
            return *this;
        }

        template <typename T_STORAGE> void Apply(T_STORAGE& storage, const ::TailProduce::ConfigValues& cv) const {
            for (auto cit : streams_to_create) {
                VLOG(3) << "Populating stream '" << cit.first << "' to the storage.";
                try {
                    // TODO(dkorolev): This logic should also go into ConfigValues.
                    // storage.Set("s:" + cit.first, cit.second);
                    storage.Set(::TailProduce::Storage::STORAGE_KEY_TYPE("s:" + cit.first),
                                ::TailProduce::Storage::KeyToValue(cit.second->ComposeStartingStorageKey(cv)));
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
