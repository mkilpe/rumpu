#ifndef SECUREPATH_SERIALISATION_HELPERS_HEADER
#define SECUREPATH_SERIALISATION_HELPERS_HEADER

#include "has_serialise.hpp"
#include "tag.hpp"

#include <string>

namespace securepath::serialisation {

template<typename Ser, typename T, typename = serialise_enabler<T, Ser>>
inline
Ser& serialise(Ser& s, T&& v) {
	const_cast<typename std::decay<T>::type&>(v).serialise(s);
	return s;
}

template<typename Ser, typename T, typename = serialise_disabler<T, Ser>>
inline
Ser& serialise(Ser& s, T&& v, int=0) {
	s.handle(std::forward<T>(v));
	return s;
}

class serialiser;
class deserialiser;

template<typename T>
inline
serialiser& operator&(serialiser& s, T&& v) {
	return serialise(s, const_cast<typename std::decay<T>::type const&>(v));
}

//allow to serialise c-strings but not deserialise
inline
serialiser& operator&(serialiser& s, char const* v) {
	return serialise(s, std::string(v));
}

template<typename T>
inline
deserialiser& operator&(deserialiser& s, T&& v) {
	auto& res = serialise(s, std::forward<T>(v));
	if(std::exception_ptr p = res.move_exception()) {
		std::rethrow_exception(p);
	}
	return res;
}

}

#endif
