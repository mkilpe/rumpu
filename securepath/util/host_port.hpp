#pragma once

#include <securepath/serialisation/sequence.hpp>

#include <cstdint>
#include <string>

namespace securepath {

/**
 * Type to hold hostname and port to connect to
 */
struct host_port {
	std::string host;
	std::uint16_t port;

	//auto operator<=>(host_port const&) const = default;
	//not working on android ndk 23 with clang

	bool operator<(host_port const& v) const;
	bool operator==(host_port const& v) const;

	bool is_valid() const;

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & host & port;
	}
};

std::ostream& operator<<(std::ostream&, host_port const&);

}