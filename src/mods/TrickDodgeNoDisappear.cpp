#include "TrickDodgeNoDisappear.hpp"

static naked void draw_off_detour()
{
	__asm
	{
		cmp byte ptr [TrickDodgeNoDisappear::cheaton], 1
		je cheat

		originalcode:
		movss xmm0, [rdi + 0x64]
		jmp qword ptr [TrickDodgeNoDisappear::ret]

		cheat:
		movss xmm0, [TrickDodgeNoDisappear::endDrawOffMod]
		jmp qword ptr [TrickDodgeNoDisappear::ret]
	}
}

std::optional<std::string> TrickDodgeNoDisappear::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = vergilefxsettings;
	m_full_name_string = "Trick dodge no disappear";
	m_author_string = "VPZadov";
	m_description_string = "Vergil's body will be not disappear while trick dodge.";

	auto getEndDrawOffAddr = patterns->find_addr(base, "33 F3 0F 10 47 64"); //DevilMayCry5.exe+1FDF1A1 (-0x1)
	if (!getEndDrawOffAddr)
	{
		return "Unanable to find TrickDodgeNoDisappear.getEndDrawOffAddr pattern.";
	}

	if (!install_hook_absolute(getEndDrawOffAddr.value() + 0x1, m_disappear_hook, &draw_off_detour, &ret, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize TrickDodgeNoDisappear.getEndDrawOff";
	}

	return Mod::on_initialize();
}

// void TrickDodgeNoDisappear::on_config_load(const utility::Config& cfg){}

// void TrickDodgeNoDisappear::on_config_save(utility::Config& cfg){}

// void TrickDodgeNoDisappear::on_frame(){}

// void TrickDodgeNoDisappear::on_draw_ui(){}

// void TrickDodgeNoDisappear::on_draw_debug_ui(){}

void TrickDodgeNoDisappear::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
