#pragma once

#include "view.hpp"
#include "ui_coroutine.hpp"

namespace securepath::drum::app {

class about_dialog : public view {
public:
    void open();
    bool draw() override;
private:
    ui_task run();
    ui_task task_;
};

}
