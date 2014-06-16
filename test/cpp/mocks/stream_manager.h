#ifndef TAILPRODUCE_MOCKS_STREAM_MANAGER_H
#define TAILPRODUCE_MOCKS_STREAM_MANAGER_H

#include <vector>
#include <map>

#include <gtest/gtest.h>

// TODO(dkorolev): Mock stream manager implementation should inherit from its base class.
// TODO(dkorolev): Entry implementations should inherit from their base class.
// TODO(dkorolev): Current serialization is a prototype, move to a more robust version.

// MockStreamManager supports the following functionality:
// 1) Integration with Entry classes and their respective order keys getters.
// 2) Create streams and maintain the list of existing streams.
// 3) Keep track of the most recent order key per each stream.
// 4) Ensure the streams have monotonically increasing order keys. This requires the use of secondary keys.
// 5) Run TailProduce jobs. Support replaying entries from the past.
// 6) Ensure outputs enties from before the cutoff point are discarded.
// 7) Ensure that if data is attempted to be over-written, its consistency is verified and no overwrites take place.
// 8) Support merging multiple streams.

template<typename T_DATA_STORAGE> class MockStreamManager {
  public:
    // TODO(dkorolev): Support the functionality of eight bullet points above.

  private:
    T_DATA_STORAGE storage_;
};

#endif  // TAILPRODUCE_MOCKS_STREAM_MANAGER_H
