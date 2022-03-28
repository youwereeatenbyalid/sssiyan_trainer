#include "VergilTrickTrailsEfx.hpp"

void VergilTrickTrailsEfx::set_up_end_efx_asm(uintptr_t vergil)
{
	if(lastTrick == None)
		return;
	if (vergil != PlayerTracker::vergilentity)
	{
		if(vergil != PlayerTracker::doppelentity)
			return;
	}
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
	if ( ( gf::StringController::str_cmp(netStr, "AirTrick_Enemy") || gf::StringController::str_cmp(netStr, "GrimTrick") ) && trickActionsSettings[0]->is_enabled())
		trickActionsSettings[0]->request_efx(vergil);
	else if(gf::StringController::str_cmp(netStr, "TrickUp"))
		trickActionsSettings[1]->request_efx(vergil);
	else if (gf::StringController::str_cmp(netStr, "TrickDown"))
		trickActionsSettings[2]->request_efx(vergil);
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
		mov rcx, rdx
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
		mov rcx, rdx
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
	m_on_page = vergilvfxsettings;
	m_full_name_string = "Tricks efx settings (+)";
	m_author_string = "VPZadov";
	m_description_string = /*"Add boss trails to trick efx and setup rotation for different trick actions. Fu capcom. Part of Lordranis's trick mod or recolor still is a must if you want cool smoke cloud.\n"
	"This mod used boss Vergil's trails data that loading to player Vergil also (always has been), so all changes to boss trails will also affect trails that this mod create. "
	"Mod doesn't disable default game's trick efx.";*/
	"Add and cusomize some efx (like 1 of boss's trick trails that been on plVergil all this time ty cumpcom) to trick moves without changing default efx itself. Changing any of efx by filemods "
	"will also affect what this mod spawns. If you want to recreate only boss trick - Lordranis'es trick alternate or my recolor file mods is a must for cool boss's trick smoke cloud.";

	set_up_hotkey();

	trickEfxID = std::make_shared<gf::GameModelRequestSetEffect::EffectID>(1, 12);
	trickEfxIDReverse = std::make_shared<gf::GameModelRequestSetEffect::EffectID>(1, 12);

	airTrickEndDrawSelfCall = pBase + 0x12E2BA0;

	auto endAirTrickAddr = patterns->find_addr(base, "E8 47 3B 50 FF");//DevilMayCry5.exe+1DDF054
	if (!endAirTrickAddr)
	{
		return "Unable to find VergilTrickTrailsEfx.endAirTrickAddr pattern.";
	}

	auto actionEndSetDrawSeflAddr = patterns->find_addr(base, "E8 75 BD 68 00");//DevilMayCry5.exe+C56E26
	if (!endAirTrickAddr)
	{
		return "Unable to find VergilTrickTrailsEfx.actionEndSetDrawSeflAddr pattern.";
	}

	if (!install_hook_absolute(endAirTrickAddr.value(), m_air_trick_end_hook, &trick_end_draw_self_detour, &airTrickEndRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilTrickTrailsEfx.endAirTrick";
	}

	if (!install_hook_absolute(actionEndSetDrawSeflAddr.value(), m_trick_set_draw_self_hook, &action_set_draw_self_detour, &actionSetDrawSelfRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilTrickTrailsEfx.actionEndSetDrawSefl";
	}

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
