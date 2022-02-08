#include "GroundTrickNoDistanceRestriction.hpp"

bool GroundTrickNoDistanceRestriction::cheaton{false};
uintptr_t GroundTrickNoDistanceRestriction::distanceRet0{NULL};
uintptr_t GroundTrickNoDistanceRestriction::distanceRet1{NULL};

static naked void distance0_detour() {
	__asm {
		cmp byte ptr [GroundTrickNoDistanceRestriction::cheaton], 1
		je cheat

		originalcode:
		movss xmm0, [rcx+0x14]
		jmp qword ptr [GroundTrickNoDistanceRestriction::distanceRet0]

		cheat:
		movss xmm0, [GroundTrickNoDistanceRestriction::TRICK_DISTANCE]
		jmp qword ptr [GroundTrickNoDistanceRestriction::distanceRet0]
	}
}

static naked void distance1_detour() {
	__asm {
		cmp byte ptr [GroundTrickNoDistanceRestriction::cheaton], 0x1
		je cheat

		originalcode:
		movss xmm1, [rax+0x14]
		jmp qword ptr [GroundTrickNoDistanceRestriction::distanceRet1]

		cheat:
		movss xmm1, [GroundTrickNoDistanceRestriction::TRICK_DISTANCE]
		jmp qword ptr [GroundTrickNoDistanceRestriction::distanceRet1]
	}
}

std::optional<std::string> GroundTrickNoDistanceRestriction::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = dantecheat;
	m_full_name_string = "Ground Trick no distance restriction";
	m_author_string = "VPZadov";
	m_description_string = "Increase max distance of Ground Trick.";

  set_up_hotkey();

	auto distanceAddr0 = patterns->find_addr(base, "F3 0F 10 41 14 48 8B 4F"); //DevilMayCry5.exe+10BFE80
	if (!distanceAddr0) {
		return "Unanable to find GroundTrickNoDistanceRestriction.distanceAddr0 pattern.";
	}

	auto distanceAddr1 = patterns->find_addr(base, "74 C0 F3 0F 10 48 14"); //DevilMayCry5.exe+10C18E8
	if (!distanceAddr1) {
		return "Unanable to find GroundTrickNoDistanceRestriction.distanceAddr1 pattern.";
	}

	if (!install_hook_absolute(distanceAddr0.value(), m_groundtrick0_hook, &distance0_detour, &distanceRet0, 0x5)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize GroundTrickNoDistanceRestriction.distance0";
	}

	if (!install_hook_absolute(distanceAddr1.value()+0x2, m_groundtrick1_hook, &distance1_detour, &distanceRet1, 0x5)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize GroundTrickNoDistanceRestriction.distance1";
	}

    return Mod::on_initialize();
}

void GroundTrickNoDistanceRestriction::on_config_load(const utility::Config& cfg)
{
}

void GroundTrickNoDistanceRestriction::on_config_save(utility::Config& cfg)
{
}

void GroundTrickNoDistanceRestriction::on_frame()
{
}

void GroundTrickNoDistanceRestriction::on_draw_ui()
{
}

void GroundTrickNoDistanceRestriction::on_draw_debug_ui()
{
}

void GroundTrickNoDistanceRestriction::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}
