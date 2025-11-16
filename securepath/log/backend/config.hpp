#ifndef SECUREPATH_LOG_CONFIG_HEADER
#define SECUREPATH_LOG_CONFIG_HEADER

#include <mutex>

namespace securepath::log {

/**
 *	Defines mutex_type and lock_guard used in log/backend implementation.
 */
using mutex_type = std::mutex;
using lock_guard = std::lock_guard<mutex_type>;

}

#endif
