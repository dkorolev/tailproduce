#ifndef TAILPRODUCE_HELPERS_H
#define TAILPRODUCE_HELPERS_H

#include <vector>
#include <string>
#include <cstring>  // strlen()
#include <cstdint>  // uint8_t

namespace TailProduce {
    template<typename T> struct bytes_impl : std::enable_if<std::is_arithmetic<T>::value> {
        static std::vector<uint8_t> run(T x) {
            return std::vector<uint8_t>(&x, (&x) + 1);
        }
    };
    template<> struct bytes_impl<const char*> {
        static std::vector<uint8_t> run(const char* s) {
            return std::vector<uint8_t>(s, s + strlen(s));
        }
    };
    template<> struct bytes_impl<std::string> {
        static std::vector<uint8_t> run(const std::string& s) {
            return std::vector<uint8_t>(s.begin(), s.end());
        }
    };

    template<typename T> static std::vector<uint8_t> bytes(T x) {
        return bytes_impl<T>::run(x);
    }

    template<typename T> static std::string antibytes(const T& x) {
        return std::string(x.begin(), x.end());
    }
};

#endif  // TAILPRODUCE_HELPERS_H
