#pragma once

namespace securepath::drum::app {

class about_dialog {
public:
    void open();
    void do_draw();
private:
    bool open_{};
};

}
