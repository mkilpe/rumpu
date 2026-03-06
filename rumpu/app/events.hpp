#pragma once

#include <cstdint>
#include <string>

namespace securepath::drum::app::event {

struct add_instrument {
    typedef void type(std::string path, std::string name);
};

struct add_track {
    typedef void type(std::uint32_t section, std::size_t instrument_index);
};

struct open_add_track_dialog {
    typedef void type(std::uint32_t section);
};

struct play_song {
    typedef void type(std::uint32_t section);
};

struct stop_song {
    typedef void type(std::uint32_t section);
};

struct select_section {
    typedef void type(std::uint32_t section_id);
};

struct add_section {
    typedef void type();
};

struct remove_track {
    typedef void type(std::size_t index);
};

struct open_project {
    typedef void type(std::string path);
};

struct save_project {
    typedef void type(std::string path);
};

}