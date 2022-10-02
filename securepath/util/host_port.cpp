#include "host_port.hpp"

#include <ostream>

namespace securepath {

bool host_port::operator<(host_port const& v) const {
	return host < v.host || (host == v.host && port < v.port);
}

bool host_port::operator==(host_port const& v) const {
	return host == v.host && port == v.port;
}

bool host_port::is_valid() const {
	return !host.empty() && port != 0;
}

std::ostream& operator<<(std::ostream& out, host_port const& hp) {
	return out << hp.host << ":" << hp.port;
}

}