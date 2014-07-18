#ifndef STREAM_PERSIST_H
#define STREAM_PERSIST_H

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>

namespace TailProduce {
    struct StreamPersist  {
        StreamPersist() = default;
        StreamPersist(std::string const& streamName, 
                      std::string const& entryTypeName, 
                      std::string const& orderKeyTypeName) :
            stream_name(streamName), 
            entry_type_name(entryTypeName),
            order_key_type_name(orderKeyTypeName)
        {}

        std::string stream_name;
        std::string entry_type_name;
        std::string order_key_type_name;
    private:
        friend class cereal::access;
        template<class Archive>
        void serialize(Archive& ar)
        {
            ar(CEREAL_NVP(stream_name), CEREAL_NVP(entry_type_name), CEREAL_NVP(order_key_type_name));
        }
    };
};

#endif
