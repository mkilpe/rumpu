#include "qml_engine.hpp"

#include <QQuickStyle>
#include <QDebug>

namespace securepath::drum {

qml_engine::qml_engine()
: song_(song_metainfo{"Untitled"}, time_signature(4,4), tempo(120))
, scaling_(*rootContext())
, main_(*rootContext())
, sections_(*rootContext())
, tracks_(*rootContext())
, instruments_(*rootContext())
{
	QQuickStyle::setStyle("Fusion");
	load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));

	connect(&main_, SIGNAL(song_changed(std::uint32_t)), &tracks_, SLOT(on_song_changed(std::uint32_t)), Qt::DirectConnection);
	connect(&main_, SIGNAL(song_changed(std::uint32_t)), &sections_, SLOT(on_song_changed()), Qt::DirectConnection);
	connect(&main_, SIGNAL(song_changed(std::uint32_t)), &instruments_, SLOT(on_song_changed()), Qt::DirectConnection);
	connect(&sections_, SIGNAL(section_changed(std::uint32_t)), &tracks_, SLOT(on_song_changed(std::uint32_t)), Qt::DirectConnection);
	connect(&instruments_, SIGNAL(instruments_changed()), &tracks_, SLOT(on_instruments_changed()), Qt::DirectConnection);

	main_.set_song(&song_);
	sections_.set_song(&song_);
	tracks_.set_song(&song_);
	instruments_.set_song(&song_);
}

}
