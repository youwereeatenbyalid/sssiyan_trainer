#pragma once
#include "Mod.hpp"
#include "PosActionEditor.hpp"
#include "FsmPlPosCtrActionStartHooks.hpp"
#include "PlSetActionData.hpp"
#include "EndLvlHooks.hpp"
#include "Mods.hpp"

class TrickDodgeNoDisappear : public Mod, private EndLvlHooks::IEndLvl
{
public:

	inline static bool cheaton = false;
	inline static bool isAuto = false;
	inline static bool isTrickStopped = true;

	static inline uintptr_t ret = 0;
	
	static inline float endDrawOffMod = 0;
	static inline float specDrawOff = 1000.0f;
	static inline float zeroDrawOff = 0;

	TrickDodgeNoDisappear()
	{
		PlSetActionData::new_action_event_sub((std::make_shared<Events::EventHandler<TrickDodgeNoDisappear, const std::array<char,PlSetActionData::ACTION_STR_LENGTH>*, 
			uintptr_t, uintptr_t, uintptr_t>>(this, &TrickDodgeNoDisappear::on_posctrl_move_started)));
	}

	~TrickDodgeNoDisappear()
	{
		auto posActionEditor = static_cast<PosActionEditor*>(g_framework->get_mods()->get_mod("PosActionEditor"));
		if (posActionEditor != nullptr)
			posActionEditor->speed_sign_changed_unsub((std::make_shared<Events::EventHandler<TrickDodgeNoDisappear, const PosActionEditor::MoveSpeedData*>>(this, &TrickDodgeNoDisappear::on_trick_dodge_end)));
		PlSetActionData::new_action_event_unsub((std::make_shared<Events::EventHandler<TrickDodgeNoDisappear, const std::array<char, PlSetActionData::ACTION_STR_LENGTH>*, uintptr_t, 
			uintptr_t, uintptr_t>>(this, &TrickDodgeNoDisappear::on_posctrl_move_started)));
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
		isTrickStopped = true;
	}

	void on_trick_dodge_end(const PosActionEditor::MoveSpeedData* data)
	{
		if(strcmp(data->get_game_move_name(), "TrickEscape_Front") || strcmp(data->get_game_move_name(), "TrickEscape_Back") || strcmp(data->get_game_move_name(), "TrickEscape_Left") || strcmp(data->get_game_move_name(), "TrickEscape_Right"))
			isTrickStopped = true;
	}

	void on_posctrl_move_started(const std::array<char, 65> *moveName, uintptr_t threadCntxt, uintptr_t str, uintptr_t pl)
	{
		if (pl != 0 && *(int*)(pl + 0xE64) == 4)
		{
			if (PlSetActionData::cmp_real_cur_action("TrickEscape_Front") || PlSetActionData::cmp_real_cur_action("TrickEscape_Back") || PlSetActionData::cmp_real_cur_action("TrickEscape_Left") || PlSetActionData::cmp_real_cur_action("TrickEscape_Right"))
				isTrickStopped = false;
		}
		else
			isTrickStopped = true;
	}

	void init_check_box_info() override;
	std::unique_ptr<FunctionHook> m_disappear_hook;
};

