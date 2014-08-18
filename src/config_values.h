#ifndef CONFIG_VALUES_H
#define CONFIG_VALUES_H

#include <cassert>
#include <string>

#include "storage.h"

namespace TailProduce {
    struct ConfigValues {
        ConfigValues(std::string const& stream_meta_prefix, std::string const& stream_data_prefix, char delimeter)
            : stream_meta_prefix_(stream_meta_prefix),
              stream_data_prefix_(stream_data_prefix),
              delimeter_(delimeter) {
        }

        char GetDelimeter() const {
            return delimeter_;
        }

        template <typename STREAM_TRAITS>
        ::TailProduce::Storage::STORAGE_KEY_TYPE HeadStorageKey(const STREAM_TRAITS& traits) const {
            return stream_meta_prefix_ + delimeter_ + traits.name;
        }

        template <typename STREAM_TRAITS>
        ::TailProduce::Storage::STORAGE_KEY_TYPE EndDataStorageKey(const STREAM_TRAITS& traits) const {
            // Guard against the case where delimeter_ is '\xff'.
            assert(static_cast<uint8_t>(delimeter_ + 1) > static_cast<uint8_t>(delimeter_));
            return stream_data_prefix_ + delimeter_ + traits.name + static_cast<char>(delimeter_ + 1);
        }

        template <typename STREAM_TRAITS> std::string GetStreamDataPrefix(const STREAM_TRAITS& traits) const {
            return stream_data_prefix_ + delimeter_ + traits.name + delimeter_;
        }

        template <typename STREAM_TRAITS> std::string GetStreamMetaPrefix(const STREAM_TRAITS& traits) const {
            return stream_meta_prefix_ + delimeter_ + traits.name + delimeter_;
        }

      private:
        const std::string stream_meta_prefix_;
        const std::string stream_data_prefix_;
        const char delimeter_;
    };
};

#endif
