#include "user_dirs.hpp"

#include <cstdlib>
#include <string>
#include <system_error>

namespace securepath::drum {

namespace {

std::filesystem::path under(std::filesystem::path const& base, std::filesystem::path const& sub) {
	if(base.empty()) {
		return {};
	}
	return base / sub;
}

// absolute in the POSIX sense the XDG specification uses (starts at the
// root), which also holds for such paths on Windows where a drive letter is
// otherwise required for is_absolute()
std::filesystem::path rooted_or_empty(std::filesystem::path path) {
	if(!path.has_root_directory()) {
		return {};
	}
	return path;
}

}

std::filesystem::path environment_path(std::string_view name) {
#ifdef _WIN32
	std::wstring const wide_name(name.begin(), name.end()); // variable names are ASCII
	wchar_t const* value = _wgetenv(wide_name.c_str());
#else
	std::string const narrow_name(name);
	char const* value = std::getenv(narrow_name.c_str());
#endif
	if(value == nullptr) {
		return {};
	}
	return value;
}

user_dirs windows_user_dirs(env_lookup const& env) {
	std::filesystem::path base = env("LOCALAPPDATA");
	if(base.empty()) {
		base = under(env("USERPROFILE"), std::filesystem::path("AppData") / "Local");
	}
	std::filesystem::path const dir = under(base, "Rumpu");
	return {dir, dir};
}

user_dirs xdg_user_dirs(env_lookup const& env) {
	std::filesystem::path const home = rooted_or_empty(env("HOME"));
	std::filesystem::path config = rooted_or_empty(env("XDG_CONFIG_HOME"));
	if(config.empty()) {
		config = under(home, ".config");
	}
	std::filesystem::path state = rooted_or_empty(env("XDG_STATE_HOME"));
	if(state.empty()) {
		state = under(home, std::filesystem::path(".local") / "state");
	}
	return {under(config, "rumpu"), under(state, "rumpu")};
}

user_dirs default_user_dirs() {
#ifdef _WIN32
	return windows_user_dirs(environment_path);
#else
	return xdg_user_dirs(environment_path);
#endif
}

std::filesystem::path user_file(std::filesystem::path const& dir, std::filesystem::path const& name) {
	if(dir.empty()) {
		return name;
	}
	std::error_code ec;
	std::filesystem::create_directories(dir, ec);
	if(ec || !std::filesystem::is_directory(dir, ec)) {
		return name;
	}
	return dir / name;
}

}
