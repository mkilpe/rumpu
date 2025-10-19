#pragma once

#include "view.hpp"

namespace securepath::drum {

class track_header : public view {
public:
    bool draw() override;
};

}