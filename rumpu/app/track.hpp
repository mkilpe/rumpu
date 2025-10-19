#pragma once

#include "view.hpp"

namespace securepath::drum {

class track : public view {
public:
    bool draw() override;
};

using track_ptr = std::unique_ptr<track>;

}