#pragma once

#include "view.hpp"

namespace securepath::drum::app {

class about_dialog : public view {
public:
    void open();
    bool draw() override;
private:
    bool open_{};
};

}
