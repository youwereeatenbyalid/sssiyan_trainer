#pragma once
#include "Mod.hpp"

//clang-format off

class DanteSDTRegen : public Mod
{
private:

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	std::shared_ptr<Detour_t> m_dante_dt_update;
	std::shared_ptr<Detour_t> m_pl_update_dt_hp_rec;

public:
	DanteSDTRegen() = default;

	static inline bool cheaton = true;

	static inline uintptr_t updateRet = 0;
	static inline uintptr_t plUpdateRet = 0;

	static naked void update_detour()
	{
		__asm {
			cmp byte ptr [DanteSDTRegen::cheaton], 1
			je cheat

			originalcode:
			cmp eax, 01
			jne ret_jne
			jmp qword ptr [DanteSDTRegen::updateRet]

			ret_jne:
			ret

			cheat:
			cmp dword ptr [rdx + 0x9B0], 2
			jne originalcode
			jmp qword ptr[DanteSDTRegen::updateRet]
		}
	}

	static naked void update_pl_detour()
	{
		__asm {
			cmp byte ptr [DanteSDTRegen::cheaton], 1
			je originalcode

			cheat:
			cmp dword ptr [rdi + 0xE64], 1
			jne originalcode
			mov al, 1

			originalcode:
			movzx ecx, al
			mov rax, [rbx + 0x50]
			jmp qword ptr [DanteSDTRegen::plUpdateRet]
		}
	}

	std::string_view get_name() const override
	{
		return "DanteSDTRegen";
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
		m_on_page = Page_DanteSDT;
		m_full_name_string = "SDT HP regeneration";
		m_author_string = "V.P.Zadov";
		m_description_string = "HP regenerates in SDT like it does in DT.";

		set_up_hotkey();
		//DevilMayCry5.app_PlayerDante__updateDevilTriggerHpRecovery110218 +0x25
		auto updateDtHpAddr = m_patterns_cache->find_addr(base, "80 BA ? ? ? ? ? 75 26 48 8B 41 50");// DevilMayCry5.exe+196FE35
		if (!updateDtHpAddr)
		{
			return "Unanable to find DanteSDTRegen.updateDtHpAddr pattern.";
		}

		auto plUpdateDtHpAddr = m_patterns_cache->find_addr(base, "D0 0F B6 C8 48 8B 43 50 48 83 78 18 00 0F 85 E8 01");// DevilMayCry5.exe+16A26E1 (-0x1)
		if (!updateDtHpAddr)
		{
			return "Unanable to find DanteSDTRegen.plUpdateDtHpAddr pattern.";
		}

		//updateJne = updateDtHpAddr.value() + 0xF;

		if (!install_new_detour(updateDtHpAddr.value()+0x25, m_dante_dt_update, &update_detour, &updateRet, 0x5))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize DanteSDTRegen.updateDtHp";
		}

		if (!install_new_detour(plUpdateDtHpAddr.value() + 0x1, m_pl_update_dt_hp_rec, &update_pl_detour, &plUpdateRet, 0x7))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize DanteSDTRegen.plUpdateDtHp";
		}

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override
	{
	}
	void on_config_save(utility::Config& cfg) override
	{
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override
	{
	}
};
//clang-format on