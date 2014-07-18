#ifndef TAILPRODUCE_UTILS_H
#define TAILPRODUCE_UTILS_H

#include <string>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <utility>

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>

#include "helpers.h"
#include "stream.h"
#include "dbm_iterator.h"
#include "config_values.h"
#include "stream_persist.h"

namespace TailProduce {
    // This block of code is used to create a stream and a producer pair from the stream persist objects in the db
    template<typename ORDER_KEY, typename STORAGE>
    auto 
    RestorePersistedStreams(TailProduce::StreamsRegistry& registry,
                            STORAGE& storage, 
                            TailProduce::ConfigValues const& cv) -> 
        std::unordered_map< std::string, std::shared_ptr<TailProduce::Stream<ORDER_KEY>>>
    {
        typedef TailProduce::Stream<ORDER_KEY> STREAM;
        typedef std::shared_ptr<STREAM> STREAM_PTR;

        std::unordered_map<std::string, STREAM_PTR> results;
        auto knownStreamsKey = cv.GetStreamsRegister(""); // passing an empty string will allow creating an iterator of all known streams
        auto iterator = storage.GetIterator(knownStreamsKey);
        while(!iterator.Done()) {
            std::string streamValues = antibytes(iterator.Value());
            TailProduce::StreamPersist persisted;
            std::istringstream is(streamValues);  // make the string a stream
            cereal::JSONInputArchive ar(is);      // make a JSON Input Archive from the string
            ar(persisted);                        // populate Persisted
            auto stream = STREAM_PTR(new STREAM(registry,
                                                cv,
                                                persisted.stream_name,
                                                persisted.entry_type_name,
                                                persisted.order_key_type_name));
            
            
            results.insert(std::make_pair(stream->GetId(), stream));
            iterator.Next();
        }
        return results;
    }

    template<typename STORAGE>
    void
    PersistStream(STORAGE storage, 
                  TailProduce::StreamPersist &sp,
                  TailProduce::ConfigValues& cv)
    {
        // The reverse of this is to store the known streams in the DB.
        std::ostringstream os;
        (cereal::JSONOutputArchive(os))(sp);
        std::string objStr = os.str();
        TailProduce::Storage::KEY_TYPE skey = cv.GetStreamsRegister(sp.stream_name);
        storage.AdminSet(skey, TailProduce::bytes(objStr));
    }

};
#endif
