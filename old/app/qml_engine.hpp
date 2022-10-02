
#ifndef SPDRUM_QML_ENGINE_HEADER
#define SPDRUM_QML_ENGINE_HEADER

#include "main_view_control.hpp"
#include "section_view_model.hpp"
#include "track_list_model.hpp"
#include "instrument_model.hpp"
#include <securepath/common/qt/scaling.hpp>

#include <QQmlApplicationEngine>

namespace securepath::drum {

class qml_engine : public QQmlApplicationEngine {
	Q_OBJECT
public:
	qml_engine();

private:
	song song_;

	qt::scaling scaling_;
	main_view_control main_;
	section_view_model sections_;
	track_list_model tracks_;
	instrument_model instruments_;
};

}

#endif
