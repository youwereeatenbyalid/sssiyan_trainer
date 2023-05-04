#pragma once
#include "Mod.hpp"
#include "GameFunctions/PlVergilDoppel.hpp"
#include "VergilDoppelInitSetup.hpp"

//clang-format off
namespace gf = GameFunctions;
class InstantDoppel : public Mod
{
public:
	InstantDoppel() = default;

	~InstantDoppel()
	{
		VergilDoppelInitSetup::on_doppel_summon_unsub(std::make_shared<Events::EventHandler<InstantDoppel, uintptr_t, VergilDoppelInitSetup::DoppelDelayState, bool*>>(this, &InstantDoppel::on_doppel_summon));
	}

	static inline bool cheaton = true;

	std::string_view get_name() const override
	{
		return "InstantDoppel";
	}
	std::string get_checkbox_name() override
	{
		return m_check_box_name;
	};
	std::string get_hotkey_name() override
	{
		return m_hot_key_name;
	};

	std::optional<std::string> on_initialize() override
	{
		init_check_box_info();
		auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
		m_is_enabled = &cheaton;
		m_on_page = Page_VergilDoppel;
		m_depends_on = { "VergilDoppelInitSetup" };
		m_full_name_string = "Summon doppelganger instantly (+)";
		m_author_string = "V.P.Zadov";
		m_description_string = "Remove Vergil's i-frame DT activation motion and summon doppelganger instantly (doppel himself still have appears motion)";

		set_up_hotkey();

		VergilDoppelInitSetup::on_doppel_summon_sub(std::make_shared<Events::EventHandler<InstantDoppel, uintptr_t, VergilDoppelInitSetup::DoppelDelayState, bool*>>(this, &InstantDoppel::on_doppel_summon));

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override 
	{
		isControlledBySpeedState = cfg.get<bool>("InstantDoppel.isControlledBySpeedState").value_or(false);
		delay = (VergilDoppelInitSetup::DoppelDelayState)cfg.get<int>("InstantDoppel.Delay").value_or(1);
	}
	void on_config_save(utility::Config& cfg) override 
	{
		cfg.set<bool>("InstantDoppel.isControlledBySpeedState", isControlledBySpeedState);
		cfg.set<int>("InstantDoppel.Delay", delay);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override 
	{
		ImGui::TextWrapped("Select this if you want use instant summon only with specific doppel's delay setup:");
		ImGui::Checkbox("Instant summon when summon via", &isControlledBySpeedState);
		bool isSelected = false;
		if(ImGui::BeginCombo("##doppelDelay", delayNames[delay - 1]))
		{
			for (int i = 1; i <= delayNames.size(); i++)
			{
				isSelected = (delay == (VergilDoppelInitSetup::DoppelDelayState)i);
				if(ImGui::Selectable(delayNames[i-1], isSelected))
					delay = (VergilDoppelInitSetup::DoppelDelayState)i;
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

private:
	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	const std::array<char*, 3> delayNames {"Fast mode", "Default mode", "Slow mode"};

	static inline VergilDoppelInitSetup::DoppelDelayState delay = VergilDoppelInitSetup::DoppelDelayState::Fast;

	static inline bool isControlledBySpeedState = false;

	void on_doppel_summon(uintptr_t vergil, VergilDoppelInitSetup::DoppelDelayState doppelSpeed, bool *skipSummon)
	{
		if (!cheaton || *(bool*)(vergil + 0x18A8) || *(float*)(vergil + 0x1110) < 3000.0f)
			return;
		if (isControlledBySpeedState)
		{
			if (doppelSpeed != InstantDoppel::delay)
			{
				*skipSummon = false;
				return;
			}
		}
		*(int*)(vergil + 0x18C8) = *(int*)(vergil + 0x18CC) = VergilDoppelInitSetup::DoppelDelayState::Fast;
		//*(bool*)(vergil + 0x18A8) = true;
		gf::SetDoppelMode setDoppel{ vergil };
		gf::GenerateDoppel genDoppel{ vergil };
		setDoppel(true);
		genDoppel(gf::Vec3(0, 1.2f, 0), false);
		*skipSummon = true;
	}

};
//clang-format on