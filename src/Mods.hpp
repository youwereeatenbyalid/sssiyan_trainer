#pragma once
#include "Mod.hpp"
#include <filesystem>
#include "fw-imgui/ui.hpp"

class Mods {
public:
    Mods();
    virtual ~Mods() {}

    std::optional<std::string> on_initialize() const;

    void on_frame() const;
    void on_draw_ui() const;
    void on_draw_debug_ui() const;
    void draw_entry(std::unique_ptr<Mod>&);
    const std::unique_ptr<Mod>& get_mod(std::string modname) const;
    std::string get_focused_mod() const;
    void on_pagelist_ui(int page, float indent = 0.f);
    void save_mods();
    void load_mods(const std::optional<utility::Config>& cfg = std::nullopt) const;
    /*void static addressseek();
    std::string get_module_path(void* address) const;*/
    auto& get_config()
    {
        return m_config;
    }

	const auto& get_mods() const {
        return m_mods;
    }

private:
    mutable std::string focusedmod;
    std::vector<std::unique_ptr<Mod>> m_mods;
    utility::Config m_config;
};