#include "MissionManager.hpp"

uint32_t MissionManager::missionNumber{NULL};

uintptr_t MissionManager::missionNumRet{NULL};

static naked void mission_num_detour() {
	__asm {
		 mov [rbx+0x7C], eax
		 mov [MissionManager::missionNumber], eax
         mov rbx,[rsp+0x40]
		 jmp qword ptr [MissionManager::missionNumRet]
  }
}

std::optional<std::string> MissionManager::on_initialize() {
  init_check_box_info();
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  auto missionNumAddr = patterns.find_addr(base, "89 43 7C 48 8B 5C 24 40");//DevilMayCry5.exe+24A9311
  if (!missionNumAddr) {
    return "Unanable to find missionNumAddr pattern.";
  }

  if (!install_hook_absolute(missionNumAddr.value(), m_mission_number_hook, mission_num_detour, &missionNumRet, 0x8)){
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize missionNum";  
  }

  return Mod::on_initialize();
}

void MissionManager::on_config_load(const utility::Config& cfg) {}

void MissionManager::on_config_save(utility::Config& cfg) {}

void MissionManager::on_frame() {}

void MissionManager::on_draw_ui() {}

void MissionManager::on_draw_debug_ui() {}

void MissionManager::init_check_box_info() {}
