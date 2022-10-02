
#include "spdrum.hpp"
#include "bar_item.hpp"

#include <securepath/log/backend/backend.hpp>
#include <securepath/log/backend/file_output.hpp>
#include <securepath/log/log.hpp>

#include <iostream>

int main( int argc, char* args[] ) {
	try {
		securepath::log::backend::add_backend<securepath::log::backend::file_output>("file", "spdrum.log");
		securepath::drum::config p;
		p.parse_file("spdrum.cfg");
		p.parse(argc, args);
		if(p.help) {
			std::cout << "Secure Path Drum" << std::endl;
			p.print_help(std::cout);
		} else {
			QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
			securepath::drum::register_bar_item();
			securepath::drum::spdrum app(argc, args, p);
			app.run();
		}
	} catch(std::exception const& ex) {
		std::cout << "exception: " << ex.what() << std::endl;
	}
}
