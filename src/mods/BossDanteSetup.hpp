#pragma once
#include "Mod.hpp"
#include "GameFunctions/GameFunc.hpp"
#include "PlSetActionData.hpp"
#include <random>

namespace gf = GameFunctions;

class BossDanteSetup : public Mod
{
public:
	inline static bool cheaton = false;
	inline static bool isDtRegenEnabled = false;
	inline static bool isSdtTransformSetup = false;

	inline static bool isDtsTimerSkip = false;
	inline static bool isCustomDtDuration = false;
	inline static bool isNoFinishSdtStun = false;
	inline static bool isNoMovesDelay = false;
	static inline bool isSdtRegen = false;
	static inline bool allowRoyalRelease = true;
	//static inline bool allowFlipper = true;

	inline static uint32_t sdtTransformMode = 1; //0 - normal, 1 - allFast; 2 - firstNormalOtherFast;
	
	inline static uintptr_t dtRegenRet = 0x0;
	inline static uintptr_t dtRegenJe = 0x0;
	inline static uintptr_t sdtTransformRet = 0x0;

	inline static uintptr_t dtTimerRet = 0x0;
	inline static uintptr_t dtDurationRet = 0x0;
	inline static uintptr_t emDanteDelayRet = 0x0;
	inline static uintptr_t emDanteDelayJmpRet = 0x0;
	inline static uintptr_t emDanteDelayJneRet = 0x0;
	static inline uintptr_t workRateRoyalRevengeRet = 0;
	static inline uintptr_t setWorkRateCall = 0;
	static inline uintptr_t sdtRegenRet = 0;
	static inline uintptr_t sdtRegenJmp = 0;
	static inline uintptr_t setRoyalReleaseRet = 0;
	static inline uintptr_t damageReactRet = 0;
	static inline uintptr_t setGuardReactionEmDante = 0;

	inline static float dtTimer = 5.0f;
	inline static float dtTimerStartPoint = 0.0f;
	inline static const float dtTimerMax = 29.9f;
	inline static float dtDuration = 35.0f;
	static inline float rgPower = 15100.0f;

	BossDanteSetup() = default;

	/*~BossDanteSetup()
	{
		PlSetActionData::new_pl_boss_action_event_unsub(new Events::EventHandler<BossDanteSetup, const std::array<char, PlSetActionData::ACTION_STR_LENGTH>&, uintptr_t>(this, &BossDanteSetup::on_boss_action_update));
	}*/

	static bool set_rg_action_asm(uintptr_t plDante);

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

	// void on_frame() override;

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override;
	// on_draw_debug_ui() is called when debug window shows up

	// void on_draw_debug_ui() override;


private:
	void init_check_box_info() override;
	std::unique_ptr<FunctionHook> m_dtregen_hook;
	std::unique_ptr<FunctionHook> m_sdttransform_hook;
	std::unique_ptr<FunctionHook> m_dttimer_hook;
	std::unique_ptr<FunctionHook> m_dtduration_hook;
	std::unique_ptr<FunctionHook> m_emdante_delay_hook;
	std::unique_ptr<FunctionHook> m_royal_revenge_delay_hook;
	std::unique_ptr<FunctionHook> m_sdt_regen_hook;
	std::unique_ptr<FunctionHook> m_set_royal_release_hook;
	std::unique_ptr<FunctionHook> m_set_guard_ract_hook;

	static gf::Vec3 get_char_pos(uintptr_t character);

	static inline std::random_device rd;

	static inline std::default_random_engine generator{rd()};

	static inline std::uniform_int_distribution<int> releaseDistrib{0, 1};
};

