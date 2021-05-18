#include "EnemySwapper.hpp"


bool EnemySwapper::cheaton{NULL};
bool EnemySwapper::swapAll{NULL};

uintptr_t EnemySwapper::setEnemyDataRet1{NULL};
uintptr_t EnemySwapper::setEnemyDataRet2{NULL};

uint32_t EnemySwapper::selectedToSwap[enemyNum];
uint32_t EnemySwapper::selectedSwapAll{NULL};
uint32_t selectedEnemy = 0;

std::array<EnemySwapper::EnemyId, EnemySwapper::enemyNum> EnemySwapper::swapSettings;

EnemySwapper::EnemyId EnemySwapper::swapForAll;



/*void swap_settings() {

}*/

static naked void enemy_swap_detour1() {//Unfinished stuff, need to make swapper to all enemies type after fix ImGui
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        cmp byte ptr [EnemySwapper::swapAll], 1
        je swapall

        originalcode:
        mov esi,[rcx+0x10]
        jmp ret_jmp

        selectedtoid:
        

        swapall:
        mov esi, [EnemySwapper::swapForAll.id]
        mov [rcx+0x10], esi
        jmp ret_jmp
        
        ret_jmp:
        test rax,rax
        jmp qword ptr [EnemySwapper::setEnemyDataRet1]

  }
}

static naked void enemy_swap_detour2() {
  __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        cmp byte ptr [EnemySwapper::swapAll], 1
        je swapall

        originalcode:
        mov r8d,[rcx+0x10]
        jmp ret_jmp

        selectedtoid:

        swapall:
        mov r8d, [EnemySwapper::swapForAll.id]
        mov [rcx+0x10], r8d
        jmp ret_jmp



        ret_jmp:
        mov rdx,rdi
        jmp qword ptr [EnemySwapper::setEnemyDataRet2]

  }
}

void EnemySwapper::on_config_load(const utility::Config& cfg) {}

void EnemySwapper::on_config_save(utility::Config& cfg) {}

void EnemySwapper::on_frame() {}

void EnemySwapper::on_draw_ui() {
  ImGui::Checkbox("Swap all enemies to:", &swapAll);
  if (swapAll) {
    ImGui::Combo("", (int*)&selectedEnemy, emNames.data(), emNames.size());
    swapForAll.set_id(selectedEnemy);
  } 
  else {
    ImGui::Separator();
    for (int i = 0; i < emNames.size(); i++) {
      ImGui::Text(emNames[i]);
      ImGui::Combo("SwapTo", (int*)&selectedToSwap[i], emNames.data(), emNames.size()); // bug here
      swapSettings[i].set_id(selectedToSwap[i]);
    }
  }

  /*ImGui::Text(emNames[0]);
  ImGui::Combo("Swap to", (int*)&selectedToSwap[0], emNames, IM_ARRAYSIZE(emNames));*/

  /*ImGui::Text(emNames[1].data());
  ImGui::Combo("Swap to", (int*)&selectedToSwap[1], emNames.data(), emNames.size());

  ImGui::Text(emNames[2].data());
  ImGui::Combo("Swap to", (int*)&selectedToSwap[2], emNames.data(), emNames.size());

  ImGui::Text(emNames[3].data());
  ImGui::Combo("Swap to", (int*)&selectedToSwap[3], emNames.data(), emNames.size());

  ImGui::Text(emNames[4].data());
  ImGui::Combo("Swap to", (int*)&selectedToSwap[4], emNames.data(), emNames.size());

  ImGui::Text(emNames[5].data());
  ImGui::Combo("Swap to", (int*)&selectedToSwap[5], emNames.data(), emNames.size());

  ImGui::Text(emNames[6].data());
  ImGui::Combo("Swap to", (int*)&selectedToSwap[6], emNames.data(), emNames.size());

  ImGui::Text(emNames[7].data());
  ImGui::Combo("Swap to", (int*)&selectedToSwap[7], emNames.data(), emNames.size());

  ImGui::Text(emNames[8].data());
  ImGui::Combo("Swap to", (int*)&selectedToSwap[8], emNames.data(), emNames.size());*/


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
  full_name_string   = "Enemy Swapper";
  author_string      = "VPZadov";
  description_string = "Spawn one enemy instead of another. Need to load from a checkpoint or completely restart a mission to take an effect.";
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

    return Mod::on_initialize();
}