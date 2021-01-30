#pragma once
#include "Mod.hpp"
#include <filesystem>
class Mods {
public:
    Mods();
    virtual ~Mods() {}

    std::optional<std::string> on_initialize() const;

    void on_frame() const;
    void on_draw_ui() const;
    void on_draw_debug_ui() const;
    void draw_entry(std::shared_ptr<Mod>&);
    std::shared_ptr<Mod> get_mod(std::string modname) const;
    std::string get_focused_mod() const;
    void on_pagelist_ui(int page);
    void save_mods() const;
    void load_mods() const;
    void static addressseek();
    std::string get_module_path(void* address) const;
    const auto& get_mods() const {
        return m_mods;
    }

private:
    mutable std::string focusedmod;
    mutable bool redrawfocusedwindow = false;
    std::vector<std::shared_ptr<Mod>> m_mods;
};