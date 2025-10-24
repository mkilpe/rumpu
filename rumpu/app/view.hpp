#pragma once

#include <memory>

namespace securepath::drum::app {

class view {
public:
    virtual ~view() = default;
    virtual bool draw() = 0;
};

using view_ptr = std::unique_ptr<view>;

}