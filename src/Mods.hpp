#pragma once
#include "Mod.hpp"
#include <filesystem>
#include "fw-imgui/ui.hpp"
#include "InitPatternsManager.hpp"
class Mods {
public:
    Mods();
    virtual ~Mods() {}

    std::optional<std::string> on_initialize(const bool& load_configs) const;

    void on_frame() const;
    void on_draw_ui() const;
    void on_draw_debug_ui() const;
    void draw_entry(Mod* mod);
    Mod* get_mod(std::string modName) const;
    std::string get_focused_mod() const;
    void set_focused_mod(const std::string& modName) const;
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
    mutable std::string m_focused_mod;
    std::vector<std::unique_ptr<Mod>> m_mods;
    utility::Config m_config;
};