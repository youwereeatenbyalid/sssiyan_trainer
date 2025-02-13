#include "CheckpointPos.hpp"
#include "MissionManager.hpp"
#include "EnemySwapper.hpp"
//clang-format off

bool CheckpointPos::cheaton{NULL};
bool CheckpointPos::isCustomPos{NULL};
bool CheckpointPos::isNoBoss{NULL};

uint32_t CheckpointPos::missionN{NULL};

uintptr_t CheckpointPos::restartPos_ret{NULL};

Vector3f CheckpointPos::customPos;
Vector3f CheckpointPos::newRestartPos;
Vector3f CheckpointPos::playerPos;

uintptr_t plCoordBase;
uintptr_t addr;
std::array<uintptr_t, 6> coordsOffsets{0x70, 0xC70, 0x40, 0xE40, 0x80, 0x30};
uintptr_t printOffs6 = 0;

void load_boss_checkpoint() {
  CheckpointPos::isNoBoss = true;

  if (CheckpointPos::isCustomPos) {
    CheckpointPos::isNoBoss = false;
    CheckpointPos::newRestartPos = CheckpointPos::customPos;
    return;
  }
  for (int i = 0; i < CheckpointPos::mPosData.size(); i++) {
      if (CheckpointPos::mPosData[i].mNumber == MissionManager::missionNumber) {
      CheckpointPos::isNoBoss = false;
        CheckpointPos::newRestartPos = CheckpointPos::mPosData[i].pos;
      return;
      }
  }
}

static naked void restart_pos_detour() {
    __asm {
        cmp byte ptr [CheckpointPos::cheaton], 01
        je cheat

        originalcode:
        movaps [rsp+0x70],xmm9
        jmp qword ptr [CheckpointPos::restartPos_ret]

        cheat:
        push rax
		push rcx
        sub rsp, 32
        call [load_boss_checkpoint]
        add rsp, 32
		pop rcx
		pop rax
        cmp byte ptr [CheckpointPos::isNoBoss], 1
        je originalcode
        movss xmm6, [CheckpointPos::newRestartPos.x]
        movss xmm7, [CheckpointPos::newRestartPos.z]
        movss xmm8, [CheckpointPos::newRestartPos.y]
        jmp originalcode
  }
}

std::optional<std::string> CheckpointPos::on_initialize() {
  init_check_box_info();
  auto base        = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled        = &cheaton;
  m_on_page           = Page_GameMode;
  m_full_name_string = "Custom Checkpoints (+)";
  m_author_string    = "VPZadov";
  m_description_string = "Create a custom checkpoint to reset to. Uses boss checkpoints by default.";

  plCoordBase = g_framework->get_module().as<uintptr_t>() + 0x07E625D0;

  auto restartPosAddr = m_patterns_cache->find_addr(base, "44 0F 29 4C 24 70 F3 44 0F 10 48"); // DevilMayCry5.exe+24A4DB7
  if (!restartPosAddr) {
    return "Unanable to find CheckpointPos.restartPosAddr pattern.";
  }

  if (!install_hook_absolute(restartPosAddr.value(), m_checkpointpos_hook, &restart_pos_detour, &restartPos_ret, 0x6)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize CheckpointPos.restartPos"; 
  }

  return Mod::on_initialize();
}

void CheckpointPos::on_config_load(const utility::Config& cfg) {
  isCustomPos = cfg.get<bool>("CheckpointPos.isCustomPos").value_or(false);
  customPos.x = cfg.get<float>("CheckpointPos.CustomPosX").value_or(0.0f);
  customPos.y = cfg.get<float>("CheckpointPos.CustomPosY").value_or(0.0f);
  customPos.z = cfg.get<float>("CheckpointPos.CustomPosZ").value_or(0.0f);
}

void CheckpointPos::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("CheckpointPos.isCustomPos", isCustomPos);
  cfg.set<float>("CheckpointPos.CustomPosX", customPos.x);
  cfg.set<float>("CheckpointPos.CustomPosY", customPos.y);
  cfg.set<float>("CheckpointPos.CustomPosZ", customPos.z);
}

// void CheckpointPos::on_frame() {}

void CheckpointPos::on_draw_ui() {
  ImGui::TextWrapped("Select \"Checkpoint\" in mission menu (or press \"Continue\" from main menu) to load at the coordinates below.");
  ImGui::Separator();
  ImGui::Checkbox("Use custom checkpoint position", &isCustomPos);
  ImGui::ShowHelpMarker("If you leave this box unchecked, you'll spawn near the boss in your selected stage.");
  if (isCustomPos) {
    ImGui::InputFloat("X coord", &customPos.x, 0.0f, 0.0f, "%.3f");
    ImGui::InputFloat("Y coord", &customPos.y, 0.0f, 0.0f, "%.3f");
    ImGui::InputFloat("Z coord", &customPos.z, 0.0f, 0.0f, "%.3f");
    if (ImGui::Button("Get current player position (don't click it if you are not in a mission)")) {
      if (EnemySwapper::nowFlow == 22)//gameplayFlow, btw still can crash if click while loading screen after gameplay
        playerPos = get_player_coords();
      }
    ImGui::TextWrapped("X: %.3f", playerPos.x);
    ImGui::TextWrapped("Y: %.3f", playerPos.y);
    ImGui::TextWrapped("Z: %.3f", playerPos.z);
    if (ImGui::Button("Use as custom position")) {
      customPos = playerPos;
    }
    }
  ImGui::Separator();
  ImGui::TextWrapped("Set player's position:");
  ImGui::DragFloat3("##TeleportPos", (float*)&newPlPos);
  ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
  if (ImGui::Button("Get current player's position"))
  {
      if (EnemySwapper::nowFlow == 22)//gameplayFlow, btw still can crash if click while loading screen after gameplay
          newPlPos = get_player_coords();
  }
  ImGui::Spacing();
  if (ImGui::Button("Get boss pos"))
      newPlPos = get_boss_pos();
  ImGui::Spacing();
  
  if (ImGui::Button("Set position"))
      {
          if(PlayerTracker::playerentity == 0 || EnemySwapper::nowFlow != 22)
              return;
          GameFunctions::Transform_SetPosition::set_player_pos(PlayerTracker::playerentity, newPlPos);
      }
  ImGui::ShowHelpMarker("It's actually instant teleport to a selected pos, so it can throw character in out of bounds.");
  }

// void CheckpointPos::on_draw_debug_ui() {}

void CheckpointPos::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

Vector3f CheckpointPos::get_boss_pos()
{
    for (int i = 0; i < CheckpointPos::mPosData.size(); i++)
    {
        if (CheckpointPos::mPosData[i].mNumber == MissionManager::missionNumber)
            return CheckpointPos::mPosData[i].pos;
    }
}

uintptr_t CheckpointPos::get_player_coords_ptr(uintptr_t addr) {
  auto offs = *(uintptr_t*)(addr + coordsOffsets[0]);
  if (offs == 0) // It's actually doesn't help)
    return 0;
  for (int i = 1; i < coordsOffsets.size() - 1; i++) {
    offs = *(uintptr_t*)(offs + coordsOffsets[i]);
  }
  return offs;
}

bool CheckpointPos::is_null_ptr(uintptr_t ptr) {
  if (ptr == 0) {
    playerPos.x = 0.0f;
    playerPos.y = 0.0f;
    playerPos.z = 0.0f;
    return true;
  }
    return false;
}

Vector3f CheckpointPos::get_player_coords() {
    uintptr_t lastOffset;
  uintptr_t offs = coordsOffsets[coordsOffsets.size() - 1];
   uintptr_t resHex;
   Vector3f res{0,0,0};
   auto address = *(uintptr_t*)plCoordBase;
   lastOffset = get_player_coords_ptr(address);
   if (is_null_ptr(lastOffset))
     return res;
   resHex = *(uintptr_t*)(lastOffset + offs);
   res.x  = *((float*)&resHex);
   resHex = *(uintptr_t*)(lastOffset + offs + 0x4);
   res.z  = *((float*)&resHex);
   resHex = *(uintptr_t*)(lastOffset + offs + 0x8);
   res.y  = *((float*)&resHex);
   return res;
}
//clang-format on
