#pragma once

#include <cstdint>

namespace securepath::drum::app::event {

struct add_track {
    typedef void type(std::uint32_t section);
};

}