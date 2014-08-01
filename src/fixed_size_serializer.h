// Support fixed-sized, zero-padded serialization and de-serialization of unsigned types.
// Requires the type of at least two bytes long. One-byte types, both signed and unsigned chars, are not supported.

#ifndef FIXED_SIZE_SERIALIZER_H
#define FIXED_SIZE_SERIALIZER_H

#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>

namespace TailProduce {
    struct FixedSizeSerializerEnabler {};
    template <typename T>
    struct FixedSizeSerializer
        : std::enable_if<std::is_unsigned<T>::value && std::is_integral<T>::value&&(sizeof(T) > 1),
                         FixedSizeSerializerEnabler>::type {
      public:
        enum { size_in_bytes = std::numeric_limits<T>::digits10 + 1 };
        static std::string PackToString(T x) {
            std::ostringstream os;
            os << std::setfill('0') << std::setw(size_in_bytes) << x;
            return os.str();
        }
        static T UnpackFromString(std::string const& s) {
            T x;
            std::istringstream is(s);
            is >> x;
            return x;
        }
    };
};

#endif
