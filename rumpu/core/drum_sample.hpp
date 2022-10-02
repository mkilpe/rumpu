#ifndef SPDRUM_COMMON_DRUM_SAMPLE_HEADER
#define SPDRUM_COMMON_DRUM_SAMPLE_HEADER

#include <securepath/serialisation/sequence.hpp>

#include <QAudioDecoder>

#include <memory>
#include <string>

namespace securepath::drum {

using sample_buffer = std::shared_ptr<std::vector<float> const>;

class drum_sample {
public:
	drum_sample() = default;
	drum_sample(std::string file);

	void load_sample(std::uint32_t sample_rate);

	std::string source_file() const { return source_file_; }
	sample_buffer buffer() const { return buffer_; }
	std::uint32_t sample_rate() const { return sample_rate_; }

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & source_file_;
	}
private:
	std::string source_file_;

	//these two are not serialised but rather filled when loaded
	std::uint32_t sample_rate_{};
	sample_buffer buffer_;
};

class sample_loader : public QObject {
	Q_OBJECT
public:
	sample_loader(std::uint32_t sample_rate);

	void load(std::string const& file);
	std::string error() const;
	sample_buffer buffer() const;

Q_SIGNALS:
	void on_done();
	void on_error(QAudioDecoder::Error);
private Q_SLOTS:
	void on_buffer_ready();
private:
	std::string name_;
	QAudioDecoder decoder_;
	std::shared_ptr<std::vector<float>> buffer_;
};

drum_sample load_drum_sample(std::string const& file, std::uint32_t sample_rate = 44100);

}

#endif