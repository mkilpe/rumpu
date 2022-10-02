#ifndef SECUREPATH_SERIALISATION_CONTAINER_HEADER
#define SECUREPATH_SERIALISATION_CONTAINER_HEADER

#include <securepath/serialisation/deserialiser.hpp>
#include <securepath/serialisation/sequence.hpp>
#include <securepath/serialisation/serialiser.hpp>

namespace securepath::serialisation {

template<typename Container>
inline
void container_serialise(serialiser& s, Container& v) {
	sequence<serialiser> seq(s);
	for(auto&& e : v) {
		seq & e;
	}
}

template<typename Container, typename Element = typename Container::value_type>
inline
void container_serialise(deserialiser& s, Container& v) {
	v.clear();
	sequence<deserialiser> seq(s);
	for(;!s.is_end_of_sequence();) {
		Element temp{};
		seq & temp;
		v.insert(v.end(), std::move(temp));
	}
}

}

#endif
