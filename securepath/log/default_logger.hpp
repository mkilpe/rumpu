#ifndef SECUREPATH_LOG_DEFAULT_LOGGER_HEADER
#define SECUREPATH_LOG_DEFAULT_LOGGER_HEADER

#include "backend/backend.hpp"
#include "detail/streambuf.hpp"
#include "detail/util.hpp"

#include <chrono>

namespace securepath::log {

template<typename Func, typename... Args>
void default_log_impl(Func func, log_info const& info, char const* format, Args const&... args) {
	using time_point = std::chrono::time_point<std::chrono::system_clock>;

	char buffer[256] = {"                                                                      "};
	std::tm t{};
	auto time = time_point::clock::now();

	if(gmtime(time_point::clock::to_time_t(time), t)) {
		std::snprintf(buffer, 20, "%02d.%02d.%04d %02d:%02d:%02d"
			, t.tm_mday, t.tm_mon + 1, t.tm_year + 1900
			, t.tm_hour, t.tm_min, t.tm_sec);
		int ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count() % 1000;
		std::snprintf(buffer + 19, 6, ".%03d ", ms);
	}

	add_log_info_to_buffer(buffer, 24, 68, info);

	ologstreambuf buf(buffer, 70);
	std::ostream os(&buf);

	print(os, format, args...);
	func(formatted_message{info, std::string_view{buf.begin(), buf.size()}, std::string_view{}});
}

/**
 *	\brief default_log is default logged that formats message and uses basic_backend to log messages (log.hpp). Called by macros defined in log.hpp.
 *  \param info contains file and line where log macro was called, as well as importance of the message (log level).
 *  \param Param format contains actual log message where optional arguments (args) are added by print function implemented in util/print_util.hpp
 *	Function writes log message containing (TODO) to (TODO) (see tutorial for usage example) /////////////////////// TODO
 */
template<typename... Args>
void default_log(log_info const& info, char const* format, Args const&... args) {
	default_log_impl(backend::log, info, format, args...);
}

}

#endif
