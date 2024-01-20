
#include "drum_sample.hpp"

#include <QEventLoop>
#include <stdexcept>

namespace securepath::drum {

drum_sample::drum_sample(std::string file)
: source_file_(std::move(file))
{
}

void drum_sample::load_sample(std::uint32_t sample_rate) {
	sample_loader loader(sample_rate);
	loader.load(source_file_);

	std::optional<std::string> error;

	QEventLoop loop;
	QObject::connect( &loader, &sample_loader::on_done, &loop, &QEventLoop::quit );
	QObject::connect( &loader, &sample_loader::on_error, [&](QAudioDecoder::Error err)
		{
			error = loader.error();
			loop.quit();
		});

	loop.exec();
	if(error) {
		throw std::runtime_error(*error);
	}
	buffer_ = loader.buffer();
	sample_rate_ = sample_rate;
}

sample_loader::sample_loader(std::uint32_t sample_rate)
: buffer_(std::make_shared<std::vector<float>>())
{
	QAudioFormat format;
	format.setSampleRate(sample_rate);
	format.setChannelCount(1);
	format.setSampleSize(32);
	format.setCodec("audio/pcm");
	format.setSampleType(QAudioFormat::Float);

	decoder_.setAudioFormat(format);
}

void sample_loader::load(std::string const& file) {
	name_ = file;
	decoder_.setSourceFilename(QString::fromStdString(name_));
	connect(&decoder_, SIGNAL(bufferReady()), this, SLOT(on_buffer_ready()));
	connect(&decoder_, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error),
    	[this](QAudioDecoder::Error error) {
    		qDebug() << "Sample loader error:" << error;
    		Q_EMIT on_error(error);
    	});
	connect(&decoder_, SIGNAL(finished()), this, SIGNAL(on_done()));
	decoder_.start();
}

void sample_loader::on_buffer_ready() {
	auto buf = decoder_.read();
	buffer_->insert(buffer_->end(), buf.constData<float>(), buf.constData<float>()+buf.sampleCount());
}

sample_buffer sample_loader::buffer() const {
	return buffer_;
}

std::string sample_loader::error() const {
	return decoder_.errorString().toStdString();
}

drum_sample load_drum_sample(std::string const& file, std::uint32_t sample_rate) {
	drum_sample sample(file);
	sample.load_sample(sample_rate);
	return sample;
}

}
