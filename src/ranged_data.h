#ifndef _RANGED_DATA_H
#define _RANGED_DATA_H

#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <chrono>
#include <limits>

/*
TODO(dkorolev): Refactor this.
namespace TailProduce {
    template <typename RANGE_TYPE> class RangedData : public std::enable_if<std::is_signed<RANGE_TYPE>::value> {
      public:
        typdef RANGE_TYPE T_RANGE_TYPE;
        RangedData() = default;
        T_RANGE_TYPE DefaultValue() {
            return DefaultValue();
        }
        std::string GetMin() {
            return asString_(std::numeric_limits<T_RANGE_TYPE>::min());
        }
        std::string GetMax() {
            return asString_(std::numeric_limits<T_RANGE_TYPE>::max());
        }
        std::string GetValue(T_RANGE_TYPE val = 0) {
            return asString_(val);
        }
        uint NDigits() {  // Useful for reverseing the key from string to type.
            return std::numeric_limits<T_RANGE_TYPE>::digits10 + 1;
        }
        T_RANGE_TYPE Reverse(std::string const& component) {
            int iter = 0;
            while (component[iter] == '0' && iter < component.length()) ++iter;  // Find first non zero value.
            if (iter >= component.size()) return 0;
            std::istringstream is(&(component.data()[iter]));
            T_RANGE_TYPE val;
            is >> val;
            return val;
        }

      private:
        std::string asString_(T_RANGE_TYPE value) {
            std::ostringstream os;
            os << std::setfill('0') << std::setw(NDigits()) << value;
            return os.str();
        }
    };

    // Uint8_t are handled separatedly because they don't encode/decode to integer values.
    template <> class RangedData<uint8_t> {
      public:
        RangedData() = default;
        uint8_t DefaultValue() {
            return DefaultValue();
        }
        std::string GetMin() {
            return asString_(std::numeric_limits<uint8_t>::min());
        }
        std::string GetMax() {
            return asString_(std::numeric_limits<uint8_t>::max());
        }
        std::string GetValue(uint8_t val = 0) {
            return asString_(val);
        }
        uint NDigits() {  // Useful for reverseing the key from string to type.
            return std::numeric_limits<uint8_t>::digits10 + 1;
        }
        uint8_t Reverse(std::string const& component) {
            int iter = 0;
            while (component[iter] == '0' && iter < component.length()) ++iter;  // Find first non zero value.
            if (iter >= component.size()) return 0;
            std::istringstream is(&(component.data()[iter]));
            uint16_t val;
            is >> val;
            return val;
        }

      private:
        std::string asString_(uint8_t value) {
            uint16_t val = value;
            std::ostringstream os;
            os << std::setfill('0') << std::setw(NDigits()) << val;
            return os.str();
        }
    };

    // Take a time point and convert it to a uint64_t, not a time_t which is signed.
    template <> class RangedData<std::chrono::time_point<std::chrono::system_clock>> {
      public:
        RangedData() = default;
        std::chrono::time_point<std::chrono::system_clock> DefaultValue() {
            return std::chrono::system_clock::now();
        }
        std::string GetMin() {
            return dr_.GetMin();
        }
        std::string GetMax() {
            return dr_.GetMax();
        }
        std::string GetValue(
            std::chrono::time_point<std::chrono::system_clock> val = std::chrono::system_clock::now()) {
            return dr_.GetValue(std::chrono::system_clock::to_time_t(val));
        }
        std::string GetValue(time_t val) {
            return dr_.GetValue(val);
        }
        std::string GetValue(uint64_t val) {
            return dr_.GetValue(val);
        }

        uint NDigits() {
            return dr_.NDigits();
        }
        std::chrono::time_point<std::chrono::system_clock> Reverse(std::string const& component) {
            return std::chrono::system_clock::from_time_t(dr_.Reverse(component));
        }

      private:
        RangedData<uint64_t> dr_;
    };
};
*/

#endif
