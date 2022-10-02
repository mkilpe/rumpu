
#ifndef SPDRUM_MAIN_VIEW_CONTROL_HEADER
#define SPDRUM_MAIN_VIEW_CONTROL_HEADER

#include "player.hpp"
#include <rumpu/core/song.hpp>

#include <QQmlContext>

namespace securepath::drum {

class main_view_control : public QObject {
	Q_OBJECT
public:
	Q_PROPERTY(float playTime READ play_time NOTIFY playChanged)
	Q_PROPERTY(int playBar READ play_bar NOTIFY playChanged)
	Q_PROPERTY(int totalPlayBars READ total_play_bars NOTIFY totalBarsChanged)
	Q_PROPERTY(QString songName READ song_name NOTIFY songMetaChanged)
	Q_PROPERTY(float gain READ gain WRITE set_gain NOTIFY gainChanged)

	main_view_control(QQmlContext& qml);

	void set_song(song*);

	Q_INVOKABLE void new_song(QString const& name, int beats, int beat_type, int tempo);
	Q_INVOKABLE void load_song(QString const& file);
	Q_INVOKABLE void save_song();
	Q_INVOKABLE void save_song(QString const& file);
	Q_INVOKABLE void export_song(QString const& file);

	Q_INVOKABLE void play();
	Q_INVOKABLE void stop();

	Q_INVOKABLE QString filename() const;

public:
	float play_time() const;
	int play_bar() const;
	int total_play_bars() const;
	QString song_name() const;
	float gain() const;
	void set_gain(float);

Q_SIGNALS:
	void song_changed(std::uint32_t section);

	void playChanged();
	void totalBarsChanged();
	void songMetaChanged();
	void sectionsChanged();
	void gainChanged();

private:
	player player_;
	QString file_;
	song* song_{};
	float gain_{0.8};
};

}

#endif
