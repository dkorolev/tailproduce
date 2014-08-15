#ifndef GENERIC_ORDER_KEY_H
#define GENERIC_ORDER_KEY_H

#include <stdexcept>
#include <string>
#include "storage.h"
#include "config_values.h"
#include "order_key.h"

/*
TODO(dkorolev): Refactor this.
namespace TailProduce {
    template <typename PRIMARY_KEY, typename SECONDARY_KEY> struct GenericOrderKey : ::TailProduce::OrderKey {
        typedef PRIMARY_KEY T_PRIMARY_KEY;
        typedef SECONDARY_KEY T_SECONDARY_KEY;
        GenericOrderKey(std::string const& streamId, ConfigValues const& cv) : config_(cv) {
            prefix_ = config_.GetStreamDataPrefix(streamId) + config_.GetConnector();
        }

        Storage::T_KEY_TYPE CreateKey() {
            return asString_(primary_.GetValue(), secondary_.GetMin());
        }

        Storage::T_KEY_TYPE CreateKey(decltype(std::declval<T_PRIMARY_KEY>().Reverse("")) pval) {
            return asString_(primary_.GetValue(pval), secondary_.GetMin());
        }

        Storage::T_KEY_TYPE CreateKey(decltype(std::declval<T_PRIMARY_KEY>().Reverse("")) pval,
                                    decltype(std::declval<T_SECONDARY_KEY>().Reverse("")) sval) {
            return asString_(primary_.GetValue(pval), secondary_.GetValue(sval));
        }

        Storage::T_KEY_TYPE GetMaxKey() {
            return asString_(primary_.GetMax(), secondary_.GetMax());
        }

        Storage::T_KEY_TYPE GetMinKey() {
            return asString_(primary_.GetMin(), secondary_.GetMin());
        }

        decltype(std::declval<T_PRIMARY_KEY>().Reverse("")) ReversePrimary(Storage::T_KEY_TYPE const& orderKey) {
            Storage::T_KEY_TYPE primaryKey = orderKey.substr(prefix_.length(), primary_.NDigits());
            return primary_.Reverse(primaryKey);
        }

        decltype(std::declval<T_SECONDARY_KEY>().Reverse("")) ReverseSecondary(Storage::T_KEY_TYPE const& orderKey) {
            Storage::T_KEY_TYPE secondaryKey = orderKey.substr(1 + prefix_.length() + primary_.NDigits());
            return secondary_.Reverse(secondaryKey);
        }

      private:
        std::string prefix_;
        ConfigValues const& config_;
        T_PRIMARY_KEY primary_;
        T_SECONDARY_KEY secondary_;

        Storage::T_KEY_TYPE asString_(std::string const& tval, std::string const& secondary) {
            return std::string(prefix_ + tval + config_.GetConnector() + secondary);
        }
    };
}
*/

#endif
