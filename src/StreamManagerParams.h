#ifndef STREAMMANAGERPARAMS_H
#define STREAMMANAGERPARAMS_H

#include <map>
#include <vector>
#include <utility>
#include "helpers.h"

namespace TailProduce {
    class StreamManagerParams {
      public:
        // Using default StreamManagerParams requires non-header-only usecase,
        // since command line flags must be defined in a single compile unit.
        // NOTE: This method is not implemented yet, and this is on purpose.
        //       The default construct of static framework, that uses is, should be avoided, at least in the tests.
        // TODO(dkorolev): Implement it.
        static StreamManagerParams FromCommandLineFlags();

        template<typename T_ORDER_KEY>
        StreamManagerParams&
        CreateStream(const std::string& name, const T_ORDER_KEY& key, const uint32_t secondary_key = 0) {
            auto& placeholder = init[name];
            if (!placeholder.empty()) {
                // TODO(dkorolev): Add a test for it.
                VLOG(3) << "throw StreamAlreadyListedForCreationException();";
                throw StreamAlreadyListedForCreationException();
            }
            ::TailProduce::Storage::KEY_TYPE kkey = OrderKey::template StaticSerializeAsStorageKey<T_ORDER_KEY>(key, secondary_key);
            std::copy(kkey.begin(), kkey.end(), std::back_inserter(placeholder));
            VLOG(3) << "Registering stream '" << name << "' as '" << kkey << "'.";
            return *this;
        }

        template<typename T_STORAGE> void Apply(T_STORAGE& storage) const {
            for (auto cit : init) {
                VLOG(3) << "Populating stream '" << cit.first << "' to the storage.";
                try {
                    storage.Set("s:" + cit.first, cit.second);
                } catch(const StorageOverwriteNotAllowedException&) {
                    VLOG(3) << "throw StreamAlreadyExistsException();";
                    throw StreamAlreadyExistsException();
                }
            }
        }

     private:
       // TODO(dkorolev): Fix this hack.
        std::map<::TailProduce::Storage::KEY_TYPE, ::TailProduce::Storage::VALUE_TYPE> init;
    };
};
#endif
