#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class DanteSDTRework : public Mod {
public:
	DanteSDTRework() = default;
	// mod name string for config
	std::string_view get_name() const override { return "DanteSDTRework"; }
	std::string get_checkbox_name() override { return m_check_box_name; };
	std::string get_hotkey_name() override { return m_hot_key_name; };
	// called by m_mods->init() you'd want to override this
	std::optional<std::string> on_initialize() override;
	void on_sdk_init() override;
	static uintptr_t jmp_ret1;
	static uintptr_t jmp_jne1;
	static uintptr_t jmp_ret2;
	static uintptr_t jmp_jae2;
	static uintptr_t jmp_ret3;
	static uintptr_t jmp_ret4;
	static uintptr_t jmp_ret5;
	static uintptr_t jmp_ret6;
	static uintptr_t jmp_ret7;
	static uintptr_t jmp_jne7;
	static uintptr_t jmp_ret8;
	static bool cheaton;

	// Override this things if you want to store values in the config file
	// void on_config_load(const utility::Config& cfg) override;
	// void on_config_save(utility::Config& cfg) override;

	// on_frame() is called every frame regardless whether the gui shows up.
	// void on_frame() override;
	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	// void on_draw_ui() override;
	// on_draw_debug_ui() is called when debug window shows up
	// void on_draw_debug_ui() override;

private:
	// function hook instance for our detour, convinient wrapper
	// around minhook
	void init_check_box_info() override;
	std::shared_ptr<Detour_t> m_detour1;
	std::shared_ptr<Detour_t> m_detour2;
	std::shared_ptr<Detour_t> m_detour3;
	std::shared_ptr<Detour_t> m_detour4;
	std::shared_ptr<Detour_t> m_detour5;
	std::shared_ptr<Detour_t> m_detour6;
	std::shared_ptr<Detour_t> m_detour7;
	std::shared_ptr<Detour_t> m_detour8;
};
