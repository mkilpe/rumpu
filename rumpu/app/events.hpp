#pragma once

#include <cstdint>
#include <string>
#include <rumpu/core/time_signature.hpp>

namespace securepath::drum::app::event {

struct new_song {
    typedef void type(std::string name, time_signature ts, float tempo);
};

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

struct update_song_properties {
    typedef void type(std::string name, std::string author, std::string notes, time_signature ts, float tempo);
};

}