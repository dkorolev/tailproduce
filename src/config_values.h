#ifndef CONFIG_VALUES_H
#define CONFIG_VALUES_H

#include <string>

#include "storage.h"

namespace TailProduce {
    struct ConfigValues {
        ConfigValues(std::string const& sap,
                     std::string const& sdp,
                     std::string const& srl,
                     std::string const& slkl,
                     char c)
            : streamsAdminPrefix(sap),
              streamsDataPrefix(sdp),
              streamsRegisterLocator(srl),
              streamsLastKeyLocator(slkl),
              connector(c) {
        }

        // TODO(dkorolev): Connector -> Separator or Delimeter?
        char GetConnector() const {
            return connector;
        }

        template <typename T_TRAITS>
        ::TailProduce::Storage::STORAGE_KEY_TYPE HeadStorageKey(const T_TRAITS& traits) const {
            return streamsAdminPrefix + connector + traits.name;
        }

        template <typename ORDER_KEY>::TailProduce::Storage::STORAGE_KEY_TYPE EndDataStorageKey() const {
            // TODO(dkorolev): Fix this.
            return "\xff";
        }

        std::string GetStreamDataPrefix(std::string const& streamId) const {
            return streamsDataPrefix + connector + streamId + connector;
        }

        std::string GetStreamMetaPrefix(std::string const& streamId) const {
            return streamsAdminPrefix + connector + streamId + connector;
        }

        /*
        // TODO(dkorolev): Start uncommenting these one by one.

        std::string GetStreamsRegister(std::string const& streamId) const {
            return streamsAdminPrefix + connector + streamsRegisterLocator + connector + streamId;
        }
        std::string GetStreamsLastKey(std::string const& streamId) const {
            return streamsAdminPrefix + connector + streamsLastKeyLocator + connector + streamId;
        }
        */

      private:
        // these are private to prevent anyone from accidentally changing them once thay are set
        const std::string streamsAdminPrefix;      // prefix used for streams administration
        const std::string streamsDataPrefix;       // prefix used for stream data writes
        const std::string streamsRegisterLocator;  // this is the locator (identifier) for the registered streams
        const std::string streamsLastKeyLocator;   // this is the locator key for the last key per stream
        const char connector;
    };
};

#endif
