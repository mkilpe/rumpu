#include "main_view_control.hpp"

#include <rumpu/core/export.hpp>
#include <rumpu/core/song_file.hpp>
#include <securepath/log/log.hpp>

#include <QDebug>

namespace securepath::drum {

main_view_control::main_view_control(QQmlContext& qml)
{
	qml.setContextProperty("mainControl", this);
	connect(&player_, SIGNAL(on_pos_changed()), this, SIGNAL(playChanged()));
}

void main_view_control::set_song(song* s) {
	song_ = s;
}

void main_view_control::new_song(QString const& name, int beats, int beat_type, int tempo) {
	player_.stop();
	*song_ = song{song_metainfo{name.toStdString()}, time_signature(beats, beat_type), drum::tempo(tempo > 0 ? tempo : 120)};
	Q_EMIT song_changed(0);
	Q_EMIT songMetaChanged();
}

void main_view_control::load_song(QString const& file) {
	QUrl url(file);
	QString local_file = url.toLocalFile();
	try {
		song s = load_song_file(local_file.toStdString());
		s.load_instruments();
		player_.stop();
		*song_ = std::move(s);
		file_ = local_file;
		Q_EMIT song_changed(0);
		Q_EMIT songMetaChanged();
	} catch(std::exception const& ex) {
		LOG_WARN("failed to load song: % (%)", ex, local_file.toStdString());
	}
}

void main_view_control::save_song() {
	if(song_) {
		try {
			save_song_file(file_.toStdString(), *song_);
		} catch(std::exception const& ex) {
			LOG_WARN("failed to save song: % (%)", ex, file_.toStdString());
		}
	}
}

void main_view_control::save_song(QString const& file) {
	if(song_) {
		QUrl url(file);
		QString local_file = url.toLocalFile();
		try {
			save_song_file(local_file.toStdString(), *song_);
			file_ = local_file;
		} catch(std::exception const& ex) {
			LOG_WARN("failed to save song: % (%)", ex, local_file.toStdString());
		}
	}
}

void main_view_control::export_song(QString const& file) {
	if(song_) {
		QUrl url(file);
		export_as_wav(url.toLocalFile().toStdString(), *song_);
	}
}

QString main_view_control::filename() const {
	return file_;
}

void main_view_control::play() {
	if(song_) {
		player_.play(song_);
		//t: connect this to something real when adding/removing sections/bars
		Q_EMIT totalBarsChanged();
	}
}

void main_view_control::stop() {
	player_.stop();
}

float main_view_control::play_time() const {
	return player_.current_play_time();
}

int main_view_control::play_bar() const {
	return player_.current_play_bar();
}

int main_view_control::total_play_bars() const {
	int ret = 0;
	if(song_) {
		for(auto const& s : song_->section_order()) {
			auto section = song_->find_section(s);
			if(section) {
				ret += section->length();
			}
		}
	}
	return ret;
}

QString main_view_control::song_name() const {
	return QString::fromStdString(song_ ? song_->meta_info().name : "");
}

float main_view_control::gain() const {
	return gain_;
}

void main_view_control::set_gain(float g) {
	gain_ = g;
	player_.set_gain(gain_);
}

}
