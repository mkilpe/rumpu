
#include "spdrum.hpp"

#include <fstream>

namespace securepath::drum {

spdrum::spdrum(int& argc, char* args[], config c)
: conf_(std::move(c))
, app_(argc, args)
{
	app_.setApplicationName("spdrum");
	app_.setOrganizationName("Secure Path");
	app_.setOrganizationDomain("securepath.fi");

	// the above needs to be set before the QQmlApplicationEngine is constructed
	gui_engine_.emplace();
}

void spdrum::run() {
	app_.exec();
}

}
