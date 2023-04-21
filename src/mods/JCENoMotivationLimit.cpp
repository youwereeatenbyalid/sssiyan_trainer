#include "JCENoMotivationLimit.hpp"
#include "BossTrickUp.hpp"
//clang-format off

static naked void no_motivation_detour()
{
	__asm {
		cmp byte ptr [JCENoMotivationLimit::cheaton], 1
		je cheat

		originalcode:
		cmp dword ptr [rdx + 0x00001B58], 02
		jmp qword ptr [JCENoMotivationLimit::humanConcRet]

		cheat:
		jmp qword ptr [JCENoMotivationLimit::humanConcSkip]
	}
}

static naked void no_motivation1_detour()
{
	__asm {
		cmp byte ptr [JCENoMotivationLimit::cheaton], 1
		je cheat

		originalcode:
		cmp dword ptr [rdx+0x00001B58], 02
		jmp qword ptr [JCENoMotivationLimit::devilConcRet]

		cheat:
		jmp qword ptr [JCENoMotivationLimit::devilConcSkip]
	}
}

bool JCENoMotivationLimit::check_mods_settnigs_asm(uintptr_t vergil)
{
	if (BossTrickUp::cheaton)
		return true;
	if (*(int*)(vergil + 0x1978) == 1 && BossVergilMoves::cheaton && _mod->_bossMovesMod->is_air_raid_enabled())
		return true;
	return false;
}

static naked void no_motivation2_detour()
{
	__asm {
		cmp byte ptr [JCENoMotivationLimit::cheaton], 1
		je cheat

		//check for other mods conditions:
		push rax
		push rbx
		push rcx
		mov rcx, rbx
		sub rsp, 32
		call qword ptr [JCENoMotivationLimit::check_mods_settnigs_asm]
		add rsp, 32
		cmp al, 1
		pop rcx
		pop rbx
		pop rax
		je cheat

		originalcode:
		cmp dword ptr [rbx+0x00001B58], 02
		jmp qword ptr [JCENoMotivationLimit::yamatoHumanConcRet]

		cheat:
		jmp qword ptr [JCENoMotivationLimit::yamatoHumanConcSkip]
	}
}

static naked void no_motivation3_detour()
{
	__asm {
		cmp byte ptr [JCENoMotivationLimit::cheaton], 1
		je cheat

		originalcode:
		cmp dword ptr [rbx+0x00001B58], 02
		jmp qword ptr [JCENoMotivationLimit::yamatoSdtConcRet]

		cheat:
		jmp qword ptr [JCENoMotivationLimit::yamatoSdtConcSkip]
	}
}

void JCENoMotivationLimit::after_all_inits()
{
	_bossMovesMod = static_cast<BossVergilMoves*>(g_framework->get_mods()->get_mod("BossVergilMoves"));
}

std::optional<std::string> JCENoMotivationLimit::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

	m_is_enabled = &cheaton;
	m_on_page = Page_VergilCheat;
	m_full_name_string = "No Concentration Restriction On JCE";
	m_author_string = "akasha51";
	m_description_string = "Allow JCE at any concentration level.";

	auto humanConcetrationAddr = m_patterns_cache->find_addr(base, "83 BA 58 1B 00 00 02 0F 85 F9 00 00 00 48 85 D2 74 7A F3 0F 10 8A 20 1B 00 00 F3 0F 10 05 0E A5 24 06 0F 5A C9 0F 5A C0");//"DevilMayCry5.exe"+1C0A520
	if (!humanConcetrationAddr)
	{
		return "Unable to find JCENoMotivationLimit.humanConcetrationAddr pattern.";
	}

	auto devilConcetrationAddr = g_framework->get_module().as<uintptr_t>() + 0x1C0A5CD;//patterns->find_addr(base, "5F C3 83 BA 58 1B 00 00 02 75 50 45 33 C0 48 8B CB 48 85 D2 74 22");//DevilMayCry5.exe+1C0A5CD
	/*if (!devilConcetrationAddr)
	{
		return "Unable to find JCENoMotivationLimit.devilConcetrationAddr pattern.";
	}*/

	auto yamatoConcetrationAddr = m_patterns_cache->find_addr(base, "83 BB 58 1B 00 00 02 75");//DevilMayCry5.exe+54F45E
	if (!yamatoConcetrationAddr)
	{
		return "Unable to find JCENoMotivationLimit.yamatoConcetrationAddr pattern.";
	}

	auto yamatoSdtConcetrationAddr = m_patterns_cache->find_addr(base, "83 BB 58 1B 00 00 02 0F 85 76 FF FF FF 48 8B 5C 24 30");//DevilMayCry5.exe+54F495
	if (!yamatoSdtConcetrationAddr)
	{
		return "Unable to find JCENoMotivationLimit.yamatoSdtConcetrationAddr pattern.";
	}

	humanConcSkip = humanConcetrationAddr.value() + 0xD;
	devilConcSkip = devilConcetrationAddr/*.value() + 0x2*/ + 0x9;
	yamatoHumanConcSkip = yamatoConcetrationAddr.value() + 0x9;
	yamatoSdtConcSkip = yamatoSdtConcetrationAddr.value() + 0xD;

	if (!install_new_detour(humanConcetrationAddr.value(), m_humnan_motivation_detour, &no_motivation_detour, &humanConcRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize JCENoMotivationLimit.humanConcetration";
	}

	if (!install_new_detour(devilConcetrationAddr/*.value() + 0x2*/, m_devil_motivation_detour, &no_motivation1_detour, &devilConcRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize JCENoMotivationLimit.devilConcetration";
	}

	if (!install_new_detour(yamatoConcetrationAddr.value(), m_yamato_motivation_detour, &no_motivation2_detour, &yamatoHumanConcRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize JCENoMotivationLimit.yamatoConcetration";
	}

	if (!install_new_detour(yamatoSdtConcetrationAddr.value(), m_sdtyamato_motivation_detour, &no_motivation3_detour, &yamatoSdtConcRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize JCENoMotivationLimit.yamatoSdtConcetration";
	}

	return Mod::on_initialize();
}

// void JCENoMotivationLimit::on_config_load(const utility::Config& cfg){}

// void JCENoMotivationLimit::on_config_save(utility::Config& cfg){}

// void JCENoMotivationLimit::on_frame(){}

// void JCENoMotivationLimit::on_draw_ui(){}

// void JCENoMotivationLimit::on_draw_debug_ui(){}

void JCENoMotivationLimit::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
//clang-format on