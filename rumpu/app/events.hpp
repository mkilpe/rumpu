#pragma once

#include <cstdint>

namespace securepath::drum::app::event {

struct add_track {
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

}