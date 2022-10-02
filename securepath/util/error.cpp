#include "error.hpp"

#include <iostream>

namespace securepath {

error::error(std::exception_ptr ex)
: code_(make_error_code(errc::exception_occurred))
{
	try {
		std::rethrow_exception(ex);
	} catch(std::exception const& ex) {
		msg_ = ex.what();
	} catch(...) {
	}
}

std::ostream& operator<<(std::ostream& out, error const& err) {
	if(err) {
		out << err.code() << ": " << err.code().message() << " (" << err.message() << ")";
	} else {
		out << "no error";
	}
	return out;
}

namespace {
	char const* errors[] =
		{ "no error"
		, "unknown error"
		, "exception occurred"
		, "not implemented"
		, "not supported"
		, "invalid data"
		, "no such data exists"
		, "operation timed out"
		, "invalid state"
		, "constraint violation"
		, "invalid argument" };

	using category_type = def_error_category<errc, errc::not_an_error, errc::end_of_list>;

	category_type& err_cat() {
		static category_type cat("securepath error", errors);
		return cat;
	}
}

std::error_condition make_error_condition(errc e) {
	return err_cat().make_error_condition(e);
}

std::error_code make_error_code(errc e) {
	return err_cat().make_error_code(e);
}

}
