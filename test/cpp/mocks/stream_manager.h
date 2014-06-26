#ifndef TAILPRODUCE_MOCKS_STREAM_MANAGER_H
#define TAILPRODUCE_MOCKS_STREAM_MANAGER_H

#include <vector>
#include <map>

#include <gtest/gtest.h>
#include <glog/logging.h>

#include "../../../src/tailproduce.h"
#include "../../../src/helpers.h"
#include "../../../src/key_schema.h"

// TODO(dkorolev): Mock stream manager implementation should inherit from its base class.

// MockStreamManager supports the following functionality:
// 1) Integration with Entry classes and their respective order keys getters.
// 2) Create streams and maintain the list of existing streams.
// 3) Keep track of the most recent order key per each stream.
// 4) Ensure the streams have monotonically increasing order keys. This requires the use of secondary keys.
// 5) Run TailProduce jobs. Support replaying entries from the past.
// 6) Ensure outputs enties from before the cutoff point are discarded.
// 7) Ensure that if data is attempted to be over-written, its consistency is verified and no overwrites take place.
// 8) Support merging multiple streams.

template<typename T_DATA_STORAGE> class MockStreamManager : public TailProduce::StreamManager {
  public:
    // TODO(dkorolev): Support the functionality of eight bullet points above.

    /*
    void CreateStream(const std::string& stream_name) {
        const std::vector<uint8_t> key(TailProduce::KeySchema::StreamMeta(stream_name));
        if (!storage_.Get(key).empty()) {
            LOG(FATAL)
                << "Attempted to create stream '" << stream_name << "'"
                << ", that already exists.";
        }
        storage_.Set(key, bytes(true));
    }

    bool HasStream(const std::string& stream_name) const {
        return !storage_.Get(TailProduce::KeySchema::StreamMeta(stream_name)).empty();
    }

//    const T_DATA_STORAGE& 
    */

  private:
    T_DATA_STORAGE storage_;
};

#endif  // TAILPRODUCE_MOCKS_STREAM_MANAGER_H
