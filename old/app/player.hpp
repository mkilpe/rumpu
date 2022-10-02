
#ifndef SPDRUM_QML_PLAYER_HEADER
#define SPDRUM_QML_PLAYER_HEADER

#include <rumpu/core/mixer.hpp>
#include <rumpu/core/song.hpp>

#include <QAudioOutput>
#include <QBuffer>

namespace securepath::drum {

class player : public QObject {
	Q_OBJECT
public:
	player(std::uint32_t sample_rate = 44100);

	void play(song const*, bool loop = false, std::uint32_t section = 0);
	void stop();

	float current_play_time() const;
	int current_play_bar() const;

	void set_gain(float);
	float gain() const;

private:
	void write_data();

Q_SIGNALS:
	void on_pos_changed();

private Q_SLOTS:
	void on_state_changed(QAudio::State);
	void on_notify();
private:
	QAudioOutput* out_;
	QIODevice* play_buffer_;
	song const* song_;
	std::optional<mixer> mixer_;
	std::vector<float> buffer_;
	float gain_{1.0f};
};

}

#endif
