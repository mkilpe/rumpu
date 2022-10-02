#ifndef SECUREPATH_SERIALISATION_TYPES_HEADER
#define SECUREPATH_SERIALISATION_TYPES_HEADER

#include <securepath/util/types.hpp>

#include <chrono>
#include <cstdint>
#include <optional>
#include <stdexcept>

namespace securepath::serialisation {

using clock_type = std::chrono::system_clock;
using time_point = std::chrono::time_point<clock_type>;

struct tag_info {
	std::optional<std::uint_fast8_t> tag_type;
	std::uint64_t tag;
};

inline
bool operator==(tag_info const& l, tag_info const& r) {
	return l.tag_type == r.tag_type && l.tag == r.tag;
}

inline
bool operator!=(tag_info const& l, tag_info const& r) {
	return !(l == r);
}

class serialisation_error : public std::runtime_error {
public:
	serialisation_error(std::string const& s)
	: std::runtime_error(s)
	{}
};

//two's complement arbitrary size integer in big endian
class integer {
public:
	integer() = default;
	explicit integer(octet_vector v)
	: data_(std::move(v))
	{}

	octet_vector const& data() const { return data_; }
private:
	octet_vector data_;
};

inline
bool operator==(integer const& l, integer const& r) {
	return l.data() == r.data();
}

inline
bool operator!=(integer const& l, integer const& r) {
	return !(l == r);
}

class trailing_data {
public:
	trailing_data() = default;
	explicit trailing_data(octet_vector v)
	: data_(std::move(v))
	{}

	octet_vector const& data() const { return data_; }
private:
	octet_vector data_;
};

inline
bool operator==(trailing_data const& l, trailing_data const& r) {
	return l.data() == r.data();
}

inline
bool operator!=(trailing_data const& l, trailing_data const& r) {
	return !(l == r);
}

}

#endif
