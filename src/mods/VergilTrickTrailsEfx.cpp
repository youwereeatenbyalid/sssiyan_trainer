#include "VergilTrickTrailsEfx.hpp"
#include "BossTrickUp.hpp"
#include "PlSetActionData.hpp"

void VergilTrickTrailsEfx::set_up_end_efx_asm(uintptr_t threadCtxt, uintptr_t vergil, bool setDrawSelf)
{
	BossTrickUp::set_appear_pos(threadCtxt, vergil);
	if (!cheaton || lastTrick == None || !setDrawSelf)
		return;
	for (auto &trick : trickActionsSettings)
	{
		if (lastTrick == trick->get_mode())
		{
			trick->request_end_efx(vergil);
			break;
		}
	}
}

void VergilTrickTrailsEfx::set_trick_efx(uintptr_t netStr, uintptr_t vergil)
{
	if (*(int*)(vergil + 0xE64) != 4)
		return;
	for (auto& mod : trickActionsSettings)
	{
		if (mod->cmp_move_name(netStr) && mod->is_enabled())
		{
			mod->request_efx(vergil);
			break;
		}
	}
}

static naked void trick_end_draw_self_detour()
{
	__asm {
		cmp byte ptr [VergilTrickTrailsEfx::cheaton], 1
		je cheat

		originalcode:
		call [VergilTrickTrailsEfx::airTrickEndDrawSelfCall]
		jmp qword ptr [VergilTrickTrailsEfx::airTrickEndRet]

		cheat:
		push rax
		push rcx
		push rdx
		push rsp
		push r8
		push r9
		push r10
		push r11
		sub rsp, 32
		call qword ptr [VergilTrickTrailsEfx::set_up_end_efx_asm]
		add rsp, 32
		pop r11
		pop r10
		pop r9
		pop r8
		pop rsp
		pop rdx
		pop rcx
		pop rax
		jmp originalcode
	}
}

static naked void action_set_draw_self_detour()
{
	__asm {
		cmp byte ptr[BossTrickUp::cheaton], 1
		je cheat
		cmp byte ptr [VergilTrickTrailsEfx::cheaton], 1
		je cheat

		originalcode:
		call [VergilTrickTrailsEfx::airTrickEndDrawSelfCall]
		jmp qword ptr [VergilTrickTrailsEfx::actionSetDrawSelfRet]

		cheat:
		push rax
		push rcx
		push rdx
		push rsp
		push r8
		push r9
		push r10
		push r11
		sub rsp, 32
		call qword ptr [VergilTrickTrailsEfx::set_up_end_efx_asm]
		add rsp, 32
		pop r11
		pop r10
		pop r9
		pop r8
		pop rsp
		pop rdx
		pop rcx
		pop rax
		jmp originalcode
	}
}

static naked void trickdodge_set_draw_self_detour()
{
	__asm {
		cmp byte ptr [VergilTrickTrailsEfx::cheaton], 01
		je cheat

		originalcode:
		call qword ptr [VergilTrickTrailsEfx::airTrickEndDrawSelfCall]
		jmp qword ptr[VergilTrickTrailsEfx::trickDodgeSetDrawSelfRet]

		cheat:
		push rax
		push rcx
		push rdx
		push rsp
		push r8
		push r9
		push r10
		push r11
		sub rsp, 32
		call qword ptr [VergilTrickTrailsEfx::set_up_end_efx_asm]
		add rsp, 32
		pop r11
		pop r10
		pop r9
		pop r8
		pop rsp
		pop rdx
		pop rcx
		pop rax
		jmp originalcode
	}
}

std::optional<std::string> VergilTrickTrailsEfx::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	auto pBase = g_framework->get_module().as<uintptr_t>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = Page_VergilVFXSettings;
	m_full_name_string = "Tricks efx settings (+)";
	m_author_string = "V.P.Zadov";
	m_description_string = "Add and customize some efx on trick moves without changing the default efxs. Changing any efx with filemods "
		"will also affect what this mod creates. If you want to recreate boss Vergil's trick effects - Lordranis'es trick alternate or my recolor file mods are a must for the boss's trick smoke cloud.";

	set_up_hotkey();

	auto endAirTrickAddr = m_patterns_cache->find_addr(base, "E8 47 3B 50 FF");//DevilMayCry5.exe+1DDF054
	if (!endAirTrickAddr)
	{
		return "Unable to find VergilTrickTrailsEfx.endAirTrickAddr pattern.";
	}

	auto actionEndSetDrawSeflAddr = m_patterns_cache->find_addr(base, "E8 75 BD 68 00");//DevilMayCry5.exe+C56E26
	if (!actionEndSetDrawSeflAddr)
	{
		return "Unable to find VergilTrickTrailsEfx.actionEndSetDrawSeflAddr pattern.";
	}

	auto trickDodgeEnableDrawSelfAddr = m_patterns_cache->find_addr(base, "E8 A2 39 30 FF");//DevilMayCry5.exe+1FDF1F9
	if (!trickDodgeEnableDrawSelfAddr)
	{
		return "Unable to find VergilTrickTrailsEfx.trickDodgeEnableDrawSelfAddr pattern.";
	}

	airTrickEndDrawSelfCall = m_patterns_cache->find_addr(base, "48 89 5C 24 10 48 89 6C 24 18 56 48 83 EC 20 48 8B 41 50 41 0F").value_or(pBase + 0x12E2BA0);

	if (!install_new_detour(endAirTrickAddr.value(), m_air_trick_end_hook, &trick_end_draw_self_detour, &airTrickEndRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilTrickTrailsEfx.endAirTrick";
	}

	if (!install_new_detour(actionEndSetDrawSeflAddr.value(), m_trick_set_draw_self_hook, &action_set_draw_self_detour, &actionSetDrawSelfRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilTrickTrailsEfx.actionEndSetDrawSefl";
	}

	if (!install_new_detour(trickDodgeEnableDrawSelfAddr.value(), m_trickdodge_set_draw_self_hook, &trickdodge_set_draw_self_detour, &trickDodgeSetDrawSelfRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilTrickTrailsEfx.trickDodgeEnableDrawSelf";
	}

	efxList[0] = new EfxInfo{ BossStartTrails, 1, 12, "Boss's start trick trails", true, true, -2.0f, 0 };
	efxList[1] = new EfxInfo{ SmallTrails, 0, 20, "Rapid slash'es or yamato helm breaker's trails", true, true, 0.5f, 0 };
	efxList[2] = new EfxInfo{ M18WindAndDust, 3, 3, "Some dust and wind", false, false, 0, 0 };
	efxList[3] = new EfxInfo{ BlueFlame, 4, 47, "(M)FE unused blue flame efx", true, true, 0, 0 };
	efxList[4] = new EfxInfo{ SDTTrickDodge, 300, 17, "Player's SDT trick dodge", true, true, 0.95f, 0 };
	efxList[5] = new EfxInfo{ SDTAirTrick, 300, 27, "Player's SDT air trick", false, false, -0.5f, 0 };
	efxList[6] = new EfxInfo{ PlBlurTrick, 1, 17, "Player's blur when trick", false, true, 0, 0 };
	efxList[7] = new EfxInfo{ JceEndTeleport, 10, 33, "JCE end teleport", true, true, 0, 0 };

	return Mod::on_initialize();
}

void VergilTrickTrailsEfx::on_config_load(const utility::Config& cfg)
{
	for(auto &trick : trickActionsSettings)
		trick->load_data(cfg);
}

void VergilTrickTrailsEfx::on_config_save(utility::Config& cfg)
{
	for (auto& trick : trickActionsSettings)
		trick->save_data(cfg);
}

void VergilTrickTrailsEfx::on_frame()
{
}

void VergilTrickTrailsEfx::on_draw_ui()
{
	for (const auto &trick : trickActionsSettings)
	{
		trick->print_spec_settings();
		ImGui::Spacing();
		trick->print_fx_list_selection();
		ImGui::Separator();
	}
}

void VergilTrickTrailsEfx::on_draw_debug_ui()
{
}

void VergilTrickTrailsEfx::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
