#include "SecretMissionTimer.hpp"

bool SecretMissionTimer::cheaton{false};
bool SecretMissionTimer::isM9NoStart{false};

uintptr_t SecretMissionTimer::timerRet;
uintptr_t SecretMissionTimer::m9Ret;
uintptr_t SecretMissionTimer::m9Jne;

float SecretMissionTimer::timer = 90.0f;

static naked void timer_detour()
{
	__asm {
		cmp byte ptr [SecretMissionTimer::cheaton], 1
		je cheat

		originalcode:
		movss xmm0, [rbx+0x5C]
		jmp qword ptr [SecretMissionTimer::timerRet]

		cheat:
		movss xmm0, [SecretMissionTimer::timer]
		movss [rbx+0x5C], xmm0
		jmp qword ptr [SecretMissionTimer::timerRet]
	}
}

static naked void m9_groundtimer_detour()
{
	__asm
	{
		cmp byte ptr [SecretMissionTimer::cheaton], 0
		je originalcode
		cmp byte ptr [SecretMissionTimer::isM9NoStart], 0
		je originalcode

		cheat:
		jmp qword ptr[SecretMissionTimer::m9Ret]

		originalcode:
		test rdx, rdx
		jne ret_jne
		jmp qword ptr [SecretMissionTimer::m9Ret]

		ret_jne:
		jmp qword ptr [SecretMissionTimer::m9Jne]
	}
}

std::optional<std::string> SecretMissionTimer::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

	ischecked = &cheaton;
	onpage = secretmission;
	full_name_string = "Freeze secret missions timer (+)";
	author_string = "VPZadov";
	description_string = "Also can prevent starting \"no ground\" timer on sm9.";

	auto timerAddr = patterns.find_addr(base, "F3 0F 10 43 5C 0F 5A C0 F2");// DevilMayCry5.exe+9473C0
	if (!timerAddr)
	{
		return "Unanable to find timerAddr pattern.";
	}

	auto m9Addr = patterns.find_addr(base, "3F 48 8B 56 28 48 8B CB 48 85 D2 75 1B");// DevilMayCry5.exe+1FA2CC9; +0x8
	if (!m9Addr)
	{
		return "Unanable to find m9Addr pattern.";
	}

	m9Jne = m9Addr.value() + 0x8 + 0x20;

	if (!install_hook_absolute(timerAddr.value(), m_timer_hook, &timer_detour, &timerRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize SecretMissionTimer.timer";
	}

	if (!install_hook_absolute(m9Addr.value()+0x8, m_m9_hook, &m9_groundtimer_detour, &m9Ret, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize SecretMissionTimer.timer";
	}

	return Mod::on_initialize();
}

void SecretMissionTimer::on_config_load(const utility::Config& cfg)
{
	timer = cfg.get<float>("SecretMissionTimer.timer").value_or(90.0f);
	isM9NoStart = cfg.get<bool>("SecretMissionTimer.isM9NoStart").value_or(true);
}

void SecretMissionTimer::on_config_save(utility::Config& cfg)
{
	cfg.set<float>("SecretMissionTimer.timer", timer);
	cfg.set<float>("SecretMissionTimer.isM9NoStart", isM9NoStart);
}

void SecretMissionTimer::on_frame()
{
}

void SecretMissionTimer::on_draw_ui()
{
	ImGui::TextWrapped("Timer value: ");
	ImGui::InputFloat("##timerInput", &timer, 0.0f, 0.0f, "%.2f");
	ImGui::Checkbox("Disable sm9 no ground start timer", &isM9NoStart);
}

void SecretMissionTimer::on_draw_debug_ui()
{
}

void SecretMissionTimer::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
