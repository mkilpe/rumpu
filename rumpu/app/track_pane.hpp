#pragma once

#include "view.hpp"

namespace securepath::drum {

class track_pane : public view {
public:
    bool draw() override;

private:
    float gain_{};
};

}