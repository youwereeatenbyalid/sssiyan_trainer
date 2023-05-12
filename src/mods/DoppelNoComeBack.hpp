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

	std::shared_ptr<Detour_t> m_distance_doppel_detour;
	std::shared_ptr<Detour_t> m_bwjust_doppel_detour;
	std::shared_ptr<Detour_t> m_fejust_doppel_detour;

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
		m_full_name_string = "Disable doppelganger auto-return (+)";
		m_author_string = "V.P.Zadov";
		m_description_string = "Disable the doppelganger automatically returning after a just JdC or when it gets too far away from Vergil.";

		set_up_hotkey();

		auto doppelDistanceCumBackAddr = m_patterns_cache->find_addr(base, "66 0F 2F C8 77 1C 48");//DevilMayCry5.exe+528AEA
		if (!doppelDistanceCumBackAddr)
		{
			return "Unable to find DoppelNoComeBack.doppelDistanceComeBackAddr pattern.";
		}
		//.text:0000000140528C5F	app_PlayerVergilPL__updateDoppel113891	call    app_PlayerVergilPL__comeBackDoppelGanger113978
		//tu6 aob E8 CC 51 02 00
		//tu7 aob 48 8B D7 E8 ? ? ? ? 48 8B 43 50 48 83 78 ? ? 75 1F 48 8B 97 ? ? ? ? +0x3
		auto doppeBwJustCumBackAddr = m_patterns_cache->find_addr(base, "48 8B D7 E8 ? ? ? ? 48 8B 43 50 48 83 78 ? ? 75 1F 48 8B 97 ? ? ? ? ");//DevilMayCry5.exe+528C5F
		if (!doppeBwJustCumBackAddr)
		{
			return "Unable to find DoppelNoComeBack.doppeBwJustCumBackAddr pattern.";
		}
		//.text:0000000140528D6D	app_PlayerVergilPL__updateDoppel113891	call    app_PlayerVergilPL__comeBackDoppelGanger113978
		//tu6 aob E8 BE 50 02 00
		//tu7 aob E8 ? ? ? ? 48 8B 43 50 48 83 78 ? ? 0F 85 ? ? ? ? 48 8B 87 ? ? ? ? 48 8B 97 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 0F B6 88 ? ? ? ? 
		auto doppeFeJustCumBackAddr = m_patterns_cache->find_addr(base, "E8 ? ? ? ? 48 8B 43 50 48 83 78 ? ? 0F 85 ? ? ? ? 48 8B 87 ? ? ? ? 48 8B 97 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 0F B6 88 ? ? ? ? ");//DevilMayCry5.exe+528D6D
		if (!doppeFeJustCumBackAddr)
		{
			return "Unable to find DoppelNoComeBack.doppeFeJustCumBackAddr pattern.";
		}

		jaJmp = doppelDistanceCumBackAddr.value() + 0x22;
		auto doppelComeBackFuncAddr = m_patterns_cache->find_addr(base, "48 89 5C 24 18 57 48 81 EC 90 00 00 00 48 8B 41 50 48");
		if(!doppelComeBackFuncAddr) 
		{
			return "Unable to find DoppelNoComeBack.doppelComeBackFuncAddr pattern.";
		}
		doppelComeBackFunc = doppelComeBackFuncAddr.value();

		if (!install_new_detour(doppelDistanceCumBackAddr.value(), m_distance_doppel_detour, &distance_detour, &distanceRet, 0x6))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize DoppelNoComeBack.doppelDistanceComeBack";
		}

		if (!install_new_detour(doppeBwJustCumBackAddr.value()+0x3, m_bwjust_doppel_detour, &bw_just_detour, &bwJustRet, 0x5))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize DoppelNoComeBack.doppeBwJustComemBack";
		}

		if (!install_new_detour(doppeFeJustCumBackAddr.value(), m_fejust_doppel_detour, &fe_just_detour, &feJustRet, 0x5))
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
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override
	{
		ImGui::Checkbox("Disable auto-return after traveling too far away from Vergil", &byDistance);
		ImGui::Checkbox("Disable auto-return after a just JdC.", &byJC);
	}
};
//clang-format on