#include "BossDanteSetup.hpp"
//clang-format off

static naked void dtregen_detour()
{
	__asm {
		cmp byte ptr [BossDanteSetup::cheaton], 0
		je originalcode
		cmp byte ptr[BossDanteSetup::isDtRegenEnabled], 0
		je originalcode
		jmp qword ptr [BossDanteSetup::dtRegenRet]

		originalcode:
		cmp byte ptr [rsi+0x20], 00
		je ret_je
		jmp qword ptr [BossDanteSetup::dtRegenRet]

		ret_je:
		jmp qword ptr [BossDanteSetup::dtRegenJe]
	}
}

static naked void sdt_transform_detour()
{
	__asm {
		cmp byte ptr [BossDanteSetup::cheaton], 0
		je originalcode
		cmp byte ptr [BossDanteSetup::isSdtTransformSetup], 0
		je originalcode

		cheat:
		mov ecx, dword ptr [BossDanteSetup::sdtTransformMode]
		jmp ret_jmp

		originalcode:
		mov ecx, [rcx+0x10]

		ret_jmp:
		test ecx, ecx
		jmp qword ptr [BossDanteSetup::sdtTransformRet]
	}
}

static naked void dt_sdt_timer_detour()
{
	__asm {
		cmp byte ptr [BossDanteSetup::cheaton], 0
		je originalcode
		cmp byte ptr [BossDanteSetup::isDtsTimerSkip], 0
		je originalcode

		cheat:
		movss xmm0, xmm1
		comiss xmm0, [BossDanteSetup::dtTimerMax]
		ja originalcode
		comiss xmm0, [BossDanteSetup::dtTimer]
		jb originalcode
		movss xmm1, [BossDanteSetup::dtTimerMax]

		originalcode:
		movss [rdx+0x18], xmm1
		jmp qword ptr [BossDanteSetup::dtTimerRet]
	}
}

static naked void dt_duration_detour()
{
	__asm {
		cmp byte ptr [BossDanteSetup::cheaton], 0
		je originalcode
		cmp byte ptr [BossDanteSetup::isCustomDtDuration], 0
		je originalcode

		cheat:
		movss xmm0, [BossDanteSetup::dtDuration]
		jmp qword ptr [BossDanteSetup::dtDurationRet]

		originalcode:
		movss xmm0, [rax+0x14]
		jmp qword ptr [BossDanteSetup::dtDurationRet]
	}
}

static naked void motion_delay_detour()
{
	__asm {
		cmp byte ptr [BossDanteSetup::cheaton], 0
		je originalcode
		cmp byte ptr [BossDanteSetup::isNoMovesDelay], 0
		je originalcode

		cheat:
		cmp byte ptr [rdi + 0x1AD8], 0//isEmDante, check just in case
		je originalcode
		jmp qword ptr [BossDanteSetup::emDanteDelayJmpRet]

		originalcode:
		test rax, rax
		jne jne_ret 
		jmp qword ptr [BossDanteSetup::emDanteDelayRet]

		jne_ret:
		jmp qword ptr [BossDanteSetup::emDanteDelayJneRet]
	}
}

std::optional<std::string> BossDanteSetup::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = balance;
	m_full_name_string = "Boss Dante settings (+)";
	m_author_string = "VPZadov";
	m_description_string = "Vergil's nightmare begins here.";

	auto dtAddr = utility::scan(base, "80 7E 20 00 0F 84 01 02 00 00"); // DevilMayCry5.exe+19C34A9
	if (!dtAddr)
	{
		return "Unanable to find BossDanteSetup.dtAddr pattern.";
	}

	auto sdtTransformAddr = utility::scan(base, "8B 49 10 85 C9 74 35"); // DevilMayCry5.exe+19C637C
	if (!sdtTransformAddr)
	{
		return "Unanable to find BossDanteSetup.sdtTransformAddr pattern.";
	}

	auto dtTimerAddr = utility::scan(base, "F3 0F 11 4A 18 48 8B"); // DevilMayCry5.exe+318477
	if (!dtTimerAddr)
	{
		return "Unanable to find BossDanteSetup.dtTimerAddr pattern.";
	}

	auto dtDurationAddr = utility::scan(base, "F3 0F 10 40 14 48 8B 43 50 0F 5A C8"); // DevilMayCry5.exe+19C304E
	if (!dtDurationAddr)
	{
		return "Unanable to find BossDanteSetup.dtDurationAddr pattern.";
	}

	auto charUpdateDelayAddr = utility::scan(base, "32 48 85 C0 75 10"); // DevilMayCry5.exe+19BD0C4 (0x1)
	if (!charUpdateDelayAddr)
	{
		return "Unanable to find BossDanteSetup.charUpdateDelayAddr pattern.";
	}

	dtRegenJe = dtAddr.value() + 0x20B;
	emDanteDelayJmpRet = charUpdateDelayAddr.value() + 0x37 + 0x1;
	emDanteDelayJneRet = charUpdateDelayAddr.value() + 0x15 + 0x1;

	if (!install_hook_absolute(dtAddr.value(), m_dtregen_hook, &dtregen_detour, &dtRegenRet, 0xA))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize BossDanteSetup.dt";
	}

	if (!install_hook_absolute(sdtTransformAddr.value(), m_sdttransform_hook, &sdt_transform_detour, &sdtTransformRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize BossDanteSetup.sdtTransform";
	}

	if (!install_hook_absolute(dtTimerAddr.value(), m_dttimer_hook, &dt_sdt_timer_detour, &dtTimerRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize BossDanteSetup.dtTimer";
	}

	if (!install_hook_absolute(dtDurationAddr.value(), m_dtduration_hook, &dt_duration_detour, &dtDurationRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize BossDanteSetup.dtDuration";
	}

	if (!install_hook_absolute(charUpdateDelayAddr.value()+ 0x1, m_emdante_delay_hook, &motion_delay_detour, &emDanteDelayRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize BossDanteSetup.charUpdateDelay";
	}

	return Mod::on_initialize();
}

void BossDanteSetup::on_config_load(const utility::Config& cfg)
{
	isDtRegenEnabled = cfg.get<bool>("BossDanteSetup.isDtRegenEnabled").value_or(true);
	isSdtTransformSetup = cfg.get<bool>("BossDanteSetup.isSdtTransformSetup").value_or(true);
	isDtsTimerSkip = cfg.get<bool>("BossDanteSetup.isDtsTimerSkip").value_or(true);
	isCustomDtDuration = cfg.get<bool>("BossDanteSetup.isCustomDtDuration").value_or(true);
	isNoFinishSdtStun = cfg.get<bool>("BossDanteSetup.isNoFinishSdtStun").value_or(true);
	isNoMovesDelay = cfg.get<bool>("BossDanteSetup.isNoMovesDelay").value_or(true);
	sdtTransformMode = cfg.get<uint32_t>("BossDanteSetup.sdtTransformMode").value_or(0);
	dtTimer = cfg.get<float>("BossDanteSetup.dtTimer").value_or(15.0f);
	dtDuration = cfg.get<float>("BossDanteSetup.dtDuration").value_or(40.0f);
}

void BossDanteSetup::on_config_save(utility::Config& cfg)
{
	cfg.set<bool>("BossDanteSetup.isDtRegenEnabled", isDtRegenEnabled);
	cfg.set<bool>("BossDanteSetup.isSdtTransformSetup", isSdtTransformSetup);
	cfg.set<bool>("BossDanteSetup.isDtsTimerSkip", isDtsTimerSkip);
	cfg.set<bool>("BossDanteSetup.isCustomDtDuration", isCustomDtDuration);
	cfg.set<bool>("BossDanteSetup.isNoFinishSdtStun", isNoFinishSdtStun);
	cfg.set<bool>("BossDanteSetup.isNoMovesDelay", isNoMovesDelay);
	cfg.set<uint32_t>("BossDanteSetup.sdtTransformMode", sdtTransformMode);
	cfg.set<float>("BossDanteSetup.dtTimer", dtTimer);
	cfg.set<float>("BossDanteSetup.dtDuration", dtDuration);
}

void BossDanteSetup::on_frame()
{
}

void BossDanteSetup::on_draw_ui()
{
	ImGui::Checkbox("Enable DT HP regen", &isDtRegenEnabled);
	ImGui::Checkbox("Custom SDT transform setup", &isSdtTransformSetup);
	if (isSdtTransformSetup)
	{
		ImGui::Spacing();
		ImGui::TextWrapped("Change transform mode to: 0 - always normal transform, 1 - always fast transform (default), 2 - first normal other fast (idk is this one even working?)");
		ImGui::SliderInt("##TransformMode", (int*)&sdtTransformMode, 0, 2);
	}
	ImGui::Checkbox("Modify dt & sdt timers", &isDtsTimerSkip);
	if(isDtsTimerSkip)
	{
		ImGui::TextWrapped("Reactivation DT/SDT timer (depends of current mission):");
		ImGui::SliderFloat("##customTimerDelay", &dtTimer, 0.1f, 30.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
		//dtTimerStartPoint = 30.0f - dtTimer + 0.08f;
	}
	ImGui::Checkbox("Custom dt duration", &isCustomDtDuration);
	if (isCustomDtDuration)
	{
		ImGui::TextWrapped("Set dt duration on m19 (game's default - 15):");
		ImGui::InputFloat("##dtDurationSlider", &dtDuration, 1.0f, 2.0f, "%.1f");
	}
	ImGui::Checkbox("No stun after end of SDT;", &isNoFinishSdtStun);
	ImGui::Checkbox("Disable boss moves delay;", &isNoMovesDelay);
}

void BossDanteSetup::on_draw_debug_ui()
{
}

void BossDanteSetup::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
//clamg-format on
