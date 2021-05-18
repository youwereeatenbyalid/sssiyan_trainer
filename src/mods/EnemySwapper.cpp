#include "EnemySwapper.hpp"
#include <string>

bool EnemySwapper::cheaton{NULL};
bool EnemySwapper::swapAll{NULL};

uintptr_t EnemySwapper::setEnemyDataRet1{NULL};
uintptr_t EnemySwapper::setEnemyDataRet2{NULL};

uint32_t EnemySwapper::selectedToSwap[enemyListCount];
uint32_t EnemySwapper::selectedSwapAll{NULL};
uint32_t selectedForAllSwap = 0;//selected indx for all swap
uint32_t EnemySwapper::currentEnemyId = 0;//Enemy Id from asm
uint32_t EnemySwapper::newEnemyId     = 0;//Enemy Id to swap in current state

std::string uniqComboStr = "";//For comboboxes

std::array<EnemySwapper::EnemyId, EnemySwapper::enemyListCount> EnemySwapper::swapSettings;//cur id - id to swap

EnemySwapper::EnemyId EnemySwapper::swapForAll;


 
 void custom_swap_setup() {
 for (int i = 0; i < EnemySwapper::swapSettings.size(); i++) {
      if (EnemySwapper::currentEnemyId == EnemySwapper::swapSettings[i].get_current_id()) {
        EnemySwapper::newEnemyId = EnemySwapper::swapSettings[i].get_swap_id();
        break;
      } else
        EnemySwapper::newEnemyId = EnemySwapper::currentEnemyId;
  }
}

static naked void enemy_swap_detour1() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        cmp byte ptr [EnemySwapper::swapAll], 1
        je swapall

        swapsettings:
        mov esi,[rcx+0x10]
        mov [EnemySwapper::currentEnemyId], esi
        mov [EnemySwapper::newEnemyId], esi

        mov [EnemySwapper::enemySwapBackup1.rax], rax
        mov [EnemySwapper::enemySwapBackup1.rbx], rbx
        mov [EnemySwapper::enemySwapBackup1.rcx], rcx
        mov [EnemySwapper::enemySwapBackup1.rdx], rdx
        //mov [EnemySwapper::enemySwapBackup1.rsi], rsi

        call [custom_swap_setup]

        mov rax, [EnemySwapper::enemySwapBackup1.rax]
        mov rbx, [EnemySwapper::enemySwapBackup1.rbx]
        mov rcx, [EnemySwapper::enemySwapBackup1.rcx]
        mov rdx, [EnemySwapper::enemySwapBackup1.rdx]
        //mov rsi, [EnemySwapper::enemySwapBackup1.rsi]
        mov esi, [EnemySwapper::newEnemyId]
        mov [rcx+0x10], esi
        xor esi, esi

        originalcode:
        mov esi,[rcx+0x10]
        test rax,rax
        jmp qword ptr [EnemySwapper::setEnemyDataRet1]
        

        swapall:
        mov esi, [EnemySwapper::swapForAll.swapId]
        mov [rcx+0x10], esi
        //mov dword ptr [rcx+0x20], 4//EnemyNum
        jmp originalcode

  }
}

static naked void enemy_swap_detour2() {
  __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        cmp byte ptr [EnemySwapper::swapAll], 1
        je swapall

        swapsettings:
        mov r8d,[rcx+0x10]
        mov [EnemySwapper::currentEnemyId], r8d
        mov [EnemySwapper::newEnemyId], r8d

        mov [EnemySwapper::enemySwapBackup2.rax], rax
        mov [EnemySwapper::enemySwapBackup2.rbx], rbx
        mov [EnemySwapper::enemySwapBackup2.rcx], rcx
        mov [EnemySwapper::enemySwapBackup2.rdx], rdx
        mov [EnemySwapper::enemySwapBackup2.rsi], rsi
        mov [EnemySwapper::enemySwapBackup2.rsi], rsi

        call [custom_swap_setup]

        mov rax, [EnemySwapper::enemySwapBackup2.rax]
        mov rbx, [EnemySwapper::enemySwapBackup2.rbx]
        mov rcx, [EnemySwapper::enemySwapBackup2.rcx]
        mov rdx, [EnemySwapper::enemySwapBackup2.rdx]
        mov rsi, [EnemySwapper::enemySwapBackup2.rsi]

        mov r8d, [EnemySwapper::newEnemyId]
        mov [rcx+0x10], r8d
        xor r8d, r8d

        originalcode:
        mov r8d,[rcx+0x10]
        mov rdx,rdi
        jmp qword ptr [EnemySwapper::setEnemyDataRet2]

        swapall:
        mov r8d, [EnemySwapper::swapForAll.swapId]
        mov [rcx+0x10], r8d
        //mov dword ptr [rcx+0x20], 4//EnemyNum
        jmp originalcode

  }
}

void EnemySwapper::on_config_load(const utility::Config& cfg) {
  cheaton = cfg.get<bool>("EnemySwapper").value_or(false);
  selectedForAllSwap = cfg.get<uint32_t>("SwapAllEnemiesToID").value_or(0);
  std::string key;
  for (int i = 0; i < EnemySwapper::emNames.size(); i++) {
    key = std::string(EnemySwapper::emNames[i]) + "_swapTo";
    selectedToSwap[i] = cfg.get<uint32_t>(key).value_or(i);
  }
}

void EnemySwapper::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("EnemySwapper", cheaton);
  cfg.set<uint32_t>("SwapAllEnemiesToID", selectedForAllSwap);
  for (int i = 0; i < EnemySwapper::emNames.size(); i++) {
    cfg.set<uint32_t>(std::string(EnemySwapper::emNames[i]) + "_swapTo", selectedToSwap[i]);
  }
}

void EnemySwapper::on_frame() {}

void restore_default() {
  for (int i = 0; i < EnemySwapper::enemyListCount; i++)
    EnemySwapper::selectedToSwap[i] = i;
}

void EnemySwapper::on_draw_ui() {
  ImGui::Checkbox("Swap all enemies to:", &swapAll);
  if (swapAll) {
    ImGui::Combo("", (int*)&selectedForAllSwap, emNames.data(), emNames.size(),20);
    swapForAll.set_swap_id(selectedForAllSwap);
  } 
  else {
    ImGui::Separator();
    if (ImGui::Button("Restore default", ImVec2(135, 25)))
      restore_default();
    for (int i = 0; i < emNames.size(); i++) {
      ImGui::Text(emNames[i]);
      uniqComboStr = std::to_string(i) + "#SwapTo";
      ImGui::Combo(uniqComboStr.c_str(), (int*)&selectedToSwap[i], emNames.data(), emNames.size(), 20);
      swapSettings[i].set_current_id(i);
      swapSettings[i].set_swap_id(selectedToSwap[i]);
      ImGui::Separator();
    }
  }
}

void EnemySwapper::on_draw_debug_ui() {}

void EnemySwapper::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> EnemySwapper::on_initialize() {
  init_check_box_info();
  auto base      = g_framework->get_module().as<HMODULE>(); // note HMODULE
  ischecked = &cheaton;
  onpage         = balance;
  full_name_string   = "Enemy Swapper (+)";
  author_string      = "VPZadov";
  description_string = "Spawn one enemy instead of another. Need to load mission from a checkpoint or completely restart a mission to take an effect.";

  auto initAddr1 = utility::scan(base, "8B 71 10 48 85 C0 0F 84 43");// "DevilMayCry5.exe"+FE568B
  if (!initAddr1) {
    return "Unanable to find EnemySwapper pattern.";
  }

  auto initAddr2 = utility::scan(base, "44 8B 41 10 48 8B D7 48 8B CB E8 F8"); // DevilMayCry5.exe+FE57A9 
  if (!initAddr2) {
    return "Unanable to find EnemySwapper pattern.";
  }

  if (!install_hook_absolute(initAddr1.value(), m_enemy_swapper_hook1, &enemy_swap_detour1, &setEnemyDataRet1, 0x6)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.init"; 
  }

  if (!install_hook_absolute(initAddr2.value(), m_enemy_swapper_hook2, &enemy_swap_detour2, &setEnemyDataRet2, 0x7)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr2"; 
  }

  /*for (int i = 0; i < enemyListCount; i++) {
    selectedToSwap[i] = i;
  }*/
    return Mod::on_initialize();
}