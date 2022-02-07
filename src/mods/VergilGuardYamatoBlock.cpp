#include "VergilGuardYamatoBlock.hpp"

static naked void detour()
{
	__asm {
		cmp byte ptr [VergilGuardYamatoBlock::cheaton], 0
		je originalcode
		mov r8d, 0x29 //41, YamatoShotGuard

		originalcode:
		movss [rsp + 0x20], xmm0
		jmp [VergilGuardYamatoBlock::ret]
	}
}

std::optional<std::string> VergilGuardYamatoBlock::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	ischecked = &cheaton;
	onpage = vergilefxsettings;
	full_name_string = "Always Yamato shot guard";
	author_string = "VPZadov";
	description_string = "Always use Yamato shot guard animation while block.";

	auto setActionAddr = patterns.find_addr(base, "F3 0F 11 44 24 20 E8 F4 F3"); //DevilMayCry5.exe+5762A1
	if (!setActionAddr)
	{
		return "Unanable to find VergilGuardYamatoBlock.setActionAddr pattern.";
	}

	if (!install_hook_absolute(setActionAddr.value(), m_action_hook, detour, &ret, 0x6))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilGuardYamatoBlock.setAction";
	}


	return Mod::on_initialize();
}

void VergilGuardYamatoBlock::on_config_load(const utility::Config& cfg)
{
}

void VergilGuardYamatoBlock::on_config_save(utility::Config& cfg)
{
}

void VergilGuardYamatoBlock::on_frame()
{
}

void VergilGuardYamatoBlock::on_draw_ui()
{

}

void VergilGuardYamatoBlock::on_draw_debug_ui()
{
}

void VergilGuardYamatoBlock::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}