#include "JCENoMotivationLimit.hpp"
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

static naked void no_motivation2_detour()
{
	__asm {
		cmp byte ptr [JCENoMotivationLimit::cheaton], 1
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

std::optional<std::string> JCENoMotivationLimit::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

	m_is_enabled = &cheaton;
	m_on_page = vergilcheat;
	m_full_name_string = "No concentration restriction on JCE";
	m_author_string = "akasha51";
	m_description_string = "Allow JCE at any concentration level.";

	auto humanConcetrationAddr = patterns->find_addr(base, "83 BA 58 1B 00 00 02 0F 85 F9 00 00 00 48 85 D2 74 7A F3 0F 10 8A 20 1B 00 00 F3 0F 10 05 0E A5 24 06 0F 5A C9 0F 5A C0");//"DevilMayCry5.exe"+1C0A520
	if (!humanConcetrationAddr)
	{
		return "Unable to find JCENoMotivationLimit.humanConcetrationAddr pattern.";
	}

	auto devilConcetrationAddr = g_framework->get_module().as<uintptr_t>() + 0x1C0A5CD;//patterns->find_addr(base, "5F C3 83 BA 58 1B 00 00 02 75 50 45 33 C0 48 8B CB 48 85 D2 74 22");//DevilMayCry5.exe+1C0A5CD
	/*if (!devilConcetrationAddr)
	{
		return "Unable to find JCENoMotivationLimit.devilConcetrationAddr pattern.";
	}*/

	auto yamatoConcetrationAddr = patterns->find_addr(base, "83 BB 58 1B 00 00 02 75");//DevilMayCry5.exe+54F45E
	if (!yamatoConcetrationAddr)
	{
		return "Unable to find JCENoMotivationLimit.yamatoConcetrationAddr pattern.";
	}

	auto yamatoSdtConcetrationAddr = patterns->find_addr(base, "83 BB 58 1B 00 00 02 0F 85 76 FF FF FF 48 8B 5C 24 30");//DevilMayCry5.exe+54F495
	if (!yamatoSdtConcetrationAddr)
	{
		return "Unable to find JCENoMotivationLimit.yamatoSdtConcetrationAddr pattern.";
	}

	humanConcSkip = humanConcetrationAddr.value() + 0xD;
	devilConcSkip = devilConcetrationAddr/*.value() + 0x2*/ + 0x9;
	yamatoHumanConcSkip = yamatoConcetrationAddr.value() + 0x9;
	yamatoSdtConcSkip = yamatoSdtConcetrationAddr.value() + 0xD;

	if (!install_hook_absolute(humanConcetrationAddr.value(), m_humnan_motivation_hook, &no_motivation_detour, &humanConcRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize JCENoMotivationLimit.humanConcetration";
	}

	if (!install_hook_absolute(devilConcetrationAddr/*.value() + 0x2*/, m_devil_motivation_hook, &no_motivation1_detour, &devilConcRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize JCENoMotivationLimit.devilConcetration";
	}

	if (!install_hook_absolute(yamatoConcetrationAddr.value(), m_yamato_motivation_hook, &no_motivation2_detour, &yamatoHumanConcRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize JCENoMotivationLimit.yamatoConcetration";
	}

	if (!install_hook_absolute(yamatoSdtConcetrationAddr.value(), m_sdtyamato_motivation_hook, &no_motivation3_detour, &yamatoSdtConcRet, 0x7))
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