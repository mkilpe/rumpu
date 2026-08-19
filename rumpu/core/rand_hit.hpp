#pragma once

#include <securepath/serialisation/sequence.hpp>

namespace securepath::drum {

// settings for randomising hit volume
struct rand_hit_volume {
	float max_percent{}; // 0–100, percentage of volume to vary
};

// settings for randomising hit offset
struct rand_hit_offset {
	float max_ms{}; // 0+, max milliseconds of timing offset
};

}
