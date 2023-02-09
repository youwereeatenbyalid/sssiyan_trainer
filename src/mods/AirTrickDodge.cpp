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

bool AirTrickDodge::move_check_asm(uintptr_t charPtr, bool &groundFitCheat)
{
	if (!cheaton || !groundFitCheat)
		return false;
	if (charPtr == 0 || (*(int*)(charPtr + 0x108) != 0) || (*(int*)(charPtr + 0xE64) != 4))
		return false;
	for (const auto &str : trickNames)
	{
		if(PlSetActionData::cmp_real_cur_action(str))
			return true;
	}
	return false;
}

std::optional<std::string> AirTrickDodge::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = Page_VergilTrick;
	m_full_name_string = "Air Trick Dodge (+)";
	m_author_string = "V.P.Zadov & The HitchHiker";
	m_description_string = "Allow trick dodges while airborne.";
	set_up_hotkey();

	auto isInAirAddr = m_patterns_cache->find_addr(base, "0F B6 D0 48 8B 43 50 48 8B 48 18 48 85 C9 75 63 85 D2 0F 84 01"); //DevilMayCry5.exe+55088C
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
	 isNoInertia = cfg.get<bool>("AirTrickDodge.IsNoInertia").value_or(true);
	 isOverwriteGroundFitLength = cfg.get<bool>("AirTrickDodge.isOverwriteGroundFitLength").value_or(true);
 }

 void AirTrickDodge::on_config_save(utility::Config& cfg)
 {
	 cfg.set<bool>("AirTrickDodge.IsNoInertia", isNoInertia);
	 cfg.set<bool>("AirTrickDodge.isOverwriteGroundFitLength", isOverwriteGroundFitLength);
 }

// void AirTrickDodge::on_frame(){}

 void AirTrickDodge::on_draw_ui()
 {
	 ImGui::Checkbox("Remove trick inertia", &isNoInertia);
	 ImGui::ShowHelpMarker("Removes Inertia from directional air tricks. Disabling this can result in vergil performing \"Super jumps\" when directionally tricking.");
	 ImGui::Checkbox("Remove height forward dodge restriction", &isOverwriteGroundFitLength);
 }

// void AirTrickDodge::on_draw_debug_ui(){}

 void AirTrickDodge::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}