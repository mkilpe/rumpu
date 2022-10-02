
#ifndef SPDRUM_INSTRUMENT_MODEL_HEADER
#define SPDRUM_INSTRUMENT_MODEL_HEADER

#include <rumpu/core/song.hpp>

#include <QObject>
#include <QString>
#include <QQmlContext>
#include <QAbstractListModel>
#include <QModelIndex>

namespace securepath::drum {

class instrument_model : public QAbstractListModel {
	Q_OBJECT
public:
	enum {
		name = Qt::UserRole + 1,
		mute,
		volume
	};

	instrument_model(QQmlContext& qml);

	void set_song(drum::song*);

	virtual int rowCount(QModelIndex const& parent = QModelIndex()) const override;
	virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;
	virtual QHash<int, QByteArray> roleNames() const override;

	virtual Qt::ItemFlags flags(QModelIndex const& index) const override;
	virtual bool setData(QModelIndex const& index, QVariant const& value, int role = Qt::EditRole) override;

	Q_INVOKABLE void add(QString sampleUrl);

Q_SIGNALS:
	void instruments_changed();

public Q_SLOTS:
	void on_song_changed();

private:
	drum::song* song_{};
};

}

#endif
