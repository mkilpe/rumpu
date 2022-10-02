
#ifndef SPDRUM_SECTION_VIEW_MODEL_HEADER
#define SPDRUM_SECTION_VIEW_MODEL_HEADER

#include <rumpu/core/song.hpp>

#include <QObject>
#include <QString>
#include <QQmlContext>
#include <QAbstractListModel>
#include <QModelIndex>

namespace securepath::drum {

class section_view_model : public QAbstractListModel {
	Q_OBJECT
public:
	section_view_model(QQmlContext& qml);

	void set_song(drum::song*);

	virtual Qt::ItemFlags flags(QModelIndex const& index) const override;

	virtual int rowCount(QModelIndex const& parent = QModelIndex()) const override;
	virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;

	virtual bool setData(QModelIndex const& index, QVariant const& value, int role = Qt::EditRole) override;
	virtual bool insertRows(int row, int count, QModelIndex const& parent = QModelIndex()) override;
	virtual bool removeRows(int row, int count, QModelIndex const& parent = QModelIndex()) override;

	Q_INVOKABLE void append();
	Q_INVOKABLE void mouseClicked(int index);

	Q_INVOKABLE void move(int source, int dest);
Q_SIGNALS:
	void section_changed(std::uint32_t);

public Q_SLOTS:
	void on_song_changed();

private:
	drum::song* song_{};
};

}

#endif
