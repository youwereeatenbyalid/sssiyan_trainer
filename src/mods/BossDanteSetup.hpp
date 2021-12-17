#pragma once
#include "Mod.hpp"
class BossDanteSetup : public Mod
{
public:
	inline static bool cheaton = false;
	inline static bool isDtRegenEnabled = false;
	inline static bool isSdtTransformSetup = false;
	inline static bool isDtsTimerSkip = false;
	inline static bool isCustomDtDuration = false;

	inline static uint32_t sdtTransformMode = 1; //0 - normal, 1 - allFast; 2 - firstNormalOtherFast;
	
	inline static uintptr_t dtRegenRet = 0x0;
	inline static uintptr_t dtRegenJe = 0x0;
	inline static uintptr_t sdtTransformRet = 0x0;
	inline static uintptr_t dtTimerRet = 0x0;
	inline static uintptr_t dtDurationRet = 0x0;

	inline static float dtTimer = 5.0f;
	inline static float dtTimerStartPoint = 0.0f;
	inline static const float dtTimerMax = 29.9f;
	inline static float dtDuration = 35.0f;


	BossDanteSetup() = default;

	std::string_view get_name() const override
	{
		return "BossDanteSetup";
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
	void on_frame() override;
	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override;
	// on_draw_debug_ui() is called when debug window shows up
	void on_draw_debug_ui() override;

private:
	void init_check_box_info() override;
	std::unique_ptr<FunctionHook> m_dtregen_hook;
	std::unique_ptr<FunctionHook> m_sdttransform_hook;
	std::unique_ptr<FunctionHook> m_dttimer_hook;
	std::unique_ptr<FunctionHook> m_dtduration_hook;
};

