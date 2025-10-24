#pragma once

#include "view.hpp"

namespace securepath::drum::app {

class track_header : public view {
public:
    bool draw() override;
};

}