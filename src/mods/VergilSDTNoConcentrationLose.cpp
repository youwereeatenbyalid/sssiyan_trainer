#include "VergilSDTNoConcentrationLose.hpp"

uintptr_t VergilSDTNoConcentrationLose::subDamageRet;

bool VergilSDTNoConcentrationLose::cheaton = false;

float VergilSDTNoConcentrationLose::newSubValue = 0;

static naked void subdamage_detour() {
	__asm {
		cmp byte ptr [VergilSDTNoConcentrationLose::cheaton], 0
		je originalcode

		cheat:
		cmp dword ptr [VergilSDTFormTracker::vergilform_state], 0x2
		jne originalcode
		movss xmm2, [VergilSDTNoConcentrationLose::newSubValue]
		jmp qword ptr [VergilSDTNoConcentrationLose::subDamageRet]

		originalcode:
		movss xmm2, [rax+0x0000008C]
		jmp qword ptr [VergilSDTNoConcentrationLose::subDamageRet]
	}
}


std::optional<std::string> VergilSDTNoConcentrationLose::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	ischecked = &cheaton;
	onpage = vergilcheat;
	full_name_string = "SDT no concentration lose";
	author_string = "VPZadov";
	description_string = "Don't decrease a concentration gauge when Vergil takes damage in SDT.";

	auto initAddr1 = patterns.find_addr(base, "2B FF FF FF F3 0F 10 90 8C 00 00 00");// DevilMayCry5.exe+572C5D
	if (!initAddr1) {
		return "Unanable to find VergilSDTNoConcentrationLose pattern.";
	}
	uintptr_t correctInitAddr = initAddr1.value() + 0x4;

	if (!install_hook_absolute(correctInitAddr, m_sub_concentration_damage_hook, &subdamage_detour, &subDamageRet, 0x8)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilSDTNoConcentrationLose.initAddr1";
	}

	return Mod::on_initialize();
}

void VergilSDTNoConcentrationLose::on_config_load(const utility::Config& cfg)
{
}

void VergilSDTNoConcentrationLose::on_config_save(utility::Config& cfg)
{
}

void VergilSDTNoConcentrationLose::on_frame()
{
}

void VergilSDTNoConcentrationLose::on_draw_ui()
{

}

void VergilSDTNoConcentrationLose::on_draw_debug_ui()
{}

void VergilSDTNoConcentrationLose::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}
