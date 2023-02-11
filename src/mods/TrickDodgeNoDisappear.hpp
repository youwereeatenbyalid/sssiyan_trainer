#pragma once
#include "Mod.hpp"
#include "PosActionEditor.hpp"
#include "PlSetActionData.hpp"
#include "EndLvlHooks.hpp"
#include "Mods.hpp"
#include "Coroutine/Coroutines.hpp"

class TrickDodgeNoDisappear : public Mod, private EndLvlHooks::IEndLvl
{
public:

	inline static bool cheaton = false;
	//inline static bool isAuto = false;
	inline static bool _isTrickStopped = true;

	static inline uintptr_t ret = 0;
	
	static inline float endDrawOffMod = 0;
	static inline float specDrawOff = 1000.0f;
	static inline float zeroDrawOff = 0;

	TrickDodgeNoDisappear()
	{
	}

	~TrickDodgeNoDisappear()
	{
	}

	std::string_view get_name() const override
	{
		return "TrickDodgeNoDisappear";
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
	// Inherited via IEndLvl
	void reset(EndLvlHooks::EndType type) override
	{
		_isTrickStopped = true;
	}

	void init_check_box_info() override;
	std::unique_ptr<FunctionHook> m_disappear_hook;
};

