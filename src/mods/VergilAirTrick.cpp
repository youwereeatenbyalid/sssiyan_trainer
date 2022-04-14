#include "VergilAirTrick.hpp"

bool VergilAirTrick::cheaton = false;
bool VergilAirTrick::isSpeedUp = false;
bool VergilAirTrick::isCustomOffset = false;
bool VergilAirTrick::isCustomWaitTime = false;

float VergilAirTrick::initSpeed = 0.0f;
float VergilAirTrick::finishOffsetZ = 0.0f;
float VergilAirTrick::waitTime      = 0.0f;

float specTeleportOffset = 0.25f;

uintptr_t VergilAirTrick::airTrickRet{NULL};
uintptr_t VergilAirTrick::finishOffsetRet{NULL};
uintptr_t VergilAirTrick::waitTimeRet{NULL};
uintptr_t VergilAirTrick::maxXZRet{NULL};
uintptr_t VergilAirTrick::maxSpeedZRet{NULL};
uintptr_t VergilAirTrick::speedAccRet{NULL};
uintptr_t VergilAirTrick::initSpeedRet{NULL};
uintptr_t VergilAirTrick::routineStartRet{NULL};
uintptr_t VergilAirTrick::finishRangeRet{NULL};

volatile uintptr_t rsi_b = 0;
//volatile uintptr_t rbx_b = 0;

static naked void waittime_detour() {
	__asm {
		cmp byte ptr [VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr [VergilAirTrick::isCustomWaitTime], 0
		je originalcode

		cheat:
		mov eax, dword ptr [VergilAirTrick::waitTime]
		jmp qword ptr [VergilAirTrick::waitTimeRet]

		originalcode:
		mov eax, [rsi+0x000000B8]
		jmp qword ptr [VergilAirTrick::waitTimeRet]
  }
}

static naked void finish_offset_detour() {//don't forget t skip all coords opcodes
	__asm {
		cmp byte ptr [VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr [VergilAirTrick::isCustomOffset], 0
		je originalcode

		cheat:
		movss xmm0,[rsi+0x000000D0]
        movss xmm1, [VergilAirTrick::finishOffsetZ]
        movss xmm2, [VergilAirTrick::modFinishOffsetX]
		jmp qword ptr [VergilAirTrick::finishOffsetRet]

		originalcode:
		movss xmm0, [rsi+0x000000D0]
        movss xmm1, [rsi+0x000000D4]
        movss xmm2, [rsi+0x000000D8]
		jmp qword ptr [VergilAirTrick::finishOffsetRet]
  }
}

static naked void initspeed_detour() {
	__asm {
		cmp byte ptr[VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr[VergilAirTrick::isTeleport], 1
		je originalcode//teleport
		cmp byte ptr [VergilAirTrick::isSpeedUp], 0
		je originalcode

		cheat:
		mov eax, [VergilAirTrick::initSpeed]
		jmp qword ptr [VergilAirTrick::initSpeedRet]

		originalcode:
		mov eax, dword ptr [rsi+0x000000A0]
		jmp qword ptr [VergilAirTrick::initSpeedRet]

		teleport:
		mov eax, 0
		jmp qword ptr [VergilAirTrick::initSpeedRet]
	}
}

void VergilAirTrick::xypos_teleport(uintptr_t vergil, TeleportType type, float& x, float& y, GameFunctions::Vec3 pPos, float trickX, float trickY, float trickLen)
{
	switch (type)
	{
		case VergilAirTrick::Front:
		{
			x = pPos.x + (trickLen - trickCorrection) * trickX / (float)trickLen;
			y = pPos.y + (trickLen - trickCorrection) * trickY / (float)trickLen;
			break;
		}
		case VergilAirTrick::Behind:
		{
			x = pPos.x + (trickLen + trickCorrection) * trickX / (float)trickLen;
			y = pPos.y + (trickLen + trickCorrection) * trickY / (float)trickLen;
			break;
		}
		default:
		{
			x = pPos.x + (trickLen - trickCorrection) * trickX / (float)trickLen;
			y = pPos.y + (trickLen - trickCorrection) * trickY / (float)trickLen;
			break;
		}
	}
}

void VergilAirTrick::change_pos_asm(uintptr_t trickAction)
{
	if(PlayerTracker::vergilentity == 0 || trickAction == 0)
		return;
	uint16_t trickOffsAddr = 0;
	__asm (
	"movq %1, %%rax;"
	"movq (%%rax), %%rax;"
	"mov %%ax, %0"
	:"=m"(trickOffsAddr)
	:"g"(trickAction)
	:"rax"
	);
	if (trickOffsAddr == 0xF208)
		return;
	uintptr_t vergil = *(uintptr_t*)(trickAction + 0x60);//Also for doppel when he summoned;
	if(vergil == 0)
		return;
	bool isDoppel = *(bool*)(vergil + 0x17F0);
	GameFunctions::Vec3 targetPos = *(GameFunctions::Vec3*)(trickAction + 0xF0);
	auto gameObj = *(uintptr_t*)(vergil + 0x10);
	auto transformGameObj = *(uintptr_t*)(gameObj + 0x18);
	float xTmp, yTmp;

	void *transform = (void*)(*(uintptr_t*)(vergil + 0x1F0));
	auto pPos = *(GameFunctions::Vec3*)((uintptr_t)transform + 0x30);

	GameFunctions::Vec3 trickVec(targetPos.x - pPos.x, targetPos.y - pPos.y, targetPos.z - pPos.z);
	float trickVecLen = GameFunctions::Vec3::vec_length(pPos, targetPos);
	TeleportType curType = trickType;

	if (isDoppelOppositeTeleport)
	{
		if (isDoppel)
		{
			if(trickType == Front)
				curType = Behind;
			else
				curType = Front;
		}
	}
	xypos_teleport(vergil, curType, xTmp, yTmp, pPos, trickVec.x, trickVec.y, trickVecLen);
	targetPos.x = xTmp;
	targetPos.y = yTmp;
	float oldTargetZ = targetPos.z + colliderZUp;
	targetPos.z += teleportZOffs;

	GameFunctions::Transform_SetPosition set_pos{transform};
	set_pos(targetPos);
	targetPos.z = oldTargetZ;

	//------------------This stuff can ignore walls, whats no good, especially for back step trick-----------------//
	// Actually code above do this too, but seems little more safer;
	//void *posCorPtr = (void*)(*(uintptr_t*)(vergil + 0x8E8));
	//////*(uintptr_t*)((uintptr_t)posCorPtr+0x18) = 0;
	//GameFunctions::PositionErrorCorrector posCor{posCorPtr};
	//posCor.set_rcx(posCor.get_thread_context().value());
	//posCor.set_position(targetPos);
	//-------------------------------------------------------------------------------------------------------------//

	GameFunctions::Transform_SetPosition::set_character_pos(vergil, targetPos, true);

	//*(bool*)(trickAction + 0x144) = true; //isPushHit;
	*(bool*)(trickAction + 0x146) = true; //isInterrupted;
}


static naked void speed_acc_detour() {
  __asm {
		cmp byte ptr[VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr[VergilAirTrick::isTeleport], 1
		je originalcode//teleport
		cmp byte ptr [VergilAirTrick::isSpeedUp], 0
		je originalcode

		cheat:
		movss xmm1, [VergilAirTrick::modSpeedAcc]
		jmp qword ptr [VergilAirTrick::speedAccRet]

		originalcode:
		movss xmm1, [rsi+0x000000A4]
		jmp qword ptr [VergilAirTrick::speedAccRet]

		teleport:
		movss xmm1, dword ptr [VergilAirTrick::modSpeedAcc]
		jmp qword ptr [VergilAirTrick::speedAccRet]

  }
}

static naked void maxspeedz_detour() {
	__asm {
		cmp byte ptr [VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr [VergilAirTrick::isTeleport], 1
		je originalcode
		cmp byte ptr [VergilAirTrick::isSpeedUp], 0
		je originalcode

		cheat:
		movss xmm0, [VergilAirTrick::modMaxSpeedZ]
		jmp qword ptr [VergilAirTrick::maxSpeedZRet]

		originalcode:
		movss xmm0, [rsi+0x000000A8]
		jmp qword ptr [VergilAirTrick::maxSpeedZRet]
  }
}

static naked void max_xz_detour() {
	__asm {
		cmp byte ptr [VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr[VergilAirTrick::isTeleport], 1
		je originalcode
		cmp byte ptr [VergilAirTrick::isSpeedUp], 0
		je originalcode

		cheat:
		movss xmm2,[VergilAirTrick::modMaxXZ]
		jmp qword ptr [VergilAirTrick::maxXZRet]

		originalcode:
		movss xmm2,[rsi+0x000000B0]
		jmp qword ptr [VergilAirTrick::maxXZRet]
  }
}

volatile uintptr_t r9_b = 0;
static naked void air_trick_routine3_detour()
{
	__asm {
		cmp byte ptr [VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr [VergilAirTrick::isTeleport], 0
		je originalcode

		cheat:
		je originalcode
		push rax
		//mov [rbx_b], rbx
		push rcx
		push rdx
		mov [rsi_b], rsi
		push r8
		mov [r9_b], r9
		//push r9
		mov rcx, rdi
		sub rsp, 32
		call qword ptr [VergilAirTrick::change_pos_asm]
		add rsp, 32
		mov r9, qword ptr [r9_b]
		//pop r9
		pop r8
		mov rsi, qword ptr [rsi_b]
		pop rdx
		pop rcx
		//mov rbx, qword ptr [rbx_b]
		pop rax

		originalcode:
		mov dword ptr [rdi + 0x68], 0x3
		jmp qword ptr [VergilAirTrick::routineStartRet]
	}
}

static naked void finish_range_detour()
{
	__asm {
		cmp byte ptr [VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr [VergilAirTrick::isTeleport], 0
		je originalcode

		cheat:
		movss xmm0, [VergilAirTrick::trickCorrection]
		jmp qword ptr [VergilAirTrick::finishRangeRet]


		originalcode:
		movss xmm0, [r14 + 0x000000AC]
		jmp qword ptr [VergilAirTrick::finishRangeRet]
	}
}

std::optional<std::string> VergilAirTrick::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = Page_VergilTrick;
	m_full_name_string = "Air Trick Settings (+)";
	m_author_string = "VPZadov";
	m_description_string = "Adjust the properties of Vergil's Trick Actions.";

  set_up_hotkey();

	auto waitTimeAddr = m_patterns_cache->find_addr(base, "07 00 00 8B 86 B8 00 00 00"); //DevilMayCry5.exe+1DDCB5D
    if (!waitTimeAddr) {
          return "Unanable to find AirTrick.waitTime pattern.";
    }

	auto finishOffsetAddr = m_patterns_cache->find_addr(base, "60 F3 0F 10 86 D0 00 00 00"); //DevilMayCry5.exe+1DDC4FF
    if (!waitTimeAddr) {
          return "Unanable to find AirTrick.finishOffsetAddr pattern.";
    }

	auto initSpeedAddr = m_patterns_cache->find_addr(base, "08 00 00 8B 86 A0 00 00 00"); //DevilMayCry5.exe+1DDCB43
    if (!initSpeedAddr) {
          return "Unanable to find AirTrick.initSpeedAddr pattern.";
    }

	auto maxSpeedZAddr = m_patterns_cache->find_addr(base, "0A 00 00 F3 0F 10 86 A8 00 00 00"); //DevilMayCry5.exe+1DDD585
    if (!maxSpeedZAddr) {
          return "Unanable to find AirTrick.maxSpeedZAddr pattern.";
    }

	auto maxXZAddr = m_patterns_cache->find_addr(base, "06 00 00 F3 0F 10 96 B0 00 00 00"); //DevilMayCry5.exe+1DDCCC5
    if (!maxXZAddr) {
          return "Unanable to find AirTrick.maxXZAddr pattern.";
    }

	auto speedAccAddr = m_patterns_cache->find_addr(base, "F3 0F 10 8E A4 00 00 00 0F 5A C0 0F 5A C9 48 85 C0 0F 84 40"); //DevilMayCry5.exe+1DDD539
    if (!speedAccAddr) {
          return "Unanable to find AirTrick.speedAccAddr pattern.";
    }

	auto routine3Addr = m_patterns_cache->find_addr(base, "03 00 00 C7 47 68 03 00 00 00"); //DevilMayCry5.exe+1DE0427
	if (!routine3Addr)
	{
		return "Unanable to find AirTrick.routine3Addr pattern.";
	}

	auto finishRangeAddr = m_patterns_cache->find_addr(base, "F3 41 0F 10 86 AC 00 00 00"); //DevilMayCry5.exe+1DDE979
	if (!routine3Addr)
	{
		return "Unanable to find AirTrick.finishRangeAddr pattern.";
	}

	if (!install_hook_absolute(initSpeedAddr.value()+0x3, m_initspeed_hook, &initspeed_detour, &initSpeedRet, 0x6)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.initSpeed"; 
    }

	if (!install_hook_absolute(waitTimeAddr.value()+0x3, m_waittime_hook, &waittime_detour, &waitTimeRet, 0x6)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.waitTime"; 
    }

	if (!install_hook_absolute(finishOffsetAddr.value()+0x1, m_finish_offset_hook, &finish_offset_detour, &finishOffsetRet, 0x18)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.finishOffset"; 
    }

	if (!install_hook_absolute(maxSpeedZAddr.value()+0x3, m_maxspeed_z_hook, &maxspeedz_detour, &maxSpeedZRet, 0x8)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.maxSpeedZ"; 
    }

	if (!install_hook_absolute(maxXZAddr.value()+0x3, m_max_xz_hook, &max_xz_detour, &maxXZRet, 0x8)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.maxXZ"; 
    }

	if (!install_hook_absolute(speedAccAddr.value(), m_speed_acc_hook, &speed_acc_detour, &speedAccRet, 0x8)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.speedAcc"; 
    }

	if (!install_hook_absolute(routine3Addr.value()+0x3, m_teleport_hook, &air_trick_routine3_detour, &routineStartRet, 0x7)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.routine3"; 
    }

	if (!install_hook_absolute(finishRangeAddr.value(), m_finish_range_hook, &finish_range_detour, &finishRangeRet, 0x9)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.finishRange"; 
    }


	return Mod::on_initialize();
}

void VergilAirTrick::on_config_load(const utility::Config& cfg)
{
	initSpeed = cfg.get<float>("VergilAirTrick.initSpeed").value_or(3.0f);
	finishOffsetZ = cfg.get<float>("VergilAirTrick.finishOffsetZ").value_or(1.95f);
	waitTime = cfg.get<float>("VergilAirTrick.waitTime").value_or(0.0f);
	isSpeedUp = cfg.get<bool>("VergilAirTrick.isSpeedUp").value_or(true);
	isCustomWaitTime = cfg.get<bool>("VergilAirTrick.isCustomWaitTime").value_or(true);
	isCustomOffset = cfg.get<bool>("VergilAirTrick.isCustomOffset").value_or(false);
	isTeleport = cfg.get<bool>("VergilAirTrick.isTeleport").value_or(false);
	isDoppelOppositeTeleport = cfg.get<bool>("VergilAirTrick.isDoppelOppositeTeleport").value_or(false);
	trickType = (TeleportType)cfg.get<int>("VergilAirTrick.trickType").value_or(Front);
	trickCorrection = cfg.get<float>("VergilAirTrick.trickCorrection").value_or(1.8f);
	teleportZOffs = cfg.get<float>("VergilAirTrick.teleportZOffs").value_or(-1.3f);
}

void VergilAirTrick::on_config_save(utility::Config& cfg)
{
	cfg.set<float>("VergilAirTrick.initSpeed", initSpeed);
	cfg.set<float>("VergilAirTrick.finishOffsetZ", finishOffsetZ);
	cfg.set<float>("VergilAirTrick.waitTime", waitTime);
	cfg.set<bool>("VergilAirTrick.isCustomWaitTime", isCustomWaitTime);
	cfg.set<bool>("VergilAirTrick.isSpeedUp", isSpeedUp);
	cfg.set<bool>("VergilAirTrick.isCustomOffset", isCustomOffset);
	cfg.set<bool>("VergilAirTrick.isTeleport", isTeleport);
	cfg.set<bool>("VergilAirTrick.isDoppelOppositeTeleport", isDoppelOppositeTeleport);
	cfg.set<int>("VergilAirTrick.trickType", (int)trickType);
	cfg.set<float>("VergilAirTrick.trickCorrection", trickCorrection);
	cfg.set<float>("VergilAirTrick.teleportZOffs", teleportZOffs);
}

// void VergilAirTrick::on_frame(){}

void VergilAirTrick::on_draw_ui()
{
	ImGui::Checkbox("Change Air Trick startup.", &isCustomWaitTime);
	if (isCustomWaitTime) {
          ImGui::TextWrapped("Delay before trick movement starts. 6 - default game value.");
          UI::SliderFloat("##WaitTimeSlider", &waitTime, 0.0f, 20.0f, "%.1f");
	}
	ImGui::Separator();

	ImGui::TextWrapped("Removes Velocity & Acceleration limits on teleportation movement.");
	ImGui::Checkbox("Unclamp Air Trick Speed", &isSpeedUp);
	if (isSpeedUp) {
		ImGui::TextWrapped("High values can cause glitches. 0.7 - default game value.");
          UI::SliderFloat("Air Trick speed", &initSpeed, 0.7f, 3.0f, "%.1f");
	}
	ImGui::Separator();

	ImGui::TextWrapped("Teleport directly to coordinates instead of moving through the air invisibly. Can send you out of bounds (you have been warned).");
	ImGui::Checkbox("Instant Transmission", &isTeleport);
	if (isTeleport)
	{
		ImGui::Spacing();
		ImGui::TextWrapped("Teleport position:");
		ImGui::RadioButton("In front of the enemy", (int*)&trickType, 0);
		ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
		ImGui::RadioButton("Behind the enemy", (int*)&trickType, 1);
		ImGui::Checkbox("Teleport doppelganger to opposite side", &isDoppelOppositeTeleport);
		ImGui::TextWrapped("Distance from enemy:");
		UI::SliderFloat("##CorrectionFinishOffset", &trickCorrection, 0.25f, 2.25f, "%.2f", 1.0F, ImGuiSliderFlags_None);
		ImGui::TextWrapped("Height above/below enemy.");
		UI::SliderFloat("##CorrectionFinishZOffset", &teleportZOffs, -1.5f, 3.25f, "%.2f", 1.0F, ImGuiSliderFlags_None);
	}
	ImGui::Separator();

	ImGui::Checkbox("Custom height offset when finishing Air Trick", &isCustomOffset);
	if (isCustomOffset) {
		UI::SliderFloat("Height offset", &finishOffsetZ, 0.0f, 3.0f, "%.1f");
	}
}

// void VergilAirTrick::on_draw_debug_ui(){}

void VergilAirTrick::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}

