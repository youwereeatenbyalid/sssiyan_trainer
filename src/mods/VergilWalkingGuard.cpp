#include "VergilWalkingGuard.hpp"
#include "PlayerTracker.hpp"

static naked void is_pad_input_detuor()
{
	__asm {
		cmp byte ptr [VergilWalkingGuard::cheaton], 0
		je originalcode

		cheat:
		cmp byte ptr [VergilWalkingGuard::skipInSdt], 1
		je sdtskip
		mov al, 1
		jmp originalcode

		sdtskip:
		cmp [PlayerTracker::vergilentity], 0
		je originalcode
		push rbx
		mov rbx, [PlayerTracker::vergilentity]
		mov ebx, dword ptr [rbx + 0x09B0]
		cmp ebx, 2
		pop rbx
		je originalcode
		mov al, 1

		originalcode:
		movzx ecx, al
		mov rax, [rbx + 0x50]
		jmp qword ptr [VergilWalkingGuard::ret]
	}
}

std::optional<std::string> VergilWalkingGuard::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = vergiltrick;
	m_full_name_string = "Walking guard (+)";
	m_author_string = "VPZadov";
	m_description_string = "Vergil can perform block while he moving.";

	auto isPadInputAddr = utility::scan(base, "5B 41 00 0F B6 C8 48 8B 43 50"); //DevilMayCry5.exe+171128A (-0x3)
	if (!isPadInputAddr)
	{
		return "Unanable to find VergilWalkingGuard.isPadInputAddr pattern.";
	}

	if (!install_hook_absolute(isPadInputAddr.value() + 0x3, m_is_pad_input_hook, &is_pad_input_detuor, &ret, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilWalkingGuard.isPadInput";
	}

	return Mod::on_initialize();
}

void VergilWalkingGuard::on_config_load(const utility::Config& cfg)
{
	skipInSdt = cfg.get<bool>("VergilWalkingGuard.skipInSdt").value_or(true);
}

void VergilWalkingGuard::on_config_save(utility::Config& cfg)
{
	cfg.set<bool>("VergilWalkingGuard.skipInSdt", skipInSdt);
}

void VergilWalkingGuard::on_frame()
{
}

void VergilWalkingGuard::on_draw_ui()
{
	ImGui::Checkbox("Use default behavior while in SDT", &skipInSdt);
}

void VergilWalkingGuard::on_draw_debug_ui()
{
}

void VergilWalkingGuard::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}