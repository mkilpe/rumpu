#pragma once

#include "child_window.hpp"
#include <rumpu/core/song.hpp>

namespace securepath::drum::app {

struct track_context {
    std::size_t index{};
    drum::song* song{};
    std::uint32_t section_id{};
    drum::section* section{};

    bool is_valid() const {
        return song && section && index < section->tracks().size();
    }
};

class track_view : public child_window_base {
public:
    using child_window_base::child_window_base;
    virtual void set_context(track_context context) = 0;
};

class track : public track_view {
public:
    track(std::string name);

    bool do_draw() override;
    void set_context(track_context context) override;
private:
    track_context context_;    
};

using track_ptr = std::unique_ptr<track_view>;

}