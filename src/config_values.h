#ifndef CONFIG_VALUES_H
#define CONFIG_VALUES_H

#include <cassert>
#include <string>

#include "storage.h"

namespace TailProduce {
    struct ConfigValues {
        ConfigValues(std::string const& stream_meta_prefix, std::string const& stream_data_prefix, char delimiter)
            : stream_meta_prefix_(stream_meta_prefix),
              stream_data_prefix_(stream_data_prefix),
              delimiter_(delimiter) {
        }

        // TODO(dkorolev): We have removed the delimiter between primary and secondary key components. Retire this?
        // char GetDelimiter() const {
        //     return delimiter_;
        // }

        template <typename STREAM_TRAITS>
        ::TailProduce::Storage::STORAGE_KEY_TYPE HeadStorageKey(const STREAM_TRAITS& traits) const {
            return stream_meta_prefix_ + delimiter_ + traits.name;
        }

        template <typename STREAM_TRAITS>
        ::TailProduce::Storage::STORAGE_KEY_TYPE EndDataStorageKey(const STREAM_TRAITS& traits) const {
            // Guard against the case where delimiter_ is '\xff'.
            assert(static_cast<uint8_t>(delimiter_ + 1) > static_cast<uint8_t>(delimiter_));
            return stream_data_prefix_ + delimiter_ + traits.name + static_cast<char>(delimiter_ + 1);
        }

        template <typename STREAM_TRAITS> std::string GetStreamDataPrefix(const STREAM_TRAITS& traits) const {
            return stream_data_prefix_ + delimiter_ + traits.name + delimiter_;
        }

        template <typename STREAM_TRAITS> std::string GetStreamMetaPrefix(const STREAM_TRAITS& traits) const {
            return stream_meta_prefix_ + delimiter_ + traits.name + delimiter_;
        }

      private:
        const std::string stream_meta_prefix_;
        const std::string stream_data_prefix_;
        const char delimiter_;
    };
};

#endif
