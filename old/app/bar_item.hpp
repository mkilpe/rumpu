
#ifndef SPDRUM_BAR_ITEM_HEADER
#define SPDRUM_BAR_ITEM_HEADER

#include <rumpu/core/track.hpp>

#include <QQuickPaintedItem>
#include <QString>
#include <QQmlContext>

namespace securepath::drum {

struct bar_data {
	Q_GADGET
public:
	bar_data() = default;

	explicit bar_data(int div)
	: default_division(div)
	{}

	template<typename T>
	bar_data(int div, T const& t)
	: default_division(div)
	, data(t)
	{}

	Q_INVOKABLE QVariant clear(bool only_data);
	Q_INVOKABLE QVariant toggle_hit(float npos);
	Q_INVOKABLE QVariant toggle_stop(float npos);

	Q_INVOKABLE int division() const;
	Q_INVOKABLE QVariant divide(int div);

	Q_INVOKABLE int subdivision(int beat) const;
	Q_INVOKABLE QVariant subdivide(int beat, int div);

	int default_division{};
	bar data;

private:
	QVariant toggle(float npos, beat::action_type action);
};

inline bool operator==(bar_data const& l, bar_data const& r) {
	return l.default_division == r.default_division && l.data == r.data;
}

inline bool operator!=(bar_data const& l, bar_data const& r) {
	return !(l == r);
}

class bar_item : public QQuickPaintedItem {
	Q_OBJECT
	Q_PROPERTY(securepath::drum::bar_data model MEMBER data_ NOTIFY model_changed)
public:
	using QQuickPaintedItem::QQuickPaintedItem;
	virtual void paint(QPainter* painter);

Q_SIGNALS:
	void model_changed();
private:
	bar_data data_;
};

inline void register_bar_item() {
	qmlRegisterType<bar_item>("SPDrum", 1, 0, "Bar");
	qmlRegisterUncreatableType<bar_data>("SPDrum", 1, 0, "BarData", "this is opaque type");
}

}

Q_DECLARE_METATYPE(securepath::drum::bar_data);

#endif
