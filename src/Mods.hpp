#pragma once

#include "Mod.hpp"

class Mods {
public:
    Mods();
    virtual ~Mods() {}

    std::optional<std::string> on_initialize() const;

    void on_frame() const;
    void on_draw_ui() const;
    void on_draw_debug_ui() const;
    std::shared_ptr<Mod> get_mod(std::string modname) const;
    std::string get_focused_mod() const;
    void on_pagelist_ui(int page) const;
    const auto& get_mods() const {
        return m_mods;
    }

private:
    mutable std::string focusedmod;
    mutable bool redrawfocusedwindow = false;
    std::vector<std::shared_ptr<Mod>> m_mods;
};