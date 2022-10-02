#ifndef SECUREPATH_UTIL_SPAN_HEADER
#define SECUREPATH_UTIL_SPAN_HEADER

#include <cstdint>
#include <vector>

namespace securepath {

template<typename Type, typename Iterator = Type const*>
class span {
public:
	using value_type = Type;
	using iterator = Iterator;
	using const_iterator = iterator;

	span() = default;
	span(iterator s, iterator e) : begin_(s), end_(e) {}
	span(iterator s, std::size_t size) : begin_(s), end_(s+size) {}
	span(std::vector<value_type> const& vec) : begin_(vec.data()), end_(vec.data()+vec.size()) {}

	std::size_t size() const { return end_ - begin_; }
	bool empty() const { return begin_ == end_; }
	value_type const* data() const { return &*begin_; }

	iterator begin() { return begin_; }
	const_iterator begin() const { return begin_; }
	const_iterator cbegin() const { return begin_; }
	iterator end() { return end_; }
	const_iterator end() const { return end_; }
	const_iterator cend() const { return end_; }

	value_type const& operator[](std::size_t p) const {
		return *(begin_+p);
	}

	span subspan(std::size_t start, std::size_t length = std::size_t(-1)) const {
		iterator s = start < size() ? begin_+start : end_;
		iterator e = length == std::size_t(-1) ? end_ : (start+length < size() ? begin_+start+length : end_);
		return span(s, e);
	}

private:
	iterator begin_{};
	iterator end_{};
};

template<typename T>
using mutable_span = span<T, T*>;

using octet_span = span<std::uint8_t>;

}

#endif
