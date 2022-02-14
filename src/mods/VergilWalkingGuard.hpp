#pragma once
#include "Mod.hpp"
class VergilWalkingGuard : public Mod
{
public:

	static inline bool cheaton = false;
	static inline bool skipInSdt = false;

	static inline uintptr_t ret = 0;

	VergilWalkingGuard() = default;

	std::string_view get_name() const override
	{
		return "WalkingGuard";
	}
	std::string get_checkbox_name() override
	{
		return m_check_box_name;
	};
	std::string get_hotkey_name() override
	{
		return m_hot_key_name;
	};

	std::optional<std::string> on_initialize() override;

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
	std::unique_ptr<FunctionHook> m_is_pad_input_hook;
};

