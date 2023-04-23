#pragma once
#include "Mod.hpp"
#include <filesystem>
#include "fw-imgui/ui.hpp"
#include "InitPatternsManager.hpp"
#include "events/Events.hpp"
#include "mods/Coroutine/Coroutines.hpp"
#include <sol/sol.hpp>
#include <unordered_map>

class Mods {
public:
    Mods();
    virtual ~Mods() {}

    std::optional<std::string> on_initialize(const bool& load_configs);

    void on_frame() const;
    void on_draw_ui() const;
    void on_draw_debug_ui() const;
    void draw_entry(Mod* mod);
    Mod* get_mod(const std::string& modName) const;
    const std::string& get_focused_mod() const;
    void set_focused_mod(const std::string& modName) const;
    void on_pagelist_ui(int page, float indent = 0.f);
    void on_lua_state_created(lua_State* l);
    void on_lua_state_destroyed(lua_State* l);
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
    bool update_mod_state(Mod* mod) const;

    mutable std::string m_focused_mod;
    std::vector<std::unique_ptr<Mod>> m_mods;
    utility::Config m_config;
    lua_State* global_lua_state{nullptr};
    std::unique_ptr<Coroutines::Coroutine<void(Mods::*)(), Mods*>> m_init_sdk_coroutine = std::make_unique<Coroutines::Coroutine<void(Mods::*)(), Mods*>>(&Mods::init_sdk, true, true);
	std::unordered_map<std::string, Mod*> m_name_to_mod_map{};

    void init_sdk()
    {
        for (const auto& i : m_mods)
            i->on_sdk_init();
        m_init_sdk_coroutine->stop();
        m_init_sdk_coroutine = nullptr;
    }
};