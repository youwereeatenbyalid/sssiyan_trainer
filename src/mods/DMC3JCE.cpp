#include "DMC3JCE.hpp"

bool DMC3JCE::can_execute_asm()
{
	int sdtState = *(int*)(PlayerTracker::vergilentity + 0x9B0);
	float curSdt = *(float*)(PlayerTracker::vergilentity + 0x1B20);
	if (autoSdtType == None)
	{
		if (sdtState != 2)
		{
			if(curSdt >= minSdt)
				return true;
		}
	}
	else
	{
		if(curSdt >= minSdt)
			return true;
	}
	return false;
}

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
		push rax
		call qword ptr [DMC3JCE::can_execute_asm]
		cmp al, 0
		pop rax
		je originalcode
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
		push rax
		call qword ptr [DMC3JCE::can_execute_asm]
		cmp al, 0
		pop rax
		je originalcode

		forcejce:
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
		push rax
		sub rsp, 32
		call qword ptr [DMC3JCE::use_default_behaviour_asm]
		add rsp, 32
		cmp al, 1
		pop rax
		je originalcode
		cmp dword ptr [r8+0x1978], 0
		jne originalcode
		movss xmm1, [r8+0x00001B20]
		movss xmm0, [DMC3JCE::humanJCECost]
		subss xmm1, xmm0
		movss [r8 + 0x00001B20], xmm1
		jmp qword ptr [DMC3JCE::subSdtRet]
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
		push rax
		sub rsp, 32
		call qword ptr [DMC3JCE::use_default_behaviour_asm]
		add rsp, 32
		cmp al, 1
		pop rax
		je originalcode

		jne_ret:
		jmp qword ptr [DMC3JCE::jcePfbJneJmp]
	}
}

bool DMC3JCE::use_default_behaviour_asm()
{
	if (jceController->is_in_jce_sdt() || jceController->is_executing())
		return false;
	return true;
}

void DMC3JCE::end_jcesdt_asm(uintptr_t vergil)
{
	if (jceController->is_in_jce_sdt())
		jceController->request_set_jce_dt(vergil, gf::PlVergilSetDT::Human);
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

		push rax
		push rcx
		push rdx
		push rsp
		push r8
		push r9
		push r10
		push r11
		mov rcx, r8
		call qword ptr [DMC3JCE::end_jcesdt_asm]
		pop r11
		pop r10
		pop r9
		pop r8
		pop rsp
		pop rdx
		pop rcx
		
		mov eax, dword ptr [r8+0x09B0]
		cmp eax, 2
		pop rax
		je originalcode
		nullpfb:
		mov rsi, 0
		jmp originalcode
	}
}

void DMC3JCE::jce_cancel_hook(uintptr_t threadCtxt, uintptr_t vergil)
{
	_mod->m_jce_cancel_hook->get_original<decltype(jce_cancel_hook)>()(threadCtxt, vergil);
	if (!DMC3JCE::cheaton)
		return;
	if(jceController->is_executing())
		jceController->stop_jce();
	if (jceController->is_in_jce_sdt())
		jceController->request_set_jce_dt(vergil, gf::PlVergilSetDT::DevilTrigger::Human);
	_mod->_jceTimer = 0;
}

void DMC3JCE::pl0800_start_jce_update(uintptr_t threadCtxt, uintptr_t fsmStartJce, uintptr_t actionArg)
{
	_mod->m_update_jce_hook->get_original<decltype(DMC3JCE::pl0800_start_jce_update)>()(threadCtxt, fsmStartJce, actionArg);
	//if cheat not on, end
	if (!DMC3JCE::cheaton)
		return;
	auto vergil = *(uintptr_t*)(fsmStartJce + 0x40);
	//if in SDT && Not Executing already doing DMC3 JCE and not already going SDT JCE (that's the normal one right?) then return
	if (*(int*)(vergil + 0x09B0) == 2 && !jceController->is_executing() && !jceController->is_in_jce_sdt())
		return;
	int jceRoutine = *(int*)(fsmStartJce + 0x68);
	if (jceRoutine == 0 && !jceController->is_in_jce_sdt() && !jceController->is_executing())
		jceController->request_set_jce_dt(vergil, gf::PlVergilSetDT::SDT, autoSdtType);
	if (jceRoutine == 1 && jceController->is_in_jce_sdt() && !jceController->is_executing())
	{
		jceController->start_jce(*(uintptr_t*)(fsmStartJce + 0x40));
		jceController->force_jcesdt_health_update(vergil);
	}
	_mod->_jceTimer += *(float*)(fsmStartJce + 0x6C);
	if (( _mod->_jceTimer >= jceController->get_current_execution_time() && jceController->is_executing() ) || jceRoutine == 2)
	{
		*(float*)(fsmStartJce + 0x6C) = _mod->_jceTimer + 0.1f;
		jceController->stop_jce(false);
		_mod->_jceTimer = 0;
	}
	else
		*(float*)(fsmStartJce + 0x6C) = 0;
}
/// <summary>
/// Override DT gain to 0 for DMC3 JCE?
/// </summary>
/// <param name="threadCtxt"></param>
/// <param name="pl"></param>
/// <param name="val"></param>
/// <param name="dtAddType"></param>
/// <param name="fixedValue"></param>
void DMC3JCE::on_pl_add_dt(uintptr_t threadCtxt, uintptr_t pl, float* val, int dtAddType, bool fixedValue)
{
	if (!cheaton || *(uintptr_t*)(pl + 0xE64) != 4 || !jceController->is_executing())
		return;
	*val = 0;
}

std::optional<std::string> DMC3JCE::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

	m_is_enabled = &cheaton;
	m_on_page = Page_VergilTrick;
	m_depends_on = { "GameplayStateTracker","VergilInstantSDT","InputSystem","EndLvlHooks"};
	m_full_name_string = "DMC3 Judgement Cut End (+)";
	m_author_string = "V.P.Zadov";
	m_description_string = "When not in Devil Trigger, replaces Vergil's Judgment Cut End with his Judgment Cut Barrage from his boss fight in Devil May Cry 3. Can be executed without full STD bar.";

	auto canExeJceAddr = m_patterns_cache->find_addr(base, "0F 96 C0 84 C0 74 90");//DevilMayCry5.exe+54F481
	if (!canExeJceAddr)
	{
		return "Unable to find DMC3JCE.canExeJceAddr pattern.";
	}

	auto canExeJceAddr1 = m_patterns_cache->find_addr(base, "0F 96 C0 84 C0 0F 84 CF 00 00 00 48 8B CB 48 85 D2 0F 84 9D 00 00 00 E8 D8");//DevilMayCry5.exe+1C0A54C
	if (!canExeJceAddr1)
	{
		return "Unable to find DMC3JCE.canExeJceAddr1 pattern.";
	}

	auto subHumanJceAddr = m_patterns_cache->find_addr(base, "F3 41 0F 11 88 20 1B 00 00");//DevilMayCry5.exe+25009B2
	if (!subHumanJceAddr)
	{
		return "Unable to find DMC3JCE.subHumanJceAddr pattern.";
	}

	auto jcePrefab2Addr = m_patterns_cache->find_addr(base, "48 39 78 18 0F 85 C8 18 00 00");//DevilMayCry5.exe+1C0739D
	if (!jcePrefab2Addr)
	{
		return "Unable to find DMC3JCE.jcePrefab2Addr pattern.";
	}

	auto finishPfbAddr = m_patterns_cache->find_addr(base, "4C 8B C6 4C 89 6C 24 28");//DevilMayCry5.exe+56CAD7
	if (!finishPfbAddr)
	{
		return "Unable to find DMC3JCE.finishPfbAddr pattern.";
	}

	auto cancelJceAddr = m_patterns_cache->find_addr(base, "48 8B 82 E0 1B 00 00");//DevilMayCry5.app_PlayerVergilPL__JudgementCutEndCancel114060
	if (!cancelJceAddr)
	{
		return "Unable to find DMC3JCE.cancelJceAddr pattern.";
	}

	auto jceStartUpdateAddr = m_patterns_cache->find_addr(base, "30 5F C3 CC CC CC CC CC 48 89 5C 24 18 48 89 74 24 20 57 48 83 EC 20");
	//DevilMayCry5.app_fsm2_player_pl0800_StartJudgementCutEnd__update312745 (-0x8)
	if (!jceStartUpdateAddr)
	{
		return "Unable to find DMC3JCE.jceStartUpdateAddr pattern.";
	}

	try
	{
		jceController = std::make_unique<JCEController>();
	}
	catch(const std::exception &e)
	{
		spdlog::error("[{}] failed to initialize JCEController", get_name());
		return "DMC3JCE; can't init 3JCEController: " + std::string(e.what());
	}
	jcePfbJneJmp = jcePrefab2Addr.value() + 0x18D2;

	_mod = this;

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

	if (!install_hook_absolute(finishPfbAddr.value(), m_jce_finishpfb_hook, &finish_pfb_detour, &jceFinishPfbRet, 0x8))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DMC3JCE.finishPfb";
	}

	m_jce_cancel_hook = std::make_unique<FunctionHook>(cancelJceAddr.value(), &DMC3JCE::jce_cancel_hook);
	m_jce_cancel_hook->create();
	m_update_jce_hook = std::make_unique<FunctionHook>(jceStartUpdateAddr.value() + 0x8, &DMC3JCE::pl0800_start_jce_update);
	m_update_jce_hook->create();

	return Mod::on_initialize();
}

void DMC3JCE::on_config_load(const utility::Config& cfg)
{
	jceController->set_rndspawn_delay( cfg.get<int>("DMC3JCE._rndDelayTime").value_or(jceController->defaultRndDelay));
	rndDelay = jceController->get_rndspawn_delay();
	jceController->set_trackspawn_delay(cfg.get<int>("DMC3JCE._trackDelayTime").value_or(jceController->defaultTrackDelay));
	trackDelay = jceController->get_trackspawn_delay();
	jceController->set_jce_type( (JCEController::Type)cfg.get<int>("DMC3JCE.jceType").value_or(JCEController::Dynamic) );
	jcTypeUi = jceController->get_jce_type();
	jceController->rndEmTrackInterval = cfg.get<int>("DMC3JCE.rndEmTrackInterval").value_or(22);
	humanJCECost = cfg.get<float>("DMC3JCE.humanJCECost").value_or(3000.0f);
	minSdt = cfg.get<float>("DMC3JCE.minSdt").value_or(3000.0f);
	isCrashFixEnabled = true;
	rndExeDuration = jceController->rndExeTime = cfg.get<float>("DMC3JCE.rndExeDuration").value_or(jceController->rndExeTimeModDefault);
	trackExeDuration = jceController->trackExeTime = cfg.get<float>("DMC3JCE.trackExeDuration").value_or(jceController->trackExeTimeModDefault);
	autoSdtType = (AutoSDTType)cfg.get<int>("DMC3JCE.autoSdtType").value_or(AutoSDTType::LessEfx);
}

void DMC3JCE::on_config_save(utility::Config& cfg)
{
	cfg.set<int>("DMC3JCE._rndDelayTime", jceController->get_rndspawn_delay());
	cfg.set<int>("DMC3JCE._trackDelayTime", jceController->get_trackspawn_delay());
	cfg.set<int>("DMC3JCE.jceType", jceController->get_jce_type());
	cfg.set<int>("DMC3JCE.rndEmTrackInterval", jceController->rndEmTrackInterval);
	cfg.set<float>("DMC3JCE.humanJCECost", humanJCECost);
	cfg.set<float>("DMC3JCE.minSdt", minSdt);
	cfg.set<float>("DMC3JCE.rndExeDuration", rndExeDuration);
	cfg.set<float>("DMC3JCE.trackExeDuration", trackExeDuration);
	cfg.set<int>("DMC3JCE.autoSdtType", (int)autoSdtType);
}

void DMC3JCE::on_draw_ui()
{
	
	//ImGui::TextWrapped("Random mode uses Just Judgement Cut projectile and 2x increased damage. Tracking mode uses default Judgement Cut projectile and 1.2x damage. The modes also use different execution times.");
	//ImGui::Separator();
	ImGui::TextWrapped("SDT Requirement:");
	UI::SliderFloat("##minSdtSlider", &minSdt, 0, 10000.0f, "%.1f", 1.0F, ImGuiSliderFlags_AlwaysClamp);

	ImGui::Separator();

	ImGui::TextWrapped("Automatically enter SDT:");
	ImGui::ShowHelpMarker("Vergil will automatically enter SDT when performing the Judgment Cut Barrage input. After re-appearing or being interrupted, Vergil will automatically exit SDT.");
	ImGui::RadioButton("Disable SDT explosion efx", (int*)&autoSdtType, 1);
	ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
	ImGui::RadioButton("Enable SDT explosion efx", (int*)&autoSdtType, 2);
	ImGui::Separator();
	ImGui::TextWrapped("Select Tracking Mode:");

	if (ImGui::RadioButton("Dynamic", (int*)&jcTypeUi, 2))
		jceController->set_jce_type(jcTypeUi);
	ImGui::ShowHelpMarker("Hold the attack button when Vergil starts the Judgment Cut Barrage to perform a JCE that tracks the enemy. Otherwise, the barrage will spread out in a circle around the starting point.");

	if (ImGui::RadioButton("Random", (int*)&jcTypeUi, 0))
		jceController->set_jce_type(jcTypeUi);
	ImGui::SameLine(); ImGui::Spacing();
	if (ImGui::RadioButton("Target", (int*)&jcTypeUi, 1))
		jceController->set_jce_type(jcTypeUi);
	
	

	switch (jceController->get_jce_type())
	{
		case JCEController::Type::Random:
		{
			ImGui::TextWrapped("Delay between each JdC spawn. Low values can crash the game:");
			if(UI::SliderInt("##DelayRand", &rndDelay, 78, 450, "%d", 1.0F, ImGuiSliderFlags_None))
				jceController->set_rndspawn_delay(rndDelay);
			ImGui::TextWrapped("Interval between spawning a Judgement Cut on the lock-on target:");
			UI::SliderInt("##rndInterval", &jceController->rndEmTrackInterval, 12, 32, "%d", 1.0F, ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextWrapped("Execution duration:");
			UI::SliderFloat("##rndExeTime", &jceController->rndExeTime, 2.8f, 8.0f, "%.1f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
			break;
		}
		case JCEController::Type::Track:
		{
			ImGui::TextWrapped("Delay between each JdC spawn. Low values can crash the game:");
			if(UI::SliderInt("##DelayTrack", &trackDelay, 115, 450, "%d", 1.0F, ImGuiSliderFlags_None))
				jceController->set_trackspawn_delay(trackDelay);
			ImGui::TextWrapped("Execution duration:");
			UI::SliderFloat("##trackExeTime", &jceController->trackExeTime, 3.0f, 8.5f, "%.1f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
			break;
		}
		default:
			break;
	}
	ImGui::Separator();
	//ImGui::InputInt("RayCastQuerySize", &curRayCastSize);

	ImGui::TextWrapped("If something goes wrong and Judgment Cuts continue to spawn after Vergil appears, press this:");
	if (ImGui::Button("Stop JCE Barrage."))
	{
		isJceRunning = false;
		jceController->stop_jce();
	}
}

void DMC3JCE::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
