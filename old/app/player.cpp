
#include "player.hpp"
#include <securepath/log/log.hpp>

#include <algorithm>

namespace securepath::drum {

player::player(std::uint32_t sample_rate) {
	QAudioFormat format;
	format.setSampleRate(sample_rate);
	format.setChannelCount(1);
	format.setSampleSize(32);
	format.setCodec("audio/pcm");
	format.setSampleType(QAudioFormat::Float);

	QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
	if (info.isFormatSupported(format)) {
		out_ = new QAudioOutput(format, this);
		connect(out_, SIGNAL(stateChanged(QAudio::State)), this, SLOT(on_state_changed(QAudio::State)));
		connect(out_, SIGNAL(notify()), this, SLOT(on_notify()));
		out_->setBufferSize(sample_rate*4); //set the buffer to be one second
	} else {
		qWarning() << "Raw audio format not supported by backend, cannot play audio.";
	}
}

void player::play(song const* s, bool loop, std::uint32_t section) {
	stop();
	song_ = s;
	if(out_) {
		play_buffer_ = out_->start();

		int buffer_length = out_->format().durationForBytes(out_->bufferSize())/1000;
		int preferred_update_interval = out_->format().durationForBytes(out_->periodSize()*4)/1000;
		out_->setNotifyInterval(std::min(preferred_update_interval, buffer_length/2));

		qDebug() << "update interval" << std::min(preferred_update_interval, buffer_length/2) << out_->notifyInterval();

		buffer_.resize(out_->bufferSize()/sizeof(float));
		if(section) {
			mixer_.emplace(*song_, section, loop, out_->format().sampleRate());
		} else {
			mixer_.emplace(*song_, loop, out_->format().sampleRate());
		}
		write_data();
	}
}

float player::current_play_time() const {

	return mixer_ && out_
		? mixer_->play_position() - out_->format().durationForBytes(out_->bufferSize() - out_->bytesFree())/1000000.0
		: 0.0f;
}

int player::current_play_bar() const {
	return mixer_ ? mixer_->currently_playing_bar() : 0;
}

void player::set_gain(float v) {
	gain_ = v;
}

float player::gain() const {
	return gain_;
}

void player::write_data() {
	if(mixer_) {
		std::size_t process = std::min<std::size_t>(out_->bytesFree()/sizeof(float), buffer_.size());
		if(process) {
			int samples = mixer_->process(buffer_.data(), process);
			if(samples) {
				std::for_each(buffer_.data(), buffer_.data()+samples, [&](float& v)
				{
					v *= gain_;
				});
				int res = play_buffer_->write(reinterpret_cast<char const*>(buffer_.data()), samples*sizeof(float));
				if(res != samples*sizeof(float)) {
					qDebug() << "could not write all to buffer";
				}
			} else {
				stop();
			}
			Q_EMIT on_pos_changed();
		}
	}
}

void player::stop() {
	if(out_) {
		out_->stop();
		mixer_ = std::nullopt;
		Q_EMIT on_pos_changed();
	}
}

void player::on_state_changed(QAudio::State s) {
	qDebug() << "state changed" << s;
}

void player::on_notify() {
	write_data();
}

}
