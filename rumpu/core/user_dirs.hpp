#pragma once

#include <filesystem>
#include <functional>
#include <string_view>

namespace securepath::drum {

/// Per-user directories the app may write to. A member is empty when the
/// environment names no usable location; callers then fall back to the
/// working directory.
struct user_dirs {
	std::filesystem::path config; ///< settings, imgui.ini
	std::filesystem::path state;  ///< logs
};

/// Value of an environment variable as a path, empty when it is unset
using env_lookup = std::function<std::filesystem::path(std::string_view name)>;

/// The process environment (wide on Windows, so non-ASCII profile paths survive)
std::filesystem::path environment_path(std::string_view name);

/// Windows convention: %LOCALAPPDATA%\Rumpu, or the profile's AppData\Local
/// when LOCALAPPDATA is unset; config and state share the directory
user_dirs windows_user_dirs(env_lookup const& env);

/// XDG base directory convention: $XDG_CONFIG_HOME/rumpu and
/// $XDG_STATE_HOME/rumpu, defaulting to ~/.config and ~/.local/state.
/// Relative overrides are ignored, as the specification requires.
user_dirs xdg_user_dirs(env_lookup const& env);

/// The convention of the platform this was built for, from the process environment
user_dirs default_user_dirs();

/// dir/name with dir created on demand; just name (relative to the working
/// directory) when dir is empty or cannot be used as a directory
std::filesystem::path user_file(std::filesystem::path const& dir, std::filesystem::path const& name);

}
