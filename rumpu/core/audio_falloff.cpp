#include "audio_falloff.hpp"

#include <cmath>

namespace securepath::drum {

namespace {

// player classes

struct immediate_player : audio_falloff_player {
	bool is_done() const override { return true; }
	fp_type factor(fp_type) override { return 0.0_fp; }
};

struct linear_player : audio_falloff_player {
	fp_type pos_{};
	fp_type end_{};

	explicit linear_player(fp_type end) : end_(end) {}

	bool is_done() const override { return pos_ >= end_; }

	fp_type factor(fp_type delta) override {
		pos_ += delta;
		if(pos_ > end_) {
			pos_ = end_;
		}
		return 1.0_fp - pos_ / end_;
	}
};

struct exponential_player : audio_falloff_player {
	fp_type pos_{};
	fp_type end_{};

	explicit exponential_player(fp_type end) : end_(end) {}

	bool is_done() const override { return pos_ >= end_; }

	fp_type factor(fp_type delta) override {
		pos_ += delta;
		if(pos_ > end_) {
			pos_ = end_;
		}
		fp_type x = pos_ / end_ * 7 - 5;
		return 1.0_fp - std::exp2(x) / 4;
	}
};

// definition classes

class immediate_falloff : public audio_falloff {
public:
	explicit immediate_falloff(fp_type db) : duration_beats_(db) {}

	falloff_type type() const override { return immediate; }
	fp_type duration_beats() const override { return duration_beats_; }
	void set_duration_beats(fp_type v) override { duration_beats_ = v; }

	std::unique_ptr<audio_falloff> clone() const override {
		return std::make_unique<immediate_falloff>(duration_beats_);
	}

	std::unique_ptr<audio_falloff_player> create_player(fp_type) const override {
		return std::make_unique<immediate_player>();
	}

private:
	fp_type duration_beats_;
};

class linear_falloff : public audio_falloff {
public:
	explicit linear_falloff(fp_type db) : duration_beats_(db) {}

	falloff_type type() const override { return linear; }
	fp_type duration_beats() const override { return duration_beats_; }
	void set_duration_beats(fp_type v) override { duration_beats_ = v; }

	std::unique_ptr<audio_falloff> clone() const override {
		return std::make_unique<linear_falloff>(duration_beats_);
	}

	std::unique_ptr<audio_falloff_player> create_player(fp_type beat_duration_seconds) const override {
		return std::make_unique<linear_player>(duration_beats_ * beat_duration_seconds);
	}

private:
	fp_type duration_beats_;
};

class exponential_falloff : public audio_falloff {
public:
	explicit exponential_falloff(fp_type db) : duration_beats_(db) {}

	falloff_type type() const override { return exponential; }
	fp_type duration_beats() const override { return duration_beats_; }
	void set_duration_beats(fp_type v) override { duration_beats_ = v; }

	std::unique_ptr<audio_falloff> clone() const override {
		return std::make_unique<exponential_falloff>(duration_beats_);
	}

	std::unique_ptr<audio_falloff_player> create_player(fp_type beat_duration_seconds) const override {
		return std::make_unique<exponential_player>(duration_beats_ * beat_duration_seconds);
	}

private:
	fp_type duration_beats_;
};

} // anon namespace

// factory

std::unique_ptr<audio_falloff> audio_falloff::create(falloff_type type, fp_type duration_beats) {
	switch(type) {
	case immediate:
		return std::make_unique<immediate_falloff>(duration_beats);
	case linear:
		return std::make_unique<linear_falloff>(duration_beats);
	case exponential:
		return std::make_unique<exponential_falloff>(duration_beats);
	}
	return std::make_unique<exponential_falloff>(duration_beats);
}

}
