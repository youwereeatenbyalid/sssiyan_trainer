#include "EnemySwapper.hpp"
#include <string>

bool EnemySwapper::cheaton{NULL};
bool EnemySwapper::isSwapAll{NULL};
bool EnemySwapper::isCustomRandomSettings{false};
bool EnemySwapper::isCustomSeed{false};
bool EnemySwapper::isCustomSpawnPos{NULL};

bool resetSeed = false;

uintptr_t EnemySwapper::setEnemyDataRet1{NULL};
uintptr_t EnemySwapper::setEnemyDataRet2{NULL};
uintptr_t EnemySwapper::posSpawnRet{NULL};
uintptr_t EnemySwapper::posSpawnTestJne{NULL};

uint32_t EnemySwapper::selectedToSwap[enemyListCount];
uint32_t EnemySwapper::selectedSwapAll{NULL};
uint32_t selectedForAllSwap = 0;//selected indx for all swap
uint32_t EnemySwapper::currentEnemyId = 0;//Enemy Id from asm
uint32_t EnemySwapper::newEnemyId     = 0;//Enemy Id to swap in current state

float EnemySwapper::spawnPosZOffset = 0.0f;
float EnemySwapper::curSpawnPosZ = 0.0f;

std::string uniqComboStr = "";//For comboboxes

std::array<EnemySwapper::EnemyId, EnemySwapper::enemyListCount> EnemySwapper::swapSettings;//cur id - id to swap

EnemySwapper::EnemyId EnemySwapper::swapForAll;

std::vector<uintptr_t> *setDataAddrs;

uintptr_t curSetDataAddr = 0;


 
 void custom_swap_setup() {
  EnemySwapper::newEnemyId = EnemySwapper::currentEnemyId;
   if (setDataAddrs->size() != 0) {
    for (int i = 0; i < setDataAddrs->size(); i++) {
       if (curSetDataAddr == (*setDataAddrs)[i])
        return;
    }
  }
 for (int i = 0; i < EnemySwapper::swapSettings.size(); i++) {
     EnemySwapper::newEnemyId = EnemySwapper::currentEnemyId;
      if (EnemySwapper::currentEnemyId == EnemySwapper::swapSettings[i].get_current_id()) {
       if (EnemySwapper::currentEnemyId != EnemySwapper::swapSettings[i].get_swap_id()) {
          EnemySwapper::newEnemyId = EnemySwapper::swapSettings[i].get_swap_id();
         setDataAddrs->push_back(curSetDataAddr);
       }
        break;
      }
  }
}

static naked void enemy_swap_detour1() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall

        swapsettings:
        mov [curSetDataAddr], rcx
        mov esi,[rcx+0x10]
        mov [EnemySwapper::currentEnemyId], esi

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
        mov esi, dword ptr [EnemySwapper::newEnemyId]
        mov [rcx+0x10], esi

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
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall

        swapsettings:
        mov r8d,[rcx+0x10]
        mov [EnemySwapper::currentEnemyId], r8d
        mov qword ptr [curSetDataAddr], rcx
        mov [EnemySwapper::enemySwapBackup2.rax], rax
        mov [EnemySwapper::enemySwapBackup2.rbx], rbx
        mov [EnemySwapper::enemySwapBackup2.rcx], rcx
        mov [EnemySwapper::enemySwapBackup2.rdx], rdx
        mov [EnemySwapper::enemySwapBackup2.rsi], rsi

        call [custom_swap_setup]

        mov rax, [EnemySwapper::enemySwapBackup2.rax]
        mov rbx, [EnemySwapper::enemySwapBackup2.rbx]
        mov rcx, [EnemySwapper::enemySwapBackup2.rcx]
        mov rdx, [EnemySwapper::enemySwapBackup2.rdx]
        mov rsi, [EnemySwapper::enemySwapBackup2.rsi]

        mov r8d, dword ptr [EnemySwapper::newEnemyId]
        mov [rcx+0x10], r8d

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

static void clear_setData_addrs() {
  if (setDataAddrs->size() != 0)
    setDataAddrs->clear();
}

static naked void spawn_pos_detour() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        cmp byte ptr [EnemySwapper::isSwapAll], 0
        je vectorclear
        cmp byte ptr [EnemySwapper::isCustomSpawnPos], 1
        je cheat

        vectorclear:
        mov [EnemySwapper::spawnPosBackup.rax], rax
        mov [EnemySwapper::spawnPosBackup.rbx], rbx
        mov [EnemySwapper::spawnPosBackup.rcx], rcx
        mov [EnemySwapper::spawnPosBackup.rdx], rdx
        mov [EnemySwapper::spawnPosBackup.rsi], rsi
        call [clear_setData_addrs]
        mov rax, [EnemySwapper::spawnPosBackup.rax]
        mov rbx, [EnemySwapper::spawnPosBackup.rbx]
        mov rcx, [EnemySwapper::spawnPosBackup.rcx]
        mov rdx, [EnemySwapper::spawnPosBackup.rdx]
        mov rsi, [EnemySwapper::spawnPosBackup.rsi]
        cmp byte ptr [EnemySwapper::isCustomSpawnPos], 0
        je originalcode

        cheat:
        movss [EnemySwapper::curSpawnPosZ], xmm1
        addss xmm1, [EnemySwapper::spawnPosZOffset]
        movss [r13+0x34], xmm1

        originalcode:
        test rcx,rcx
        jne shortjne
        jmp qword ptr [EnemySwapper::posSpawnRet]

        shortjne:
        jmp qword ptr [EnemySwapper::posSpawnTestJne]

  }
}

void EnemySwapper::set_swapper_setting(int emListIndx) {
  swapSettings[emListIndx].set_current_id(emListIndx);
  swapSettings[emListIndx].set_swap_id(selectedToSwap[emListIndx]);
}

void EnemySwapper::set_swapper_settings(std::array<int ,enemyListCount> &swapToIndx)
{
  for (auto& indx : swapToIndx) {
    set_swapper_setting(indx);
  }
}


void EnemySwapper::on_config_load(const utility::Config& cfg) {
  cheaton = cfg.get<bool>("EnemySwapper").value_or(false);
  isSwapAll          = cfg.get<bool>("EnemySwapper.isSwapAll").value_or(false);
  selectedForAllSwap = cfg.get<uint32_t>("SwapAllEnemiesToID").value_or(0);
  isCustomSpawnPos = cfg.get<bool>("EnemySwapper.isCustomSpawnPos").value_or(false);
  spawnPosZOffset  = cfg.get<float>("EnemySwapper.spawnPosZOffset").value_or(0.0f);
  std::string key;
  for (int i = 0; i < EnemySwapper::emNames.size(); i++) {
    key = std::string(EnemySwapper::emNames[i]) + "_swapTo";
    selectedToSwap[i] = cfg.get<uint32_t>(key).value_or(i);
    set_swapper_setting(i);
  }
}

void EnemySwapper::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("EnemySwapper", cheaton);
  cfg.set<uint32_t>("SwapAllEnemiesToID", selectedForAllSwap);
  cfg.set<bool>("EnemySwapper.isCustomSpawnPos", isCustomSpawnPos);
  cfg.set<float>("EnemySwapper.spawnPosZOffset", spawnPosZOffset);
  for (int i = 0; i < EnemySwapper::emNames.size(); i++) {
    cfg.set<uint32_t>(std::string(EnemySwapper::emNames[i]) + "_swapTo", selectedToSwap[i]);
  }
}

void EnemySwapper::on_frame() {}

void EnemySwapper::restore_default_settings() {
  for (int i = 0; i < EnemySwapper::enemyListCount; i++)
    EnemySwapper::selectedToSwap[i] = i;
  EnemySwapper::curMinIndx = minIndx;
  EnemySwapper::curMaxIndx = maxIndx;
  seed       = -1;
}

void EnemySwapper::on_draw_ui() {
  ImGui::TextWrapped("Use custom offset to increase z spawn coord to fix spawning flying enemies under the floor. Note that this will affect all enemies.");
  ImGui::Checkbox("Offset for Z spawn coord", &isCustomSpawnPos);
  if (isCustomSpawnPos) {
    ImGui::TextWrapped("Add Z offset");
    ImGui::SliderFloat("##spawnPosZOffsetSlider", &spawnPosZOffset, 0.0f, 6.0f, "%.01f");
  }

  ImGui::Separator();

  ImGui::Checkbox("Swap all enemies to:", &isSwapAll);
  if (isSwapAll) {
    ImGui::Combo("", (int*)&selectedForAllSwap, emNames.data(), emNames.size(),20);
    swapForAll.set_swap_id(selectedForAllSwap);
  } 
  else {
    ImGui::Separator();
    if (ImGui::Button("Restore default", ImVec2(135, 25)))
      restore_default_settings();
    ImGui::Checkbox("Custom random settings", &isCustomRandomSettings);
    if (isCustomRandomSettings) {
      ImGui::Checkbox("Use custom seed", &isCustomSeed);
      if (isCustomSeed) {
        ImGui::Text("Seed:");
        if (ImGui::SliderInt("##seedSlider", &seed, 0, 10000))
          seed_rnd_gen(seed);
        if (ImGui::Button("Use \"std::time(0)\" as a seed")) {
          seed      = std::time(0);
          seed_rnd_gen(seed);
        }
      } else
        seed = -1;
      ImGui::TextWrapped("Min enemy index:");
      ImGui::SliderInt("##minIndxSlider", &curMinIndx, 0, 39);
      ImGui::TextWrapped("Max enemy index:");
      ImGui::SliderInt("##maxIndxSlider", &curMaxIndx, 1, 40);
      if (curMinIndx >= curMaxIndx)
        curMinIndx = curMaxIndx - 1;
    } else {
      curMinIndx = minIndx;
      curMaxIndx = maxIndx;
    }
    if (ImGui::Button("Random regular enemies", ImVec2(165, 25))) {
      /*if (resetSeed)
        seed_rnd_gen(seed);*/
      random_em_swap(curMinIndx, curMaxIndx);
    }

    for (int i = 0; i < emNames.size(); i++) {
      ImGui::TextWrapped(emNames[i]);
      uniqComboStr = std::to_string(i) + "##SwapTo";
      ImGui::Combo(uniqComboStr.c_str(), (int*)&selectedToSwap[i], emNames.data(), emNames.size(), 20);
      set_swapper_setting(i);
      ImGui::Separator();
    }
  }
}

void EnemySwapper::on_draw_debug_ui() {
  ImGui::TextWrapped("Vector size is: %X", setDataAddrs->size());
}

void EnemySwapper::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

void EnemySwapper::seed_rnd_gen(int seed) {
  if (resetSeed) {
    if (seed == -1)
      gen.seed(rd());
    else
      gen.seed(seed);
  }
}

void EnemySwapper::random_em_swap(uint32_t min, uint32_t max) {
  std::uniform_int_distribution<> distrib(min, max);
  for (int i = 0; i < 20; i++) {//Rand only for regular enemies
    selectedToSwap[i] = distrib(gen);
  }
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

  //auto tempcustomSpawnAddr = g_framework->get_module().as<uintptr_t>() + 0x11C58BC;

  auto customSpawnAddr = utility::scan(base, "F3 41 0F11 4D 34 48 8B 4B 18 48 85 C9 75 10"); // DevilMayCry5.exe+11C58BC
  if (!customSpawnAddr) {
    return "Unanable to find customSpawnAddr pattern.";
  }
  const uintptr_t spawnAddrOffset = 0xA;

  posSpawnTestJne = customSpawnAddr.value() + spawnAddrOffset + 0x15;

  if (!install_hook_absolute(initAddr1.value(), m_enemy_swapper_hook1, &enemy_swap_detour1, &setEnemyDataRet1, 0x6)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr1"; 
  }

  if (!install_hook_absolute(initAddr2.value(), m_enemy_swapper_hook2, &enemy_swap_detour2, &setEnemyDataRet2, 0x7)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr2"; 
  }

  if (!install_hook_absolute(customSpawnAddr.value() + spawnAddrOffset, m_spawn_pos_hook, &spawn_pos_detour, &posSpawnRet, 0x5)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.customSpawnAddr"; 
  }

  seed_rnd_gen(-1);
  setDataAddrs = new std::vector<uintptr_t>();
    return Mod::on_initialize();
}