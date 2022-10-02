#ifndef SECUREPATH_LOG_LOG_HEADER
#define SECUREPATH_LOG_LOG_HEADER

#include "log_handler.hpp"

/**
 *	Macros that can be used for logging messages.
 */
#define LOG_TRACE(...) SP_LOG_IMPL(0, __VA_ARGS__)
#define LOG_INFO(...) SP_LOG_IMPL(4, __VA_ARGS__)
#define LOG_WARN(...) SP_LOG_IMPL(7, __VA_ARGS__)
#define SP_LOG_IMPL(level, ...) ::securepath::log::log_handler(::securepath::log::log_info{__FILE__, __LINE__, level}, __VA_ARGS__);

#endif
