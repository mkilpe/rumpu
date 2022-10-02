
#ifndef SPDRUM_TRACK_LIST_MODEL_HEADER
#define SPDRUM_TRACK_LIST_MODEL_HEADER

#include <rumpu/core/song.hpp>

#include <QObject>
#include <QString>
#include <QQmlContext>
#include <QAbstractItemModel>
#include <QModelIndex>

namespace securepath::drum {

class track_list_model : public QAbstractItemModel {
	Q_OBJECT
public:
	Q_PROPERTY(int section READ section_id NOTIFY sectionChanged)
	Q_PROPERTY(int length READ length WRITE setLength NOTIFY lengthChanged)

	track_list_model(QQmlContext& qml);

	enum {
		bar = Qt::UserRole + 1,
		division
	};
public:
	void set_song(drum::song*, std::uint32_t section = 0);

	Qt::ItemFlags flags(QModelIndex const& index) const;
	int columnCount(QModelIndex const& parent = QModelIndex()) const;
	int rowCount(QModelIndex const& parent = QModelIndex()) const;

	QModelIndex index(int row, int column, QModelIndex const& parent = QModelIndex()) const;
	QModelIndex parent(QModelIndex const& child) const;

	QVariant data(QModelIndex const& index, int role) const;
	QHash<int, QByteArray> roleNames() const;

	Q_INVOKABLE bool setMark(int row, int column, QVariant const& value);

	void setLength(int length);
	int length() const;
	int section_id() const;

public Q_SLOTS:
	void on_instruments_changed();
	void on_song_changed(std::uint32_t sec);

Q_SIGNALS:
	void sectionChanged();
	void lengthChanged();

private:
	drum::song* song_{};
	std::uint32_t section_id_{};
	section* section_{};
};

}

#endif
