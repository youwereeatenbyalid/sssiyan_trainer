#include "DanteAirTrickSettings.hpp"

bool DanteAirTrickSettings::cheaton{false};
bool DanteAirTrickSettings::isAddZOffset{false};
bool DanteAirTrickSettings::isNoDistanceRestriction{false};
bool DanteAirTrickSettings::groundIsNoDistanceRestriction{false};
float DanteAirTrickSettings::addZOffset = 0.0f;

uintptr_t DanteAirTrickSettings::distanceRet0{NULL};
uintptr_t DanteAirTrickSettings::distanceRet1{NULL};
uintptr_t DanteAirTrickSettings::offsetRet{NULL};

static naked void distance0_detour() {
	__asm {
		cmp byte ptr [DanteAirTrickSettings::cheaton], 0
		je originalcode
		cmp byte ptr [DanteAirTrickSettings::isNoDistanceRestriction], 0
		je originalcode

		cheat:
		movss xmm1, [DanteAirTrickSettings::TRICK_DISTANCE]
		jmp qword ptr[DanteAirTrickSettings::distanceRet0]


		originalcode:
		movss xmm1, [rdx+0x10]
		jmp qword ptr [DanteAirTrickSettings::distanceRet0]
	}
}

static naked void distance1_detour() {
	__asm {
		cmp byte ptr [DanteAirTrickSettings::cheaton], 0
		je originalcode
		cmp byte ptr [DanteAirTrickSettings::isNoDistanceRestriction], 0
		je originalcode

		cheat:
		movss xmm0, [DanteAirTrickSettings::TRICK_DISTANCE]
		jmp qword ptr [DanteAirTrickSettings::distanceRet1]

		originalcode:
		movss xmm0, [r8+0x10]
		jmp qword ptr [DanteAirTrickSettings::distanceRet1]
	}
}

static naked void offset_detour() {
	__asm {
		cmp byte ptr [DanteAirTrickSettings::cheaton], 0
		je originalcode
		cmp byte ptr [DanteAirTrickSettings::isAddZOffset], 0
		je originalcode

		cheat:
		movss xmm1, [DanteAirTrickSettings::addZOffset]
		jmp qword ptr [DanteAirTrickSettings::offsetRet]

		originalcode:
		movss xmm1, [rdi+0x68]
		jmp qword ptr [DanteAirTrickSettings::offsetRet]
	}
}

std::optional<std::string> DanteAirTrickSettings::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = Page_DanteCheat;
	m_full_name_string = "Trick Settings (+)";
	m_author_string = "V.P.Zadov";
	m_description_string = "Adjust the properties of Dante's Trick Actions.";

  set_up_hotkey();

	auto distanceAddr0 = m_patterns_cache->find_addr(base, "F3 0F 10 4A 10 0F 57 C0 F3 41 0F 5A C2"); //DevilMayCry5.exe+F24790
	if (!distanceAddr0) {
		return "Unanable to find DanteAirTrickSettings.distanceAddr0 pattern.";
	}

	auto distanceAddr1 = m_patterns_cache->find_addr(base, "F3 41 0F 10 40 10 0F 5A C0 48 85 C0"); //DevilMayCry5.exe+F256F3
	if (!distanceAddr1) {
		return "Unanable to find DanteAirTrickSettings.distanceAddr1 pattern.";
	}

	auto offsetAddr = m_patterns_cache->find_addr(base, "F3 0F 10 4F 68 48 8D 47"); //DevilMayCry5.exe+F25034
	if (!offsetAddr) {
		return "Unanable to find DanteAirTrickSettings.offsetAddr pattern.";
	}

	if (!install_new_detour(distanceAddr0.value(), m_airtrick_distance0_detour, &distance0_detour, &distanceRet0, 0x5)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DanteAirTrickSettings.distance0";
	}

	if (!install_new_detour(distanceAddr1.value(), m_airtrick_distance1_detour, &distance1_detour, &distanceRet1, 0x6)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DanteAirTrickSettings.distance1";
	}

	if (!install_new_detour(offsetAddr.value(), m_airtrick_offset_detour, &offset_detour, &offsetRet, 0x5)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DanteAirTrickSettings.offset";
	}

    return Mod::on_initialize();
}

void DanteAirTrickSettings::on_config_load(const utility::Config& cfg)
{
	isAddZOffset = cfg.get<bool>("DanteAirTrickSettings.isAddZOffset").value_or(false);
	isNoDistanceRestriction = cfg.get<bool>("DanteAirTrickSettings.isNoDistanceRestriction").value_or(true);
	groundIsNoDistanceRestriction = cfg.get<bool>("DanteAirTrickSettings.groundIsNoDistanceRestriction").value_or(true);
	addZOffset = cfg.get<float>("DanteAirTrickSettings.addZOffset").value_or(0.0f);
}

void DanteAirTrickSettings::on_config_save(utility::Config& cfg)
{
	cfg.set<bool>("DanteAirTrickSettings.isAddZOffset", isAddZOffset);
	cfg.set<bool>("DanteAirTrickSettings.isNoDistanceRestriction", isNoDistanceRestriction);
	cfg.set<bool>("DanteAirTrickSettings.groundIsNoDistanceRestriction", groundIsNoDistanceRestriction);
	cfg.set<float>("DanteAirTrickSettings.addZOffset", addZOffset);
}

// void DanteAirTrickSettings::on_frame(){}

void DanteAirTrickSettings::on_draw_ui()
{
	ImGui::Checkbox("Air Trick infinite range", &isNoDistanceRestriction);
	ImGui::Checkbox("Ground Trick infinite range", &groundIsNoDistanceRestriction);
	ImGui::Separator();
	ImGui::Checkbox("Adjust Air Trick Height", &isAddZOffset);
	if (isAddZOffset) {
		ImGui::TextWrapped("Additional height above enemy when reappearing:");
		UI::SliderFloat("##OffssetSlider", &addZOffset, 0.0f, 10.0f, "%.01f");
	}
}

// void DanteAirTrickSettings::on_draw_debug_ui(){}

void DanteAirTrickSettings::init_check_box_info() {
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
