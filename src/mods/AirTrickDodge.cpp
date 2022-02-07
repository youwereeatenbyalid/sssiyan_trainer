#include "AirTrickDodge.hpp"

static naked void detour()
{
	__asm {
		cmp byte ptr [AirTrickDodge::cheaton], 0
		je originalcode
		mov al, 1

		originalcode:
		movzx edx, al
		mov rax, [rbx + 0x50]
		jmp qword ptr [AirTrickDodge::ret]
	}
}

std::optional<std::string> AirTrickDodge::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	ischecked = &cheaton;
	onpage = vergiltrick;
	full_name_string = "Air trick dodge";
	author_string = "VPZadov";
	description_string = "Allow use trick dodge in the air.";

	auto isInAirAddr = patterns.find_addr(base, "0F B6 D0 48 8B 43 50 48 8B 48 18 48 85 C9 75 63 85 D2 0F 84 01"); //DevilMayCry5.exe+55088C
	if (!isInAirAddr)
	{
		return "Unanable to find AirTrickDodge.isInAirAddr pattern.";
	}

	if (!install_hook_absolute(isInAirAddr.value(), m_aircheck_hook, &detour, &ret, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirTrickDodge.isInAir";
	}

	return Mod::on_initialize();
}

void AirTrickDodge::on_config_load(const utility::Config& cfg)
{
}

void AirTrickDodge::on_config_save(utility::Config& cfg)
{
}

void AirTrickDodge::on_frame()
{
}

void AirTrickDodge::on_draw_ui()
{

}

void AirTrickDodge::on_draw_debug_ui()
{
}

void AirTrickDodge::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}