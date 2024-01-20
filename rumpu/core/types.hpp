#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace securepath::drum {
	using fp_type = float;

	inline constexpr fp_type operator""_fp(long double value) {
		return static_cast<fp_type>(value);
	}
}
