#include "VergilAirTrick.hpp"

bool VergilAirTrick::cheaton = false;
bool VergilAirTrick::isSpeedUp = false;
bool VergilAirTrick::isCustomOffset = false;
bool VergilAirTrick::isCustomWaitTime = false;
//bool VergilAirTrick::isTeleport = false;


float VergilAirTrick::initSpeed = 0.0f;
float VergilAirTrick::finishOffsetZ = 0.0f;
float VergilAirTrick::waitTime      = 0.0f;
//float VergilAirTrick::emCoordX      = 0.0f;
//float VergilAirTrick::emCoordY      = 0.0f;
//float VergilAirTrick::emCoordZ      = 0.0f;
//float VergilAirTrick::playerCoordX  = 0.0f;
//float VergilAirTrick::playerCoordY  = 0.0f;
//float VergilAirTrick::playerCoordZ  = 0.0f;

float specTeleportOffset = 0.25f;


uintptr_t VergilAirTrick::airTrickRet{NULL};
uintptr_t VergilAirTrick::finishOffsetRet{NULL};
uintptr_t VergilAirTrick::waitTimeRet{NULL};
uintptr_t VergilAirTrick::maxXZRet{NULL};
uintptr_t VergilAirTrick::maxSpeedZRet{NULL};
uintptr_t VergilAirTrick::speedAccRet{NULL};
uintptr_t VergilAirTrick::initSpeedRet{NULL};
//uintptr_t VergilAirTrick::teleportRet{NULL};
//uintptr_t VergilAirTrick::teleportRet2{NULL};
//uintptr_t VergilAirTrick::transformRet{NULL};

//uintptr_t VergilAirTrick::playerTransformAddr{NULL};


static naked void waittime_detour() {
	__asm {
		cmp byte ptr [VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr [VergilAirTrick::isCustomWaitTime], 0
		je originalcode

		cheat:
		mov eax, [VergilAirTrick::waitTime]
		jmp qword ptr [VergilAirTrick::waitTimeRet]

		originalcode:
		mov eax,[rsi+0x000000B8]
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
		//cmp byte ptr [VergilAirTrick::isTeleport], 0
		//je originalcode
		cmp byte ptr [VergilAirTrick::isSpeedUp], 0
		je originalcode

		cheat:
		mov eax, [VergilAirTrick::initSpeed]
		jmp qword ptr [VergilAirTrick::initSpeedRet]

		originalcode:
		mov eax,[rsi+0x000000A0]
		jmp qword ptr [VergilAirTrick::initSpeedRet]

		/*teleport:
		mov eax, 0
		jmp qword ptr [VergilAirTrick::initSpeedRet]*/
	}
}

static naked void speed_acc_detour() {
  __asm {
		cmp byte ptr[VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr [VergilAirTrick::isSpeedUp], 0
		je originalcode

		cheat:
		movss xmm1, [VergilAirTrick::modSpeedAcc]
		jmp qword ptr [VergilAirTrick::speedAccRet]

		originalcode:
		movss xmm1,[rsi+0x000000A4]
		jmp qword ptr [VergilAirTrick::speedAccRet]
  }
}

static naked void maxspeedz_detour() {
	__asm {
		cmp byte ptr[VergilAirTrick::cheaton], 0
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
		cmp byte ptr[VergilAirTrick::cheaton], 0
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


//static naked void teleport_detour() {
//	__asm {
//		cmp byte ptr[VergilAirTrick::cheaton], 0
//		je originalcode
//		cmp byte ptr [VergilAirTrick::isTeleport], 0
//		je originalcode
//
//		cheat:
//		mov r8, rax
//		mov rax,[rax+0x000019B0]
//		test rax, rax
//		je cheatret
//		push r10
//		//mov r10, [rax+0x20]//EnemyGameObj
//		//test r10, r10
//		//je badptr
//		mov r10, [r15+0x1F0]//Transform
//		test r10, r10
//		je badptr
//		/*mov r8, [r10+0x30]
//		mov r8, qword ptr [r8]
//		mov qword ptr [VergilAirTrick::emCoordX], r8
//		mov r8, [r10+0x34]
//		mov r8, qword ptr [r8]
//		mov qword ptr [VergilAirTrick::emCoordZ], r8
//		mov r8, [r10+0x38]
//		mov r8, qword ptr [r8]
//		mov qword ptr [VergilAirTrick::emCoordY], r8*/
//		movss xmm0, [r10+0x30]
//		movss xmm1, [r10+0x34]
//		addss xmm1, [specTeleportOffset]
//		movss xmm2, [r10+0x38]
//		mov r8, [r8+0x1F0]//Transform
//		movss [r8+0x30], xmm0
//		movss [r8+0x34], xmm1
//		movss [r8+0x38], xmm2
//		mov r8, [r8+0x78]
//		test r8, r8
//		je badptr
//		movss [r8+0x30], xmm0
//		movss [r8+0x34], xmm1
//		movss [r8+0x38], xmm2
//		//mov r8d, 0x2
//		pop r10
//		jmp cheatret
//
//		originalcode:
//		//movss xmm0,[rax+0x30]
//		mov rax,[rax+0x000019B0]
//		jmp qword ptr [VergilAirTrick::teleportRet]
//
//		badptr:
//		pop r10
//        // pop r8
//
//		cheatret:
//		mov r8d, 0x2
//		jmp qword ptr [VergilAirTrick::teleportRet]
//
//  }
//}

//static naked void teleport2_detour() {
//	__asm {
//		cmp byte ptr[VergilAirTrick::cheaton], 0
//    	je originalcode
//    	cmp byte ptr [VergilAirTrick::isTeleport], 0
//    	je originalcode
//
//		cheat:
//		mov rsi, [rdx+0x150] // EmObj
//		test rsi, rsi
//		je cheatret
//		mov rsi, [rsi+0x18]  // emTransform
//		test rsi, rsi
//		je cheatret
//		movss xmm0, [rsi+0x30]
//   		movss xmm1, [rsi+0x34]
//        // addss xmm1, [specTeleportOffset]
//   		movss xmm2, [rsi+0x38]
//		mov rsi, [rdx+0x30]//Player Transform
//		test rsi, rsi
//		je cheatret
//  		movss [rsi+0x30], xmm0
//        movss [rsi+0x34], xmm1
//        movss [rsi+0x38], xmm2
//
//		cheatret:
//		mov esi, 0x3
//		jmp originalcode
//
//		originalcode:
//		mov r15,r8
//        mov rsi,rdx
//		jmp qword ptr [VergilAirTrick::teleportRet2]
//  }
//}

std::optional<std::string> VergilAirTrick::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	ischecked = &cheaton;
	onpage = vergiltrick;
	full_name_string = "Air Trick settings (+)";
	author_string = "VPZadov";
	description_string = "Change air trick's speed, finish offset and delay before perform.";

	auto waitTimeAddr = utility::scan(base, "07 00 00 8B 86 B8 00 00 00"); //DevilMayCry5.exe+1DDCB5D
    if (!waitTimeAddr) {
          return "Unanable to find AirTrick.waitTime pattern.";
    }

	auto finishOffsetAddr = utility::scan(base, "60 F3 0F 10 86 D0 00 00 00"); //DevilMayCry5.exe+1DDC4FF
    if (!waitTimeAddr) {
          return "Unanable to find AirTrick.finishOffsetAddr pattern.";
    }

	auto initSpeedAddr = utility::scan(base, "08 00 00 8B 86 A0 00 00 00"); //DevilMayCry5.exe+1DDCB43
    if (!initSpeedAddr) {
          return "Unanable to find AirTrick.initSpeedAddr pattern.";
    }

	auto maxSpeedZAddr = utility::scan(base, "0A 00 00 F3 0F 10 86 A8 00 00 00"); //DevilMayCry5.exe+1DDD585
    if (!maxSpeedZAddr) {
          return "Unanable to find AirTrick.maxSpeedZAddr pattern.";
    }

	auto maxXZAddr = utility::scan(base, "06 00 00 F3 0F 10 96 B0 00 00 00"); //DevilMayCry5.exe+1DDCCC5
    if (!maxXZAddr) {
          return "Unanable to find AirTrick.maxXZAddr pattern.";
    }

	auto speedAccAddr = utility::scan(base, "F3 0F 10 8E A4 00 00 00 0F 5A C0 0F 5A C9 48 85 C0 0F 84 40"); //DevilMayCry5.exe+1DDD539
    if (!speedAccAddr) {
          return "Unanable to find AirTrick.speedAccAddr pattern.";
    }

	//auto teleport2Addr = utility::scan(base, "4D 8B F8 48 8B F2 48 8B F9 E8 8D"); //DevilMayCry5.exe+1DDC915
 //   if (!teleport2Addr) {
 //         return "Unanable to find AirTrick.teleport2Addr pattern.";
 //   }

	//auto transformAddr = utility::scan(base, "14 41 8B 06 89 47 30 41 8B 46 04"); //DevilMayCry5.exe+1DDC915
 //   if (!transformAddr) {
 //         return "Unanable to find AirTrick.transformAddr pattern.";
 //   }

	//uintptr_t teleportAddr = finishOffsetAddr.value() + 0x1 + 0x30;

	//--------------------------------------------------------------//

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

	/*if (!install_hook_absolute(teleport2Addr.value(), m_teleport2_hook, &teleport2_detour, &teleportRet2, 0x6)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.teleport2"; 
    }*/

	/*if (!install_hook_absolute(teleportAddr, m_teleport_hook, &teleport_detour, &teleportRet, 0x7)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.teleport"; 
    }*/

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
	//isTeleport = cfg.get<bool>("VergilAirTrick.isTeleport").value_or(false);

}

void VergilAirTrick::on_config_save(utility::Config& cfg)
{
	cfg.set<float>("VergilAirTrick.initSpeed", initSpeed);
	cfg.set<float>("VergilAirTrick.finishOffsetZ", finishOffsetZ);
	cfg.set<float>("VergilAirTrick.waitTime", waitTime);
	cfg.set<bool>("VergilAirTrick.isCustomWaitTime", isCustomWaitTime);
	cfg.set<bool>("VergilAirTrick.isSpeedUp", isSpeedUp);
	cfg.set<bool>("VergilAirTrick.isCustomOffset", isCustomOffset);
	//cfg.set<bool>("VergilAirTrick.isTeleport", isTeleport);
}

void VergilAirTrick::on_frame()
{
}

void VergilAirTrick::on_draw_ui()
{
	ImGui::Checkbox("Change delay before Air Trick.", &isCustomWaitTime);
	if (isCustomWaitTime) {
          ImGui::TextWrapped("Set delay before start trick movement. 6 - game default.");
          ImGui::SliderFloat("##WaitTimeSlider", &waitTime, 0.0f, 20.0f, "%.1f");
	}
	ImGui::Separator();

	ImGui::TextWrapped("Make Air Trick a bit faster. Remove speed limitation and acceleration.");
	ImGui::Checkbox("Faster Air Trick", &isSpeedUp);
	if (isSpeedUp) {
		ImGui::TextWrapped("High speed value can occur movement glitches during trick. 0.7 - default game value.");
          ImGui::SliderFloat("Trick speed", &initSpeed, 0.7f, 3.0f, "%.1f");
	}
	ImGui::Separator();

	/*ImGui::TextWrapped("(UNFINISHED) Default Air Trick mechanic force Vergil to move with high speed. This instantly changes Vergil's coords and teleports him to enemy.");
	ImGui::Checkbox("Instant teleport", &isTeleport);
    if (isTeleport) {
		isSpeedUp = false;
    }*/

	ImGui::Separator();

	ImGui::TextWrapped("Set Z axis offset for finish Air Trick");
	ImGui::Checkbox("Use custom finish Z offset", &isCustomOffset);
	if (isCustomOffset) {
		ImGui::SliderFloat("Z offset", &finishOffsetZ, 0.0f, 3.0f, "%.1f");
	}
}

void VergilAirTrick::on_draw_debug_ui()
{
}

void VergilAirTrick::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
