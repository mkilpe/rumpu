
#ifndef SPDRUM_TRACK_LIST_HEADER_ITEM_HEADER
#define SPDRUM_TRACK_LIST_HEADER_ITEM_HEADER

#include <QQuickPaintedItem>
#include <QString>
#include <QQmlContext>

namespace securepath::drum {

class track_list_header_item : public QQuickPaintedItem {
	Q_OBJECT
	Q_PROPERTY(QObject* model MEMBER model_)
public:
	using QQuickPaintedItem::QQuickPaintedItem;
	virtual void paint(QPainter* painter);

private:
	QObject* model_{};
};

inline void register_track_list_header_item() {
	qmlRegisterType<track_list_header_item>("SPDrum", 1, 0, "TrackListHeader");
}

}

#endif
