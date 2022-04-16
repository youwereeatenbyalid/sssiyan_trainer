#pragma once
#include "ImGuiExtensions/ImGuiExtensions.h"
#include "Mod.hpp"

class DoppelNoComeBack/*noCUMBack))00)))00)))00)))*/ : public Mod
{
private:

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	std::unique_ptr<FunctionHook> m_distance_doppel_hook;
	std::unique_ptr<FunctionHook> m_bwjust_doppel_hook;
	std::unique_ptr<FunctionHook> m_fejust_doppel_hook;

public:
	DoppelNoComeBack() = default;

	static inline bool cheaton = false;
	static inline bool byDistance = false;
	static inline bool byJC = false;

	static inline uintptr_t distanceRet = 0;
	static inline uintptr_t jaJmp = 0;
	static inline uintptr_t bwJustRet = 0;
	static inline uintptr_t feJustRet = 0;

	static inline uintptr_t doppelComeBackFunc = 0;


	static naked void distance_detour()
	{
		__asm {
			cmp byte ptr [DoppelNoComeBack::cheaton], 0
			je originalcode
			cmp byte ptr [DoppelNoComeBack::byDistance], 1
			je cheat

			originalcode:
			comisd xmm1, xmm0
			ja cheat
			jmp qword ptr [DoppelNoComeBack::distanceRet]

			cheat:
			jmp qword ptr [DoppelNoComeBack::jaJmp]
		}
	}

	static naked void bw_just_detour()
	{
		__asm {
			cmp byte ptr [DoppelNoComeBack::cheaton], 0
			je originalcode
			cmp byte ptr [DoppelNoComeBack::byJC], 1
			je cheat

			originalcode:
			call qword ptr [DoppelNoComeBack::doppelComeBackFunc]
			jmp qword ptr [DoppelNoComeBack::bwJustRet]

			cheat:
			jmp qword ptr [DoppelNoComeBack::bwJustRet]
		}
	}

	static naked void fe_just_detour()
	{
		__asm {
			cmp byte ptr [DoppelNoComeBack::cheaton], 0
			je originalcode
			cmp byte ptr [DoppelNoComeBack::byJC], 1
			je cheat

			originalcode:
			call qword ptr [DoppelNoComeBack::doppelComeBackFunc]
			jmp qword ptr [DoppelNoComeBack::feJustRet]

			cheat:
			jmp qword ptr [DoppelNoComeBack::feJustRet]
		}
	}

	std::string_view get_name() const override
	{
		return "DoppelNoComeBack";
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
		m_full_name_string = "Doppelganger no auto come back";
		m_author_string = "VPZadov";
		m_description_string = "Disable doppelganger's auto come back based of distance restriction or after perfect JC.";

		set_up_hotkey();

		auto doppelDistanceCumBackAddr = m_patterns_cache->find_addr(base, "66 0F 2F C8 77 1C 48");//DevilMayCry5.exe+528AEA
		if (!doppelDistanceCumBackAddr)
		{
			return "Unable to find DoppelNoComeBack.doppelDistanceComeBackAddr pattern.";
		}

		auto doppeBwJustCumBackAddr = m_patterns_cache->find_addr(base, "E8 CC 51 02 00");//DevilMayCry5.exe+528C5F
		if (!doppeBwJustCumBackAddr)
		{
			return "Unable to find DoppelNoComeBack.doppeBwJustCumBackAddr pattern.";
		}

		auto doppeFeJustCumBackAddr = m_patterns_cache->find_addr(base, "E8 BE 50 02 00");//DevilMayCry5.exe+528D6D
		if (!doppeBwJustCumBackAddr)
		{
			return "Unable to find DoppelNoComeBack.doppeFeJustCumBackAddr pattern.";
		}

		jaJmp = doppelDistanceCumBackAddr.value() + 0x22;
		doppelComeBackFunc = m_patterns_cache->find_addr(base, "48 89 5C 24 18 57 48 81 EC 90 00 00 00 48 8B 41 50 48").value_or(g_framework->get_module().as<uintptr_t>() + 0x54DE30);

		if (!install_hook_absolute(doppelDistanceCumBackAddr.value(), m_distance_doppel_hook, &distance_detour, &distanceRet, 0x6))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize DoppelNoComeBack.doppelDistanceComeBack";
		}

		if (!install_hook_absolute(doppeBwJustCumBackAddr.value(), m_bwjust_doppel_hook, &bw_just_detour, &bwJustRet, 0x5))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize DoppelNoComeBack.doppeBwJustComemBack";
		}

		if (!install_hook_absolute(doppeFeJustCumBackAddr.value(), m_fejust_doppel_hook, &fe_just_detour, &feJustRet, 0x5))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize DoppelNoComeBack.doppeFeJustComemBack";
		}

		//Yamato just jc hook already in doppelWeaponSwitcher

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override
	{
		byDistance = cfg.get<bool>("DoppelNoComeBack.byDistance").value_or(true);
		byJC = cfg.get<bool>("DoppelNoComeBack.byJC").value_or(false);
	}
	void on_config_save(utility::Config& cfg) override
	{
		cfg.set<bool>("DoppelNoComeBack.byDistance", byDistance);
		cfg.set<bool>("DoppelNoComeBack.byJC", byJC);
		ImGui::ShowHelpMarker("This screw up doppel's prefect jc after FE helm breaker if doppel was in air :(");
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override
	{
		ImGui::Checkbox("By distance", &byDistance);
		ImGui::Checkbox("By perfect JC", &byJC);
	}
};
//clang-format on