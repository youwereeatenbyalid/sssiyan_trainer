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
		/*cmp byte ptr [TrickDodgeNoDisappear::isAuto], 1
		je spec_check*/
		movss xmm0, [TrickDodgeNoDisappear::endDrawOffMod]
		jmp qword ptr [TrickDodgeNoDisappear::ret]

		/*spec_check:
		cmp byte ptr [TrickDodgeNoDisappear::_isTrickStopped], 1
		je zero_frame
		movss xmm0, [TrickDodgeNoDisappear::specDrawOff]
		jmp qword ptr [TrickDodgeNoDisappear::ret]

		zero_frame:
		movss xmm0, [TrickDodgeNoDisappear::zeroDrawOff]
		jmp qword ptr [TrickDodgeNoDisappear::ret]*/
	}
}

std::optional<std::string> TrickDodgeNoDisappear::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = Page_VergilVFXSettings;
	m_full_name_string = "Change invisibility duration on Trick Dodge(+)";
	m_author_string = "V.P.Zadov";
	m_description_string = "Extend or complitely remove invisibility while trick dodging.";

	auto getEndDrawOffAddr = m_patterns_cache->find_addr(base, "33 F3 0F 10 47 64"); //DevilMayCry5.exe+1FDF1A1 (-0x1)
	if (!getEndDrawOffAddr)
	{
		return "Unanable to find TrickDodgeNoDisappear.getEndDrawOffAddr pattern.";
	}

	if (!install_new_detour(getEndDrawOffAddr.value() + 0x1, m_disappear_detour, &draw_off_detour, &ret, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize TrickDodgeNoDisappear.getEndDrawOff";
	}

	return Mod::on_initialize();
}

 void TrickDodgeNoDisappear::on_config_load(const utility::Config& cfg)
 {
	 //isAuto = cfg.get<bool>("TrickDodgeNoDisappear.isAuto").value_or(false);
	 endDrawOffMod = cfg.get<float>("TrickDodgeNoDisappear.endDrawOffMod").value_or(0);
 }

 void TrickDodgeNoDisappear::on_config_save(utility::Config& cfg)
 {
	 //cfg.set<bool>("TrickDodgeNoDisappear.isAuto", isAuto);
	 cfg.set<float>("TrickDodgeNoDisappear.endDrawOffMod", endDrawOffMod);
 }

// void TrickDodgeNoDisappear::on_frame(){}

 void TrickDodgeNoDisappear::on_draw_ui()
 {
	 //ImGui::Checkbox("Automatically set duration based on trick move duration", &isAuto);
	 //ImGui::ShowHelpMarker("This option works correctly only when Position Action Editor mod enabled and trick dodge speed changed.");
	 //if (!isAuto)
	 //{
	ImGui::TextWrapped("Frames after Vergil's model will be visible:");
	UI::SliderFloat("##frameDrawOff", &endDrawOffMod, 0, 360.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
	 //}	
 }

// void TrickDodgeNoDisappear::on_draw_debug_ui(){}

void TrickDodgeNoDisappear::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
