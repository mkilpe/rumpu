#ifndef SECUREPATH_LOG_BACKEND_FILE_OUTPUT_HEADER
#define SECUREPATH_LOG_BACKEND_FILE_OUTPUT_HEADER

#include <fstream>
#include <string>

namespace securepath::log::backend {

/**
 *	\class file_output
 *	\brief A class that is used for writing log message to file.
 *
 */
class file_output : public output_base {
public:
	/**
	 *	\param String file is name for the file where log messages will be written.
	 */
	file_output(std::string const& file, int level = 0)
	: out_(file, std::ios_base::app)
	, level_(level)
	{}

	/**
	 *	\param Virtual output function that logs message to file.
	 */
	virtual void log(formatted_message const& m) {

		if(level_ <= m.info.level) {
			out_.write(m.message.data(), m.message.size());
			out_ << std::endl;		
		}
		
	}

	/**
	 *	Minimum level of the log message this class logs.
	 * 	If the level of the log message is lower than function returns, message won't be logged.
	 */
	virtual int get_min_log_level() const {
		return level_;
	}

private:
	std::ofstream out_;
	int level_;
};

}

#endif
