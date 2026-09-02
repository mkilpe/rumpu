#include <catch2/catch_all.hpp>

#include <rumpu/core/user_dirs.hpp>

#include <fstream>
#include <map>
#include <string>

using namespace securepath::drum;
namespace fs = std::filesystem;

namespace {

env_lookup fake_env(std::map<std::string, std::string> vars) {
	return [vars = std::move(vars)](std::string_view name) -> fs::path {
		auto const it = vars.find(std::string(name));
		if(it == vars.end()) {
			return {};
		}
		return it->second;
	};
}

}

TEST_CASE("xdg dirs honour XDG_CONFIG_HOME and XDG_STATE_HOME", "[user_dirs]") {
	user_dirs const d = xdg_user_dirs(fake_env({
		{"HOME", "/home/u"}, {"XDG_CONFIG_HOME", "/cfg"}, {"XDG_STATE_HOME", "/st"}}));
	CHECK(d.config == fs::path("/cfg/rumpu"));
	CHECK(d.state == fs::path("/st/rumpu"));
}

TEST_CASE("xdg dirs default to .config and .local/state under HOME", "[user_dirs]") {
	user_dirs const d = xdg_user_dirs(fake_env({{"HOME", "/home/u"}}));
	CHECK(d.config == fs::path("/home/u/.config/rumpu"));
	CHECK(d.state == fs::path("/home/u/.local/state/rumpu"));
}

TEST_CASE("xdg dirs ignore relative overrides", "[user_dirs]") {
	user_dirs const d = xdg_user_dirs(fake_env({{"HOME", "/home/u"}, {"XDG_CONFIG_HOME", "rel"}}));
	CHECK(d.config == fs::path("/home/u/.config/rumpu"));
}

TEST_CASE("xdg dirs are empty without a home", "[user_dirs]") {
	user_dirs const d = xdg_user_dirs(fake_env({}));
	CHECK(d.config.empty());
	CHECK(d.state.empty());
}

TEST_CASE("windows dirs use LOCALAPPDATA, then the profile's AppData\\Local", "[user_dirs]") {
	user_dirs const local = windows_user_dirs(fake_env({{"LOCALAPPDATA", "C:/Users/u/AppData/Local"}}));
	CHECK(local.config == fs::path("C:/Users/u/AppData/Local/Rumpu"));
	CHECK(local.state == local.config);

	user_dirs const profile = windows_user_dirs(fake_env({{"USERPROFILE", "C:/Users/u"}}));
	CHECK(profile.config == fs::path("C:/Users/u/AppData/Local/Rumpu"));

	CHECK(windows_user_dirs(fake_env({})).config.empty());
}

TEST_CASE("user_file creates the directory and otherwise falls back to the working directory", "[user_dirs]") {
	fs::path const root = fs::path(TEST_OUT_DIR) / "user_dirs";
	fs::remove_all(root);
	fs::path const dir = root / "nested" / "deeper";

	CHECK(user_file(dir, "a.log") == dir / "a.log");
	CHECK(fs::is_directory(dir));

	CHECK(user_file({}, "a.log") == fs::path("a.log"));

	// nothing can be created below a regular file
	fs::path const blocker = root / "blocker";
	std::ofstream(blocker) << "x";
	CHECK(user_file(blocker / "sub", "a.log") == fs::path("a.log"));
}
