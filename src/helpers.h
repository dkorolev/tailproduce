#ifndef TAILPRODUCE_HELPERS_H
#define TAILPRODUCE_HELPERS_H

// TODO(dkorolev): Perhaps factor out bytes() into a different header file,
//                 have a single header file #include-ing all of the others
//                 and call it other than helpers.h?

#include <vector>
#include <string>
#include <cstring>  // strlen()
#include <cstdint>  // uint8_t

// Template magic: source-file-level template specialization is tricky
// and would require extra struct/class name, and to have header-file-level
// partial specialization one has to use class-level, not method-level specialization.
// Hence, template struct bytes_impl<> is intruduced to do the job.
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
// The actual template<> bytes(T x) implementation.
template<typename T> static std::vector<uint8_t> bytes(T x) {
    return bytes_impl<T>::run(x);
}

#endif  // TAILPRODUCE_HELPERS_H
