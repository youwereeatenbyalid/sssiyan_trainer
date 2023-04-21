#pragma once
#include "Mod.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h"
#include "PlSetActionData.hpp"
#include "PlayerTracker.hpp"

class AirTrickDodge :  public Mod
{
public:
	static inline bool cheaton = false;
	static inline bool isOverwriteGroundFitLength = true;
	static inline bool isNoInertia = false;

	static inline uintptr_t ret = 0;

	AirTrickDodge()
	{
		PlayerTracker::pl_on_fsm2_pos_cntr_action_update_speed_sub(std::make_shared<Events::EventHandler<AirTrickDodge, uintptr_t, uintptr_t>>(this, &AirTrickDodge::on_fsm_pos_cntr_update));
	}

	~AirTrickDodge()
	{
		PlayerTracker::pl_on_fsm2_pos_cntr_action_update_speed_unsub(std::make_shared<Events::EventHandler<AirTrickDodge, uintptr_t, uintptr_t>>(this, &AirTrickDodge::on_fsm_pos_cntr_update));
	}

	std::string_view get_name() const override
	{
		return "AirTrickDodge";
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
	static inline constexpr std::array<const char*, 4> _trickNames
	{
		"TrickEscape_Front",
		"TrickEscape_Back",
		"TrickEscape_Left",
		"TrickEscape_Right"
	};

	void on_fsm_pos_cntr_update(uintptr_t threadCntx, uintptr_t fsm2PlPosCntrAction);

	void init_check_box_info() override;
	std::shared_ptr<Detour_t> m_aircheck_detour;
	std::shared_ptr<Detour_t> m_is_air_detour; // unused?
};

