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

static naked void revenge_delay_detour()
{
	__asm {
		cmp byte ptr[BossDanteSetup::cheaton], 0
		je originalcode
		cmp byte ptr[BossDanteSetup::isNoMovesDelay], 0
		je originalcode
		jmp qword ptr [BossDanteSetup::workRateRoyalRevengeRet]

		originalcode:
		call qword ptr [BossDanteSetup::setWorkRateCall]
		jmp qword ptr [BossDanteSetup::workRateRoyalRevengeRet]
	}
}

static naked void sdt_regen_detour()
{
	__asm {
		cmp byte ptr [BossDanteSetup::cheaton], 0
		je originalcode
		cmp byte ptr [BossDanteSetup::isSdtRegen], 0
		je originalcode

		jmp qword ptr [BossDanteSetup::sdtRegenJmp]

		originalcode:
		cmp dword ptr [rdi + 0x9B0], 01
		jmp qword ptr [BossDanteSetup::sdtRegenRet]
	}
}

static naked void set_release_detour()
{
	__asm {
		cmp byte ptr [BossDanteSetup::cheaton], 0
		je originalcode
		cmp byte ptr [BossDanteSetup::allowRoyalRelease], 0
		je originalcode

		mov rbx, [rcx + 0x20]//Dante
		test rbx, rbx
		je originalcode
		cmp byte ptr [rbx + 0x1AD8], 0 //isEmDante
		je originalcode
		mov dword ptr [rcx + 0x18], 2
		movss xmm0, [BossDanteSetup::rgPower]
		movss [rcx + 0x28], xmm0

		originalcode:
		cmp [rcx + 0x18], eax
		mov rbx, [rsp + 0x30]
		jmp qword ptr [BossDanteSetup::setRoyalReleaseRet]
	}
}

static naked void set_damage_react_detour()
{
	__asm {
		cmp byte ptr [BossDanteSetup::cheaton], 0
		je originalcode
		cmp byte ptr [BossDanteSetup::allowRoyalRelease], 0
		je originalcode

		push rax
		push rbx
		push rcx
		push rdx
		push rdi
		push rsp
		push r8
		push r9
		push r10
		push r11
		mov rcx, rdx
		sub rsp, 32
		call qword ptr [BossDanteSetup::set_rg_action_asm]
		add rsp, 32
		pop r11
		pop r10
		pop r9
		pop r8
		pop rsp
		pop rdi
		pop rdx
		pop rcx
		pop rbx
		cmp al, 0
		pop rax
		je skip

		originalcode:
		call qword ptr [BossDanteSetup::setGuardReactionEmDante]
		skip:
		jmp qword ptr [BossDanteSetup::damageReactRet]
	}
}

gf::Vec3 BossDanteSetup::get_char_pos(uintptr_t character)
{
	auto gameObj = *(uintptr_t*)(character + 0x10);
	auto transform = *(uintptr_t*)(gameObj + 0x18);
	return *(gf::Vec3*)(transform + 0x30);
}

bool BossDanteSetup::set_rg_action_asm(uintptr_t plDante)
{
	if(plDante == 0 || !*(bool*)(plDante + 0x1AD8))
		return true;
	gf::Vec3 DantePos = get_char_pos(plDante);
	auto plManager = sdk::get_managed_singleton<REManagedObject>("app.PlayerManager");
	if(plManager == nullptr)
		return true;
	auto pl = *(uintptr_t*)((uintptr_t)plManager + 0x60);
	auto plPos = get_char_pos(pl);
	if (gf::Vec3::vec_length(plPos, DantePos) < 9.5f)
	{
		int action = releaseDistrib(generator) == 0 ? 2165 : 2166;

		sdk::call_object_func_easy<void*>((REManagedObject*)plDante, "setAction(app.PlayerDante.Actions, System.UInt32, System.Single, System.Single, via.motion.InterpolationMode, via.motion.InterpolationCurve, "
			"System.Boolean, System.Boolean)",
			/*2165, 0, 0, 0, 2, 1, false, true);*/
			action, 0, 0, 0, 2, 1, true, false);
		return false;
	}

	return true;
}

std::optional<std::string> BossDanteSetup::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

	m_is_enabled = &cheaton;
	m_on_page = Page_Enemies;
	m_full_name_string = "Boss Dante settings (+)";
	m_author_string = "V.P.Zadov";
	m_description_string = "Vergil's nightmare begins here.";

	auto dtAddr = m_patterns_cache->find_addr(base, "80 7E 20 00 0F 84 01 02 00 00"); // DevilMayCry5.exe+19C34A9

	if (!dtAddr)
	{
		return "Unanable to find BossDanteSetup.dtAddr pattern.";
	}


	auto sdtTransformAddr = m_patterns_cache->find_addr(base, "8B 49 10 85 C9 74 35"); // DevilMayCry5.exe+19C637C
	if (!sdtTransformAddr)
	{
		return "Unanable to find BossDanteSetup.sdtTransformAddr pattern.";
	}


	auto dtTimerAddr = m_patterns_cache->find_addr(base, "F3 0F 11 4A 18 48 8B"); // DevilMayCry5.exe+318477
	if (!dtTimerAddr)
	{
		return "Unanable to find BossDanteSetup.dtTimerAddr pattern.";
	}

	auto dtDurationAddr = m_patterns_cache->find_addr(base, "F3 0F 10 40 14 48 8B 43 50 0F 5A C8"); // DevilMayCry5.exe+19C304E
	if (!dtDurationAddr)
	{
		return "Unanable to find BossDanteSetup.dtDurationAddr pattern.";
	}

	auto charUpdateDelayAddr = m_patterns_cache->find_addr(base, "32 48 85 C0 75 10"); // DevilMayCry5.exe+19BD0C4 (0x1)
	if (!charUpdateDelayAddr)
	{
		return "Unanable to find BossDanteSetup.charUpdateDelayAddr pattern.";
	}

	auto royalRevengeDelayAddr = m_patterns_cache->find_addr(base, "E8 92 4F DB FE"); // DevilMayCry5.exe+19C9BF9
	if (!charUpdateDelayAddr)
	{
		return "Unanable to find BossDanteSetup.royalRevengeDelayAddr pattern.";
	}

	auto sdtRegenAddr = m_patterns_cache->find_addr(base, "83 BF B0 09 00 00 01 0F 85 42"); // DevilMayCry5.exe+19C346A
	if (!sdtRegenAddr)
	{
		return "Unanable to find BossDanteSetup.sdtRegenAddr pattern.";
	}

	auto setReleaseTypeAddr = m_patterns_cache->find_addr(base, "39 41 18 48 8B 5C 24 30"); // DevilMayCry5.exe+7A3E6D
	if (!setReleaseTypeAddr)
	{
		return "Unanable to find BossDanteSetup.setReleaseTypeAddr pattern.";
	}

	auto setGuardReactAddr = m_patterns_cache->find_addr(base, "E8 2A 7C 03 00"); // DevilMayCry5.exe+1991B91
	if (!setGuardReactAddr)
	{
		return "Unanable to find BossDanteSetup.setGuardReactAddr pattern.";
	}

	dtRegenJe = dtAddr.value() + 0x20B;
	emDanteDelayJmpRet = charUpdateDelayAddr.value() + 0x37 + 0x1;
	emDanteDelayJneRet = charUpdateDelayAddr.value() + 0x15 + 0x1;
	//setWorkRateCall = m_patterns_cache->find_addr(base, "B0 01 48 83 C4 20 5B C3 CC CC 40 53 48 83 EC 30").value_or((uintptr_t)base + 0x77EB90); // DevilMayCry5.exe+77EB90 (-0xA) // DevilMayCry5.app_WorkRateSystem__setWorkRate96343
	if(auto addr = m_patterns_cache->find_addr(base, "B0 01 48 83 C4 20 5B C3 CC CC 40 53 48 83 EC 30"); addr)
		setWorkRateCall = addr.value() + 0xA;
	else
		setWorkRateCall = (uintptr_t)base + 0x77EB90;
	sdtRegenJmp = sdtRegenAddr.value() + 0xD;
	setGuardReactionEmDante = m_patterns_cache->find_addr(base, "40 53 56 57 48 83 EC 70 48 8B D9").value_or((uintptr_t)base + 0x19C97C0);

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

	if (!install_hook_absolute(royalRevengeDelayAddr.value(), m_royal_revenge_delay_hook, &revenge_delay_detour, &workRateRoyalRevengeRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize BossDanteSetup.royalRevengeDelay";
	}

	if (!install_hook_absolute(sdtRegenAddr.value(), m_sdt_regen_hook, &sdt_regen_detour, &sdtRegenRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize BossDanteSetup.sdtRegen";
	}

	if (!install_hook_absolute(setReleaseTypeAddr.value(), m_set_royal_release_hook, &set_release_detour, &setRoyalReleaseRet, 0x8))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize BossDanteSetup.setReleaseType";
	}

	if (!install_hook_absolute(setGuardReactAddr.value(), m_set_guard_ract_hook, &set_damage_react_detour, &damageReactRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize BossDanteSetup.setGuardReact";
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
	isSdtRegen = cfg.get<bool>("BossDanteSetup.isSdtRegen").value_or(true);
	allowRoyalRelease = cfg.get<bool>("BossDanteSetup.allowRoyalRelease").value_or(true);
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
	cfg.set<bool>("BossDanteSetup.isSdtRegen", isSdtRegen);
	cfg.set<bool>("BossDanteSetup.allowRoyalRelease", allowRoyalRelease);
	cfg.set<uint32_t>("BossDanteSetup.sdtTransformMode", sdtTransformMode);
	cfg.set<float>("BossDanteSetup.dtTimer", dtTimer);
	cfg.set<float>("BossDanteSetup.dtDuration", dtDuration);
}

// void BossDanteSetup::on_frame(){}

void BossDanteSetup::on_draw_ui()
{
	ImGui::Checkbox("Enable HP regeneration in DT", &isDtRegenEnabled);
	ImGui::Checkbox("Specify SDT Transformation Type", &isSdtTransformSetup);
	if (isSdtTransformSetup)
	{
		ImGui::Spacing();
		ImGui::Combo("Transformation Type", (int*)&sdtTransformMode, "Always Normal Transform\0Always SSSS Transform (Default)\0");
		//ImGui::TextWrapped("Change transform mode to: 0 - always normal transform, 1 - always fast transform (default), 2 - first normal other fast (idk is this one even working?)");
		//ImGui::SliderInt("##TransformMode", (int*)&sdtTransformMode, 0, 2);
	}
	ImGui::Checkbox("Modify DT & SDT cooldown", &isDtsTimerSkip);
	if(isDtsTimerSkip)
	{
		ImGui::TextWrapped("Cooldown timer (depends on current mission):");
		UI::SliderFloat("##customTimerDelay", &dtTimer, 0.1f, 30.0f, "%.1f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
		//dtTimerStartPoint = 30.0f - dtTimer + 0.08f;
	}
	ImGui::Checkbox("Modify DT duration", &isCustomDtDuration);
	if (isCustomDtDuration)
	{
		ImGui::TextWrapped("Set DT duration on Mission 19 (game's default - 15):");
		ImGui::InputFloat("##dtDurationSlider", &dtDuration, 1.0f, 2.0f, "%.1f");
	}
	ImGui::Checkbox("No recovery after SDT exit", &isNoFinishSdtStun);
	ImGui::Checkbox("Disable increased startup times on certain moves (Revolver, Stinger, etc.)", &isNoMovesDelay);
	ImGui::Checkbox("Enable SDT HP regeneration", &isSdtRegen);
	ImGui::Checkbox("Use royal release", &allowRoyalRelease);
	//ImGui::Checkbox("Allow \"Flipper\"", &allowFlipper);
}

// void BossDanteSetup::on_draw_debug_ui(){}

void BossDanteSetup::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}

//clamg-format on
