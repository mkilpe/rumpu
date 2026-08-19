#ifndef SECUREPATH_UTIL_SPAN_HEADER
#define SECUREPATH_UTIL_SPAN_HEADER

#include <cstdint>
#include <span>

namespace securepath {

template<typename T>
using mutable_span = std::span<T>;

using octet_span = std::span<std::uint8_t const>;

}

#endif
