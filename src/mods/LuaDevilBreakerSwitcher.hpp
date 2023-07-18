#pragma once
#include "LuaMod.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h"
class LuaDevilBreakerSwitcher : public LuaMod
{
public:
	LuaDevilBreakerSwitcher() = default;
	// mod name string for config
	std::string_view get_name() const override { return "LuaDevilBreakerSwitcher"; }
	std::string get_checkbox_name() override { return m_check_box_name; };
	std::string get_hotkey_name() override { return m_hot_key_name; };
	// called by m_mods->init() you'd want to override this
	std::optional<std::string> on_initialize() override;
	static bool cheaton;
	int breakaway_type;
	int breakers[8]{};
	int keyboard_breaker = -1;
	bool infinite_breakers = false;
	bool breaker_to_breaker_cancel = false;
	bool disable_diagonal = false;
	//int breakaway_index;
	void on_load_lua_mod() override;
	void on_unload_lua_mod() override;
	void on_lua_mod_update() override;
	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config & cfg) override;
	void on_config_save(utility::Config & cfg) override;

	// on_frame() is called every frame regardless whether the gui shows up.
	// void on_frame() override;
	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override;
	// on_draw_debug_ui() is called when debug window shows up
	// void on_draw_debug_ui() override;
private:
	void init_check_box_info() override;
};

