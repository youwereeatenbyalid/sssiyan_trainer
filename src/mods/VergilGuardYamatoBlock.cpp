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

static naked void efx_detour()
{
	__asm{
		cmp byte ptr [VergilGuardYamatoBlock::isSelectEfx], 1
		je cheat
		mov eax, [rdx + 0x00001B58]
		jmp qword ptr [VergilGuardYamatoBlock::efxRet]

		cheat:
		mov eax, dword ptr [VergilGuardYamatoBlock::concEfx]
		jmp qword ptr[VergilGuardYamatoBlock::efxRet]
	}
}

std::optional<std::string> VergilGuardYamatoBlock::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = Page_VergilVFXSettings;
	m_full_name_string = "DMC5 Shot Block (+)";
	m_author_string = "V.P.Zadov";
	m_description_string = "Change block to the gunshot block.";

	auto setActionAddr = m_patterns_cache->find_addr(base, "F3 0F 11 44 24 20 E8 F4 F3"); //DevilMayCry5.exe+5762A1
	if (!setActionAddr)
	{
		return "Unanable to find VergilGuardYamatoBlock.setActionAddr pattern.";
	}

	auto efxAddr = m_patterns_cache->find_addr(base, "C6 8B 82 58 1B 00 00"); //DevilMayCry5.exe+576138 (-0x1)
	if (!efxAddr)
	{
		return "Unanable to find VergilGuardYamatoBlock.efxAddr pattern.";
	}

	if (!install_new_detour(setActionAddr.value(), m_action_detour, detour, &ret, 0x6))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilGuardYamatoBlock.setAction";
	}

	if (!install_new_detour(efxAddr.value() + 0x1, m_efx_detour, efx_detour, &efxRet, 0x6))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilGuardYamatoBlock.efx";
	}

	return Mod::on_initialize();
}

void VergilGuardYamatoBlock::on_config_load(const utility::Config& cfg)
{
	isSelectEfx = cfg.get<bool>("VergilGuardYamatoBlock.isSelectEfx").value_or(false);
	concEfx = cfg.get<int>("VergilGuardYamatoBlock.concEfx").value_or(2);
}

void VergilGuardYamatoBlock::on_config_save(utility::Config& cfg)
{
	cfg.set<bool>("VergilGuardYamatoBlock.isSelectEfx", isSelectEfx);
	cfg.set<int>("VergilGuardYamatoBlock.concEfx", concEfx);
}

 void VergilGuardYamatoBlock::on_draw_ui()
 {
	 ImGui::Checkbox("Select block's hit efx", &isSelectEfx);
	 ImGui::ShowHelpMarker("By default block's hit efx power depends on concentration level: 0 - no efx, 1 - small flash, 2 - big flash and idk, some blur wave shit(?). "
	 "This option allow set \"efx level\" independently of current concentration level. Can be used even if main mod is disabled.");
	 if(isSelectEfx)
		UI::SliderInt("##concLvl", &concEfx, 0, 2, "%d", 1.0F, ImGuiSliderFlags_AlwaysClamp);
 }

void VergilGuardYamatoBlock::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}