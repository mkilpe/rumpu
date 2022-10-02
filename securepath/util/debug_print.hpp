#ifndef SECUREPATH_UTIL_DEBUG_PRINT_HEADER
#define SECUREPATH_UTIL_DEBUG_PRINT_HEADER

#include "print_util.hpp"
#include <securepath/log/log.hpp>

#include <iostream>

namespace securepath {

template<typename... Params>
void debug_print( char const* msg, Params&&... params ) {
	print(std::cout, msg, std::forward<Params>(params)...);
	std::cout << std::endl;
	LOG_TRACE(msg, std::forward<Params>(params)...);
}

}

#endif
