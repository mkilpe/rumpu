#ifndef SECUREPATH_AUDIO_AUDIO_DEVICE_MODES_HEADER
#define SECUREPATH_AUDIO_AUDIO_DEVICE_MODES_HEADER

#include <cstdlib>

namespace securepath::audio {

struct mode {
protected:
	virtual ~mode() {}
};

struct notification_mode : mode {
	notification_mode(std::size_t samples)
		: samples(samples)
	{}

	std::size_t samples;
};

}

#endif
