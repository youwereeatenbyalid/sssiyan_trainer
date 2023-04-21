#pragma once
#include "Mod.hpp"
#include "BossVergilMoves.hpp"
//clang-format off
class JCENoMotivationLimit : public Mod
{
public:
	static inline bool cheaton = false;

	static inline uintptr_t humanConcRet;
	static inline uintptr_t humanConcSkip;
	static inline uintptr_t devilConcRet;
	static inline uintptr_t devilConcSkip;
	static inline uintptr_t yamatoHumanConcRet;
	static inline uintptr_t yamatoHumanConcSkip;
	static inline uintptr_t yamatoSdtConcRet;
	static inline uintptr_t yamatoSdtConcSkip;


	JCENoMotivationLimit()
	{
		_mod = this;
	}

	std::string_view get_name() const override
	{
		return "Remove concentration restriction to perform JCE";
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
	// void on_config_load(const utility::Config& cfg) override;
	// void on_config_save(utility::Config& cfg) override;

	// on_frame() is called every frame regardless whether the gui shows up.
	// void on_frame() override;
	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	// void on_draw_ui() override;
	// on_draw_debug_ui() is called when debug window shows up
	// void on_draw_debug_ui() override;

	static bool check_mods_settnigs_asm(uintptr_t vergil);

private:
	void init_check_box_info() override;
	std::shared_ptr<Detour_t> m_humnan_motivation_detour;
	std::shared_ptr<Detour_t> m_devil_motivation_detour;
	std::shared_ptr<Detour_t> m_yamato_motivation_detour;
	std::shared_ptr<Detour_t> m_sdtyamato_motivation_detour;

	BossVergilMoves* _bossMovesMod = nullptr;

	static inline JCENoMotivationLimit* _mod = nullptr;

	void after_all_inits() override;
};
//clang-format on
