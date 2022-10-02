#ifndef SECUREPATH_SERIALISATION_DEQUE_HEADER
#define SECUREPATH_SERIALISATION_DEQUE_HEADER

#include "container.hpp"

#include <deque>

namespace securepath::serialisation {

template<typename T>
inline
serialiser& serialise(serialiser& s, std::deque<T> const& v) {
	container_serialise(s, v);
	return s;
}

template<typename T>
inline
deserialiser& serialise(deserialiser& s, std::deque<T>& v) {
	container_serialise(s, v);
	return s;
}

}

#endif
