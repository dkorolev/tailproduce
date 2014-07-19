#ifndef GENERIC_ORDER_KEY_H
#define GENERIC_ORDER_KEY_H

#include <stdexcept>
#include <string>
#include "storage.h"
#include "config_values.h"
#include "order_key.h"

namespace TailProduce {
    template <typename PRIMARY_KEY, typename SECONDARY_KEY> class GenericOrderKey : public ::TailProduce::OrderKey {
      public:
        GenericOrderKey(std::string const& streamId, ConfigValues const& cv) : config_(cv) {
            prefix_ = config_.GetStreamsData(streamId) + config_.GetConnector();
        }

        Storage::KEY_TYPE CreateKey() {
            return asString_(primary_.GetValue(), secondary_.GetMin());
        }

        Storage::KEY_TYPE CreateKey(decltype(std::declval<PRIMARY_KEY>().Reverse("")) pval) {
            return asString_(primary_.GetValue(pval), secondary_.GetMin());
        }

        Storage::KEY_TYPE CreateKey(decltype(std::declval<PRIMARY_KEY>().Reverse("")) pval,
                                    decltype(std::declval<SECONDARY_KEY>().Reverse("")) sval) {
            return asString_(primary_.GetValue(pval), secondary_.GetValue(sval));
        }

        Storage::KEY_TYPE GetMaxKey() {
            return asString_(primary_.GetMax(), secondary_.GetMax());
        }

        Storage::KEY_TYPE GetMinKey() {
            return asString_(primary_.GetMin(), secondary_.GetMin());
        }

        decltype(std::declval<PRIMARY_KEY>().Reverse("")) ReversePrimary(Storage::KEY_TYPE const& orderKey) {
            Storage::KEY_TYPE primaryKey = orderKey.substr(prefix_.length(), primary_.NDigits());
            return primary_.Reverse(primaryKey);
        }

        decltype(std::declval<SECONDARY_KEY>().Reverse("")) ReverseSecondary(Storage::KEY_TYPE const& orderKey) {
            Storage::KEY_TYPE secondaryKey = orderKey.substr(1 + prefix_.length() + primary_.NDigits());
            return secondary_.Reverse(secondaryKey);
        }

      private:
        std::string prefix_;
        ConfigValues const& config_;
        PRIMARY_KEY primary_;
        SECONDARY_KEY secondary_;

        Storage::KEY_TYPE asString_(std::string const& tval, std::string const& secondary) {
            return std::string(prefix_ + tval + config_.GetConnector() + secondary);
        }
    };
}

#endif
