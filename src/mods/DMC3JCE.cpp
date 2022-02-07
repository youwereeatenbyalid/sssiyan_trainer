#include "DMC3JCE.hpp"

static naked void can_exe_jce_detour()
{
	__asm {
		setbe al
		cmp byte ptr [DMC3JCE::cheaton], 1
		je cheat

		originalcode:
		//setbe al
		test al, al
		jmp qword ptr [DMC3JCE::canExeJceRet]

		cheat:
		//cmp dword ptr [VergilSDTFormTracker::vergilform_state], 0
		push rax
		mov rax, [PlayerTracker::vergilentity]
		mov eax, dword ptr [rax+0x09B0]
		cmp eax, 0
		pop rax
		jne originalcode
		cmp dword ptr [rbx + 0x1978], 0 //isYamato
		jne originalcode
		mov al, 01
		test al, al
		jmp qword ptr [DMC3JCE::canExeJceRet]
	}
}

static naked void can_exe_jce_detour1()//need to check cur weapon too
{
	__asm {
		setbe al
		cmp byte ptr [DMC3JCE::cheaton], 1
		je cheat

		originalcode:
		//setbe al
		test al, al
		jmp qword ptr [DMC3JCE::canExeJceRet1]

		cheat:
		//cmp dword ptr [VergilSDTFormTracker::vergilform_state], 0
		push rax
		mov rax, [PlayerTracker::vergilentity]
		mov eax, dword ptr [rax+0x09B0]
		cmp eax, 0
		pop rax
		jne originalcode
		movss xmm0, [rdx+0x1B20]//CurSdtVal
		comiss xmm0, [DMC3JCE::minSdt]
		jb disable_jce
		mov al, 01
		test al, al
		jmp qword ptr[DMC3JCE::canExeJceRet1]

		disable_jce:
		mov al, 00
		test al, al
		jmp qword ptr [DMC3JCE::canExeJceRet1]
	}
}

static naked void jcehuman_sub_sdt_detour()
{
	__asm {
		cmp byte ptr [DMC3JCE::cheaton], 1
		je cheat

		originalcode:
		movss [r8+0x00001B20], xmm1
		jmp qword ptr [DMC3JCE::subSdtRet]

		cheat:
		//cmp dword ptr [VergilSDTFormTracker::vergilform_state], 0
		push rax
		mov rax, [PlayerTracker::vergilentity]
		mov eax, dword ptr [rax+0x09B0]
		cmp eax, 0
		pop rax
		jne originalcode
		cmp dword ptr [r8+0x1978], 0
		jne originalcode
		movss xmm1, [r8+0x00001B20]
		movss xmm0, [DMC3JCE::humanJCECost]
		subss xmm1, xmm0
		movss [r8 + 0x00001B20], xmm1
		jmp qword ptr [DMC3JCE::subSdtRet]
	}
}

static naked void jceprefab_detour()
{
	__asm {
		cmp byte ptr[DMC3JCE::cheaton], 1
		je cheat

		originalcode:
		mov r8, [rsi+0x10]
		lea rax, [rbp-0x4C]
		jmp qword ptr[DMC3JCE::jcePfbRet]

		cheat:
		cmp byte ptr [DMC3JCE::isUseDefaultJce], 1
		je originalcode
		//cmp dword ptr [VergilSDTFormTracker::vergilform_state], 0
		push rax
		mov rax, [PlayerTracker::vergilentity]
		mov eax, dword ptr [rax+0x09B0]
		cmp eax, 0
		pop rax
		jne originalcode
		mov r8, 0
		lea rax, [rbp-0x4C]
		jmp qword ptr[DMC3JCE::jcePfbRet]
	}
}

static naked void jceprefab1_detour()
{
	__asm {
		cmp byte ptr [DMC3JCE::cheaton], 1
		je cheat

		originalcode:
		lea r9, [rbp+0x20]
		mov rdx, rbx
		jmp qword ptr [DMC3JCE::jcePfb1Ret]

		cheat:
		cmp byte ptr [DMC3JCE::isUseDefaultJce], 1
		je originalcode
		//cmp dword ptr [VergilSDTFormTracker::vergilform_state], 0
		push rax
		mov rax, [PlayerTracker::vergilentity]
		mov eax, dword ptr [rax+0x09B0]
		cmp eax, 0
		pop rax
		jne originalcode
		jmp qword ptr [DMC3JCE::jcePfbJeJmp]
	}
}

static naked void jceprefab2_detour()
{
	__asm {
		cmp byte ptr [DMC3JCE::cheaton], 1
		je cheat

		originalcode:
		cmp [rax+0x18], rdi
		jne jne_ret
		jmp qword ptr [DMC3JCE::jcePfb2Ret]

		cheat:
		cmp byte ptr [DMC3JCE::isUseDefaultJce], 1
		je originalcode
		//cmp dword ptr [VergilSDTFormTracker::vergilform_state], 0
		push rax
		mov rax, [PlayerTracker::vergilentity]
		mov eax, dword ptr [rax+0x09B0]
		cmp eax, 0
		pop rax
		jne originalcode

		jne_ret:
		jmp qword ptr [DMC3JCE::jcePfbJneJmp]
	}
}


void DMC3JCE::stop_jce_asm()
{
	jceController->stop_jce();
}

void DMC3JCE::start_jce_asm()
{
	if (!jceController->is_executing())
	{
		jceController->start_jce();
	}
}

volatile uintptr_t rdx_back_c = 0;//i guess it's have smth bad with std::vector allocator
volatile uintptr_t rdx_back_s = 0;//i guess it's have smth bad with std::vector allocator

static naked void jce_exetime_detour()
{
	__asm {
		cmp byte ptr [DMC3JCE::cheaton], 1
		je cheat

		originalcode:
		movss xmm0, [DMC3JCE::jceTimerStaticBase]
		jmp qword ptr [DMC3JCE::jceTimerRet]

		cheat:
		cmp byte ptr [DMC3JCE::isUseDefaultJce], 1
		je originalcode
		//cmp dword ptr [VergilSDTFormTracker::vergilform_state], 0
		push rax
		mov rax, [PlayerTracker::vergilentity]
		mov eax, dword ptr [rax+0x09B0]
		cmp eax, 0
		pop rax
		jne originalcode
		movss xmm0, [DMC3JCE::JCEController::executionTimeAsm]
		comiss xmm0, xmm1
		movss [rdi+0x6C], xmm1
		ja jce_continue

		push rax
		sub rsp, 32
		call qword ptr [DMC3JCE::stop_jce_asm]
		add rsp, 32
		pop rax

		jmp qword ptr [DMC3JCE::stopJceTimerRet]
		//jmp qword ptr [DMC3JCE::jceTimerRet]

		jce_continue:
		//cmp byte ptr [DMC3JCE::isJceRunning], 1
		//je jmp_ret
		push rax
		push rcx
		mov qword ptr [rdx_back_c], rdx
		push r8
		push r9
		push r10
		push r11
		sub rsp, 32
		call qword ptr [DMC3JCE::start_jce_asm]
		add rsp, 32
		pop r11
		pop r10
		pop r9
		pop r8
		mov rdx, qword ptr [rdx_back_c]
		pop rcx
		pop rax

		jmp_ret:
		jmp qword ptr [DMC3JCE::jceTimerContinue]
	}
}

static naked void crashpoint_detour()//this mb not good idk
{
	__asm {
		cmp byte ptr [DMC3JCE::cheaton], 1
		je cheat

		originalcode:
		test byte ptr [rax+0x0C], 01
		je je_jmp
		jmp qword ptr [DMC3JCE::crashPointRet]

		cheat:
		cmp byte ptr [DMC3JCE::isUseDefaultJce], 1
		je originalcode
		cmp byte ptr [DMC3JCE::isCrashFixEnabled], 0
		je originalcode
		cmp rax, 0
		jne originalcode

		je_jmp:
		jmp qword ptr [DMC3JCE::crashPointJeJmp]
	}
}

static naked void finish_pfb_detour()
{
	__asm {
		cmp byte ptr [DMC3JCE::cheaton], 1
		je cheat

		originalcode:
		mov r8, rsi
		mov [rsp+0x28], r13
		jmp qword ptr [DMC3JCE::jceFinishPfbRet]

		cheat:
		cmp byte ptr [DMC3JCE::isUseDefaultJce], 1
		je originalcode
		//cmp dword ptr [VergilSDTFormTracker::vergilform_state], 0
		push rax
		mov rax, [PlayerTracker::vergilentity]
		mov eax, dword ptr [rax+0x09B0]
		cmp eax, 0
		pop rax
		jne originalcode
		mov rsi, 0
		jmp originalcode
	}
}

std::optional<std::string> DMC3JCE::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

	m_is_enabled = &cheaton;
	m_on_page = vergiltrick;
	m_full_name_string = "Enhanced human JCE mechanic (DMC3 JCE) (+)";
	m_author_string = "VPZadov";
	m_description_string = "Change duration, amount of SDT expended to perform JCE and overall virgin DMC5 human JCE to chad DMC3 boss Vergil JCE. Can be executed without full STD bar. This mod can crash the game :(";
	auto dmc5Base = g_framework->get_module().as<uintptr_t>();

	jceTimerStaticBase = dmc5Base + 0x45F0390;//0x45F0360;
	jceTimerStaticBase = *(uintptr_t*)jceTimerStaticBase;
	rayCastAddr = dmc5Base + 0x7E53744;

	auto canExeJceAddr = utility::scan(base, "0F 96 C0 84 C0 74 90");//DevilMayCry5.exe+54F481
	if (!canExeJceAddr)
	{
		return "Unable to find DMC3JCE.canExeJceAddr pattern.";
	}

	auto canExeJceAddr1 = utility::scan(base, "0F 96 C0 84 C0 0F 84 CF 00 00 00 48 8B CB 48 85 D2 0F 84 9D 00 00 00 E8 D8");//DevilMayCry5.exe+1C0A54C
	if (!canExeJceAddr1)
	{
		return "Unable to find DMC3JCE.canExeJceAddr1 pattern.";
	}

	auto subHumanJceAddr = utility::scan(base, "F3 41 0F 11 88 20 1B 00 00");//DevilMayCry5.exe+25009B2
	if (!subHumanJceAddr)
	{
		return "Unable to find DMC3JCE.subHumanJceAddr pattern.";
	}

	auto jcePrefab2Addr = utility::scan(base, "48 39 78 18 0F 85 C8 18 00 00");//DevilMayCry5.exe+1C0739D
	if (!jcePrefab2Addr)
	{
		return "Unable to find DMC3JCE.jcePrefab2Addr pattern.";
	}

	auto jceTimerAddr = utility::scan(base, "F3 0F 10 05 96 67 1E 04 0F 2F C1");//DevilMayCry5.exe+409BF2
	if (!jceTimerAddr)
	{
		return "Unable to find DMC3JCE.jceTimerAddr pattern.";
	}

	auto crashPointAddr = utility::scan(base, "F6 40 0C 01 74 05");//DevilMayCry5.sub_142A44550+3E
	if (!crashPointAddr)
	{
		return "Unable to find DMC3JCE.crashPointAddr pattern.";
	}

	auto finishPfbAddr = utility::scan(base, "4C 8B C6 4C 89 6C 24 28");//DevilMayCry5.exe+56CAD7
	if (!crashPointAddr)
	{
		return "Unable to find DMC3JCE.finishPfbAddr pattern.";
	}

	stopJceTimerRet = jceTimerAddr.value() + 0x12;
	jceTimerContinue = jceTimerAddr.value() + 0x2A;
	crashPointJeJmp = crashPointAddr.value() + 0xB;
	jceController = std::make_unique<JCEController>();
	jcePfbJneJmp = jcePrefab2Addr.value() + 0x18D2;

	if (!install_hook_absolute(canExeJceAddr.value(), m_can_exe_jce_hook, &can_exe_jce_detour, &canExeJceRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DMC3JCE.canExeJce";
	}

	if (!install_hook_absolute(canExeJceAddr1.value(), m_can_exe_jce1_hook, &can_exe_jce_detour1, &canExeJceRet1, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DMC3JCE.canExeJce1";
	}

	if (!install_hook_absolute(subHumanJceAddr.value(), m_sub_human_jce_hook, &jcehuman_sub_sdt_detour, &subSdtRet, 0x9))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DMC3JCE.subHumanJce";
	}

	if (!install_hook_absolute(jcePrefab2Addr.value(), m_jce_prefab2_hook, &jceprefab2_detour, &jcePfb2Ret, 0xA))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DMC3JCE.jcePrefab2";
	}

	if (!install_hook_absolute(jceTimerAddr.value(), m_jce_timer_hook, &jce_exetime_detour, &jceTimerRet, 0x8))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DMC3JCE.jceTimer";
	}

	if (!install_hook_absolute(crashPointAddr.value(), m_jce_crashpoint_hook, &crashpoint_detour, &crashPointRet, 0x6))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DMC3JCE.crashPoint";
	}

	if (!install_hook_absolute(finishPfbAddr.value(), m_jce_finishpfb_hook, &finish_pfb_detour, &jceFinishPfbRet, 0x8))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DMC3JCE.finishPfb";
	}

	//cheaton = true;
	return Mod::on_initialize();
}


void DMC3JCE::on_config_load(const utility::Config& cfg)
{
	jceController->rndDelayTime = cfg.get<int>("DMC3JCE.rndDelayTime").value_or(jceController->defaultRndDelay);
	rndDelay = jceController->rndDelayTime;
	jceController->trackDelayTime = cfg.get<int>("DMC3JCE.trackDelayTime").value_or(jceController->defaultTrackDelay);
	trackDelay = jceController->trackDelayTime;
	jceController->set_jce_type( (JCEController::Type)cfg.get<int>("DMC3JCE.jceType").value_or(JCEController::Random) );
	jcTypeUi = jceController->get_jce_type();
	isCrashFixEnabled = cfg.get<bool>("DMC3JCE.isCrashFixEnabled").value_or(true);
	isUseDefaultJce = cfg.get<bool>("DMC3JCE.isUseDefaultJce").value_or(false);
	jceController->rndEmTrackInterval = cfg.get<int>("DMC3JCE.rndEmTrackInterval").value_or(22);
	humanJCECost = cfg.get<float>("DMC3JCE.humanJCECost").value_or(3000.0f);
	minSdt = cfg.get<float>("DMC3JCE.minSdt").value_or(3000.0f);
}

void DMC3JCE::on_config_save(utility::Config& cfg)
{
	cfg.set<int>("DMC3JCE.rndDelayTime", jceController->rndDelayTime);
	cfg.set<int>("DMC3JCE.trackDelayTime", jceController->trackDelayTime);
	cfg.set<int>("DMC3JCE.jceType", jceController->get_jce_type());
	cfg.set<bool>("DMC3JCE.isCrashFixEnabled", isCrashFixEnabled);
	cfg.set<int>("DMC3JCE.rndEmTrackInterval", jceController->rndEmTrackInterval);
	cfg.set<float>("DMC3JCE.humanJCECost", humanJCECost);
	cfg.set<float>("DMC3JCE.minSdt", minSdt);
}

void DMC3JCE::on_frame()
{
}

void DMC3JCE::on_draw_ui()
{
	ImGui::TextWrapped("If after Vergil's disappear, jc doesnt start spawn and cheat has been disabled - pointer to jc wasn't loaded correctly. Restart the mission.");
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Checkbox("Use default jce in human form (no dmc3 jce).", &isUseDefaultJce);
	ImGui::TextWrapped("Random mode uses perfect jc shell and increased damage. Track mode uses default jc shell and damage. Modes also uses different execution time.");
	ImGui::Separator();
	ImGui::TextWrapped("SDT cost for performing JCE in human form:");
	ImGui::SliderFloat("##subSdtSlider", &humanJCECost, 0, 10000.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::Spacing();
	ImGui::TextWrapped("Minimum sdt to perform JCE in human form:");
	ImGui::SliderFloat("##minSdtSlider", &minSdt, 0, 10000.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

	//--------------This should be removed----------------//
	/*ImGui::Separator();
	if(ImGui::Button("Set 64 capacity"))
		jceController->set_capacity(64);
	ImGui::Separator();*/

	/*ImGui::Separator();
	ImGui::Checkbox("Set cutom capacity", &isSetCustomCapacity);
	ImGui::SliderInt("Custom capacity:", &newCapacity, 64, 256);
	ImGui::Separator();*/
	//----------------------------------------------------//
	ImGui::TextWrapped("Select DMC3 JCE type:");

	if( ImGui::RadioButton("Random", (int*)&jcTypeUi, 0) )
		jceController->set_jce_type(jcTypeUi);
	ImGui::SameLine(); ImGui::Spacing();
	if(ImGui::RadioButton("Track", (int*)&jcTypeUi, 1))
		jceController->set_jce_type(jcTypeUi);

	switch (jceController->get_jce_type())
	{
		case JCEController::Type::Random:
		{
			ImGui::TextWrapped("Delay between each jc spawn (ms). Low value can crash the game:");
			ImGui::SliderInt("##DelayRand", &rndDelay, 78, 450, "%d", ImGuiSliderFlags_AlwaysClamp);
			if (ImGui::Button("Apply delay settings ##0"))
				jceController->set_rndspawn_delay(rndDelay);
			ImGui::TextWrapped("Interval between spawn on lock on position:");
			ImGui::SliderInt("##rndInterval", &jceController->rndEmTrackInterval, 12, 32, "%d", ImGuiSliderFlags_AlwaysClamp);
			break;
		}
		case JCEController::Type::Track:
		{
			ImGui::TextWrapped("Delay between each jc spawn (ms). Low value can crash the game and make this OP as fuck vs single target:");
			ImGui::SliderInt("##DelayTrack", &trackDelay, 115, 450, "%d", ImGuiSliderFlags_AlwaysClamp);
			if (ImGui::Button("Apply delay settings ##1"))
				jceController->set_trackspawn_delay(trackDelay);
			break;
		}
		default:
			break;
	}
	ImGui::Separator();
	ImGui::TextWrapped("Enable this if game crashes with low delay between jc spawn. Can screw up some enemy projectiles while performing dmc3 jce.");
	ImGui::Checkbox("Enable crash fix", &isCrashFixEnabled);
	//ImGui::InputInt("RayCastQuerySize", &curRayCastSize);
	
	ImGui::TextWrapped("If smth goes wrong(TM) and 3 JCE execution doesn't stop after Vergil appears, press this:");
	if (ImGui::Button("Stop 3 jce"))
	{
		isJceRunning = false;
		jceController->stop_jce();
		
	}
	ImGui::Separator();
	ImGui::TextWrapped("Start spawning jce. Keep this here only for fun.");
	if (ImGui::Button("Start 3 jce"))
	{
		jceController->start_jce();
	}
}

void DMC3JCE::on_draw_debug_ui()
{
}

void DMC3JCE::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
