#ifndef SECUREPATH_UTIL_PRINT_UTIL_HEADER
#define SECUREPATH_UTIL_PRINT_UTIL_HEADER

#include <securepath/log/detail/util.hpp>

#include <sstream>

namespace securepath {

	using log::print;

	template<typename Container>
	std::ostream& print_list(std::ostream& out, Container const& c, std::string_view separator, std::string_view quote = "") {
		bool first = true;
		for(auto&& v : c) {
			if(first) {
				first = false;
				out << quote << v << quote;
			} else {
				out << separator << quote << v << quote;
			}
		}
		return out;
	}

	template<typename Container>
	std::string print_list_to_string(Container const& c, std::string_view separator, std::string_view quote = "") {
		std::ostringstream out;
		print_list(out, c, separator, quote);
		return out.str();
	}

	template<typename... Params>
	std::string print(char const* msg, Params const&... params) {
		std::ostringstream out;
		print(out, msg, params...);
		return out.str();
	}
}

#endif
