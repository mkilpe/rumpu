#pragma once

#include "view.hpp"

#include "imgui.h"

namespace securepath::drum {

class child_window : public view {
public:
    virtual ~child_window() = default;
    virtual bool draw() = 0;
    virtual std::string name() const = 0;
    virtual bool is_visible() const = 0;
    virtual void set_visible(bool show) = 0;
    virtual void set_size(const ImVec2& size) = 0;
};

using child_window_ptr = std::unique_ptr<child_window>;

class view_child_window : public child_window {
public:
    view_child_window(std::string name, view_ptr view, ImGuiWindowFlags flags = {}, ImGuiChildFlags child_flags = ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY) 
    : name_(std::move(name))
    , flags_(flags)
    , child_flags_(child_flags)
    , view_(std::move(view)) 
    {}
    
    void set_size(const ImVec2& size) override {
        size_ = size;
    }

    bool draw() override {
        bool res = show_ && view_;
        if(res) {            
            ImGui::BeginChild(name_.c_str(), size_, child_flags_, flags_); 
            res = view_->draw();
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
    view_ptr view_;
    ImVec2 size_{};
};

}
