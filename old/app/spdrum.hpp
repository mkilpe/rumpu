#ifndef SPDRUM_APP_SPDRUM_HEADER
#define SPDRUM_APP_SPDRUM_HEADER

#include "qml_engine.hpp"

#include <securepath/util/command_parser.hpp>

#include <QGuiApplication>

#include <iostream>
#include <memory>

namespace securepath::drum {

struct config : command_parser {
	bool help = false;

	config() {
		add(help, "help", "h", "show help");
	}
};

class spdrum {
public:
	spdrum(int& argc, char* args[], config c);

	void run();

private:
	config conf_;

	QGuiApplication app_;
	std::optional<qml_engine> gui_engine_;
};

}

#endif