#ifndef CONFIG_VALUES_H
#define CONFIG_VALUES_H

#include <string>

namespace TailProduce {
    struct ConfigValues {
        ConfigValues(std::string const& sap,
                     std::string const& sdp,
                     std::string const& srl,
                     std::string const& slkl,
                     char c) :
            streamsAdminPrefix(sap),
            streamsDataPrefix(sdp),
            streamsRegisterLocator(srl),
            streamsLastKeyLocator(slkl),
            connector(c) {}

        char GetConnector() const { return connector; }
        
        std::string GetStreamsData(std::string const& streamId) const {
            return streamsDataPrefix + connector + streamId;
        }

        std::string GetStreamsRegister(std::string const& streamId) const {
            return streamsAdminPrefix + connector + streamsRegisterLocator + connector + streamId;
        }

        std::string GetStreamsLastKey(std::string const& streamId) const {
            return streamsAdminPrefix + connector + streamsLastKeyLocator + streamId;
        }
    private:
        // these are private to prevent anyone from accidentally changing them once thay are set
        std::string streamsAdminPrefix;  // prefix used for streams administration
        std::string streamsDataPrefix;   // prefix used for stream data writes
        std::string streamsRegisterLocator;  // this is the locator (identifier) for the registered streams
        std::string streamsLastKeyLocator;          // this is the locater key for the last key per stream
        char connector;
    };
};

#endif
