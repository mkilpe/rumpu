#pragma once

#include "view.hpp"

#include "imgui.h"

namespace securepath::drum::app {

class child_window : public view {
public:    
    virtual bool draw() = 0;
    virtual std::string name() const = 0;
    virtual bool is_visible() const = 0;
    virtual void set_visible(bool show) = 0;
    virtual void set_size(const ImVec2& size) = 0;
protected:
    virtual bool do_draw() = 0;
};

using child_window_ptr = std::unique_ptr<child_window>;

class child_window_base : public child_window {
public:
    child_window_base(std::string name, ImGuiWindowFlags flags = {}, ImGuiChildFlags child_flags = ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY) 
    : name_(std::move(name))
    , flags_(flags)
    , child_flags_(child_flags)
    {}
    
    void set_size(const ImVec2& size) override {
        size_ = size;
    }

    bool draw() final {
        bool res = show_;
        if(res) {            
            ImGui::BeginChild(name_.c_str(), size_, child_flags_, flags_); 
            res = do_draw();
            ImGui::EndChild();
        }
        return res;
    }

    std::string name() const override {
        return name_;
    }

    bool is_visible() const override {
        return show_;
    }
    void set_visible(bool show) override {
        show_ = show;
    }
private:
    bool show_ {true};
    std::string const name_;
    ImGuiWindowFlags flags_{};
    ImGuiChildFlags child_flags_{};
    ImVec2 size_{};
};

}
