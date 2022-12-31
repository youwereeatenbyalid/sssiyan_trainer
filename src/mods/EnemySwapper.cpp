#include "EnemySwapper.hpp"
#include <string>
#include "sdk/ReClass.hpp"
#include "EnemyDataSettings.hpp"
#include "EnemySpawner.hpp"
#include "BossVergilMoves.hpp"
#include "PlayerTracker.hpp"

bool EnemySwapper::cheaton{false};
bool EnemySwapper::isSwapAll{false};
bool EnemySwapper::isCustomRandomSettings{false};
bool EnemySwapper::isCustomSeed{false};

bool isReswap = false;
bool isSkipNum = false;
bool checkForReswap = false;
bool defaultEmSetting = true;
int index = 0;

uintptr_t EnemySwapper::setEnemyDataRet1{NULL};
uintptr_t EnemySwapper::setEnemyDataRet2{NULL};
uintptr_t EnemySwapper::setEnemyDataRet3{NULL};
uintptr_t EnemySwapper::setEnemyDataRet5{NULL};
uintptr_t EnemySwapper::setEnemyDataRet6{NULL};

uint32_t EnemySwapper::selectedToSwap[enemyListCount];
uint32_t EnemySwapper::selectedSwapAll{NULL};
uint32_t selectedForAllSwap = 0;//selected indx for all swap
uint32_t EnemySwapper::currentEnemyId = 0;//Enemy Id from asm
uint32_t EnemySwapper::newEnemyId     = 0;//Enemy Id to swap in current state

uint32_t EnemySwapper::newEnemyId1 = 0;
uint32_t EnemySwapper::newEnemyId2 = 0;
uint32_t EnemySwapper::newEnemyId3 = 0;
uint32_t EnemySwapper::newEnemyId5 = 0;
uint32_t EnemySwapper::newEnemyId6 = 0;
float EnemySwapper::waitTimeMin       = 0.0f;
float EnemySwapper::waitTimeMax       = 0.0f;
float EnemySwapper::odds              = 100.0f;
int EnemySwapper::enemyNum            = 1;
static std::string uniqStr = "";
std::string uniqComboStr = "";//For comboboxes

std::array<EnemySwapper::EnemyId, EnemySwapper::enemyListCount> EnemySwapper::swapSettings;//cur id - id to swap

EnemySwapper::EnemyId EnemySwapper::swapForAll;

std::vector<uintptr_t> EnemySwapper::swapDataAddrs;

uintptr_t curSetDataAddr = 0;

 bool skip_reswap() {
  if (EnemySwapper::swapDataAddrs.size() != 0) {
     for (int i = 0; i < EnemySwapper::swapDataAddrs.size(); i++) {
      if (curSetDataAddr == EnemySwapper::swapDataAddrs[i]) {
        return true;
      }
    }
  }
  return false;
}
 
  static uint32_t custom_swap_setup() {
    uint32_t res = EnemySwapper::currentEnemyId;
    if (skip_reswap()) {
      return res;
    }
 for (int i = 0; i < EnemySwapper::swapSettings.size(); i++) {
      if (EnemySwapper::currentEnemyId == EnemySwapper::swapSettings[i].get_current_id()) {
       if (EnemySwapper::currentEnemyId != EnemySwapper::swapSettings[i].get_swap_id()) {
         res = EnemySwapper::swapSettings[i].get_swap_id();
          EnemySwapper::swapDataAddrs.push_back(curSetDataAddr);
       }
       return res;
      }
  }
 return res;
 }

 static void enemydata_settings_setup() {
   defaultEmSetting = true;
   for (auto const &em : EnemySwapper::enemySettings) {
     if (em.emId.get_current_id() == EnemySwapper::currentEnemyId) {
       if (em.useDefault) {
         return;
       }
       defaultEmSetting = false;
       EnemySwapper::waitTimeMin = em.waitTimeMin;
       EnemySwapper::waitTimeMax = em.waitTimeMax;
       EnemySwapper::enemyNum = em.enemyNum;
       EnemySwapper::odds = em.odds;
       return;
     }
   }
 }

static naked void enemy_swap_detour1() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall

        /*checkdataoption:
        cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata
        jmp originalcode*/

        swapsettings:
        cmp dword ptr [GameplayStateTracker::gameMode], 3
        jne originalcode
        //mov byte ptr [r15+0x5C], 00  // IsEndSaveData
        mov [curSetDataAddr], rcx
        //mov byte ptr [checkForReswap], 1
        mov esi,[rcx+0x10]
        mov [EnemySwapper::currentEnemyId], esi

        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32
        call [custom_swap_setup]
        add rsp, 32
        //mov dword ptr [rcx+0x10], eax
        mov [EnemySwapper::newEnemyId1], eax
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax

        mov esi, dword ptr [EnemySwapper::newEnemyId1]
        mov [rcx+0x10], esi
        jmp originalcode//setup

        originalcode:
        mov esi,[rcx+0x10]
        test rax,rax
        jmp qword ptr [EnemySwapper::setEnemyDataRet1]   

        swapall:
        mov esi, [EnemySwapper::swapForAll.swapId]
        mov [rcx+0x10], esi
        //mov dword ptr [rcx+0x20], 4//EnemyNum
        jmp originalcode//setup

  }
}

static naked void enemy_swap_detour2() {
  __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode//checkdataoption
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall
        jmp swapsettings

        swapsettings:
        //cmp byte ptr [EnemySwapper::isBp], 1
        //je originalcode
       // mov byte ptr [checkForReswap], 1
        cmp dword ptr [GameplayStateTracker::gameMode], 3
        jne originalcode
        //mov byte ptr [r15+0x5C], 00 // IsEndSaveData

        mov r8d, [rcx+0x10]
        mov [EnemySwapper::currentEnemyId], r8d
        //mov [EnemySwapper::newEnemyId], r8d
        mov [curSetDataAddr], rcx

        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32

        call qword ptr [custom_swap_setup]

        add rsp, 32
        //mov dword ptr [rcx+0x10], eax
        mov [EnemySwapper::newEnemyId2], eax
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax

        mov r8d, dword ptr [EnemySwapper::newEnemyId2]
        mov [rcx+0x10], r8d
        //cmp byte ptr [EnemyDataSettings::cheaton], 1
        //je setupemdata
        jmp originalcode//setup
        //mov byte ptr [r15+0xF8], 1//isResourceLoad
        //mov dword ptr [rcx+0x54], 0x4

        originalcode:
        mov r8d,[rcx+0x10]
        mov rdx,rdi
        jmp qword ptr [EnemySwapper::setEnemyDataRet2]

        //setupemdata:
        //mov byte ptr [r15+0x52], 01//IsInit
        //mov byte ptr [r15+0xB2], 00 // isReadSealMode
        //mov byte ptr [r15+0x55], 01 // IsManualPlayerInTargetArea
        //mov byte ptr [r15+0x5E], 01 // IsCheckLoadRequest
        //mov byte ptr [r15+0xCC], 00 // IsPause
        //mov byte ptr [r15+0x2A], 00 // IsNearThePlayer
        //mov byte ptr [r15+0xCD], 01 // IsREquestEndGenerate
        

        swapall:
        mov r8d, [EnemySwapper::swapForAll.swapId]
        mov [rcx+0x10], r8d
        //cmp byte ptr [EnemyDataSettings::cheaton], 1
       // je setupemdata
        jmp originalcode//setup

  }
}

static naked void enemy_swap_detour3() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je checkdataoption
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall
        /*cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata*/
        jmp swapsettings
        //jmp setup

        checkdataoption:
        cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata
        jmp originalcode


        swapsettings:
        cmp dword ptr [GameplayStateTracker::gameMode], 3 //for bp skip
        je checkdataoption
        //mov byte ptr [checkForReswap], 1
        mov ecx, [r14+0x10]
        mov [EnemySwapper::currentEnemyId], ecx
        //mov [EnemySwapper::newEnemyId], ecx
        mov qword ptr [curSetDataAddr], r14

        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32

        call qword ptr [custom_swap_setup]

        add rsp, 32
        mov [EnemySwapper::newEnemyId3], eax
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax

        mov ecx, [EnemySwapper::newEnemyId3]
        mov [r14+0x10], ecx
        cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata
        //mov [r8+0x10], ecx//Can't load BP
        jmp originalcode // setup

        setupemdata:
        cmp byte ptr [EnemyDataSettings::shareSettings], 1
        je setdatavalues
        mov ecx, dword ptr [r14+0x10]
        mov  [EnemySwapper::currentEnemyId], ecx
        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32

        call [enemydata_settings_setup]

		add rsp, 32
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax
        cmp byte ptr [defaultEmSetting], 1
        je originalcode

        setdatavalues:
        mov rcx, 0
        mov ecx, [EnemySwapper::odds]
        mov [r14+0x24], ecx
        mov ecx, [EnemySwapper::waitTimeMin]
        mov [r14+0x2C], ecx
        mov ecx, [EnemySwapper::waitTimeMax]
        mov [r14+0x30], ecx
        mov ecx, [EnemySwapper::enemyNum]
        mov dword ptr [r14+0x20], ecx // EnemyNum
        jmp originalcode

        originalcode:
        mov ecx,[r14+0x10]
        test rax, rax
        jmp qword ptr [EnemySwapper::setEnemyDataRet3]

        swapall:
        mov ecx, [EnemySwapper::swapForAll.swapId]
        mov [r14+0x10], ecx
        cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata
        jmp originalcode // setup
  }
}

static naked void enemy_swap_detour5() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall

        cheat:
        mov eax, dword ptr [r15+0x10]
        mov [EnemySwapper::currentEnemyId], eax
        mov [curSetDataAddr], r15
        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32

        call qword ptr [custom_swap_setup]

        add rsp, 32
    // mov dword ptr [r14+0x10], eax
        mov [r15+0x10], eax
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax
        mov eax, [r15+0x10]

        originalcode:
        mov [rsi+0x10],eax
        mov rax,[rdi+0x50]
        jmp qword ptr [EnemySwapper::setEnemyDataRet5]

        swapall:
        mov eax, [EnemySwapper::swapForAll.swapId]
        mov [r15+0x10], eax
        jmp originalcode
  }
}


static naked void enemy_swap_detour6() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode//checkdataoption
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall
        jmp swapsettings

        swapsettings:
        //cmp byte ptr [EnemySwapper::isBp], 1
        //je originalcode
        //mov byte ptr [checkForReswap], 1
        mov ebx, [r14+0x10]
        mov [EnemySwapper::currentEnemyId], ebx
        //mov [EnemySwapper::newEnemyId], ebx
        mov [curSetDataAddr], r14

        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32

        call qword ptr [custom_swap_setup]

        add rsp, 32
        //mov dword ptr [r14+0x10], eax
        mov [EnemySwapper::newEnemyId6], eax
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax
        mov ebx, [EnemySwapper::newEnemyId6]
        mov [r14+0x10], ebx
        mov rbx, 0
        /*cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata*/
        jmp originalcode // setup

        originalcode:
        mov r14d, [r14+0x10]
        mov [rsp+0x40], r15
        jmp [EnemySwapper::setEnemyDataRet6]

        swapall:
        mov ebx, dword ptr [EnemySwapper::swapForAll.swapId]
        mov [r14+0x10], ebx
        mov ebx, 0
        /*cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata*/
        jmp originalcode//setup
  }
}

void EnemySwapper::clear_swap_data_asm() {
  if (EnemySwapper::swapDataAddrs.size() != 0)
    EnemySwapper::swapDataAddrs.clear();
}

void EnemySwapper::set_swapper_setting(int emListIndx, int swapToIndx) {
  selectedToSwap[emListIndx] = swapToIndx;
  swapSettings[emListIndx].set_current_id(swapToIndx);
  swapSettings[emListIndx].set_swap_id(selectedToSwap[swapToIndx]);
}

void EnemySwapper::set_swapper_settings(std::array<int ,enemyListCount> &swapToIndx)
{
  for (int i = 0; i < swapToIndx.size(); i++)
    set_swapper_setting(i, swapToIndx[i]);
}


void EnemySwapper::on_config_load(const utility::Config& cfg) {
  isSwapAll          = cfg.get<bool>("EnemySwapper.isSwapAll").value_or(false);
  selectedForAllSwap = cfg.get<uint32_t>("SwapAllEnemiesToID").value_or(0);
  swapForAll.set_swap_id(selectedForAllSwap);
  std::string key;
  uint32_t swapTo = 0;
  for (int i = 0; i < EnemySwapper::_emNames->size(); i++) {
    key = std::string(EnemySwapper::_emNames->operator[](i)) + "_swapTo";
    swapTo = cfg.get<uint32_t>(key).value_or(i);
    //set_swapper_setting(i, swapTo);
    selectedToSwap[i] = swapTo;
    swapSettings[i].set_current_id(i);
    swapSettings[i].set_swap_id(selectedToSwap[i]);
  }
}

void EnemySwapper::on_config_save(utility::Config& cfg) {
  cfg.set<uint32_t>("SwapAllEnemiesToID", selectedForAllSwap);
  cfg.set<bool>("EnemySwapper.isSwapAll", isSwapAll);

  for (int i = 0; i < EnemySwapper::_emNames->size(); i++) {
    cfg.set<uint32_t>(std::string((*_emNames)[i]) + "_swapTo", selectedToSwap[i]);
  }
}

// void EnemySwapper::on_frame() {}

void EnemySwapper::restore_default_settings() {
  for (int i = 0; i < EnemySwapper::enemyListCount; i++)
    set_swapper_setting(i, i);
  EnemySwapper::curMinIndx = minIndx;
  EnemySwapper::curMaxIndx = maxIndx;
  seed       = -1;
}

void print_issues(const char* str) {
    ImGui::TextWrapped(str);
  ImGui::Spacing();
}

void EnemySwapper::on_draw_ui() {
  if (ImGui::CollapsingHeader("Current Issues")){
    print_issues("Killing enemies swapped with a boss in mission can cause BGM issues.");
    print_issues("Only 1 type of Dante's/Vergil's AI can be loaded for mission/BP floor.");
    print_issues("Dante AI fix doesn't work on most of BP stages.");
    print_issues("Bp stages will not end with Dante boss. Same with Griffon and Shadow.");
    print_issues("Some of battle arenas may be softlocked with boss Dante.");
    print_issues("Game may softlock if you skip swapped Dante boss fight with AI fix.");
    print_issues("Enemy Dante doesn't dealloc memory after his death. So swap all enemies to Dante on missions can crash the game. Also Dante's bodies doesn't disappears.");
    print_issues("Nightmare fix can make meteor fast. Too fast.");
    print_issues("Griffon and Shadow bodies with kill fixes doesn't disappears.");
    print_issues("Force killing Griffon and Shadow can broke mission 18.");
    print_issues("Nightmare can teleports to nowhere to heal familiars if they also in a fight.");
    print_issues("Swapping the Qliphoth root boss can softlock mission 1.");
    print_issues("Swapping Dante can softlock mission 20.");
    print_issues("Wrong swap on some BP stages sometimes.");
    print_issues("Some BP stages softlock when swapping enemies to qliphoth tentacles.");
    ImGui::Separator();
  }

  /*if (ImGui::CollapsingHeader("Enemy fixes")) {
      ImGui::Spacing();
      EnemySwapper::draw_em_fixes();
      ImGui::Separator();
      ImGui::Spacing();
  }*/

  ImGui::Checkbox("Swap all enemies", &isSwapAll);
  if (isSwapAll) {
    ImGui::TextWrapped("Swap to:");
    ImGui::SameLine();
    ImGui::Combo("##Swap to", (int*)&selectedForAllSwap, _emNames->data(), _emNames->size(),20);
    swapForAll.set_swap_id(selectedForAllSwap);
  } 
  else {


    /*if (ImGui::Button("Apply")) {
      for (int i = 0; i < _emNames.itemSize(); i++) {
        set_swapper_setting(i, selectedToSwap[i]);
      }
    }*/

    ImGui::Columns(4, NULL, false);
    for (int i = 0; i < EnemySwapper::_emNames->size(); i++) {
        if (i == 32 || i == 33)//Urizens
            continue;

        bool state = (selectedToSwap[i] != i);
        ImVec4 backgroundcolor = state ? ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered) : SELECTABLE_STYLE_ACT;
        if (i == index)
            backgroundcolor = SELECTABLE_STYLE_HVR;
        ImGui::PushStyleColor(ImGuiCol_Header, backgroundcolor);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, SELECTABLE_STYLE_ACT_HVR);

        
        if (state) {
            uniqStr = std::string((*_emNames)[i] + std::string(" ->") + _emNames->operator[](selectedToSwap[i]));
        }
        else {
            uniqStr = (*_emNames)[i];
        }

        if (ImGui::Selectable(uniqStr.c_str(), state, ImGuiSelectableFlags_AllowDoubleClick)) {
            if (ImGui::IsMouseDoubleClicked(0)) {
                selectedToSwap[i] = i;
            }
            else {
                index = i;
            }
        }
        ImGui::PopStyleColor(2);
        ImGui::NextColumn();
    }
    if (ImGui::Button("Restore default settings"))
        restore_default_settings();

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextWrapped((*_emNames)[index]);
    uniqComboStr = "##SwapToCustom";
    ImGui::Combo(uniqComboStr.c_str(), (int*)&selectedToSwap[index], _emNames->data(), _emNames->size(), 20);
    //set_swapper_setting(i, selectedToSwap[i]);
    swapSettings[index].set_current_id(index);
    swapSettings[index].set_swap_id(selectedToSwap[index]);
    ImGui::Spacing();
    ImGui::Separator();

    
    if (ImGui::Button("Randomize enemies", ImVec2(165, 25))) {
        random_em_swap(curMinIndx, curMaxIndx);
    }

    ImGui::Checkbox("Change randomizer settings", &isCustomRandomSettings);
    if (isCustomRandomSettings) {
      ImGui::Checkbox("Use custom seed", &isCustomSeed);
      if (isCustomSeed) {
        ImGui::Text("Seed:");
        if (ImGui::InputInt("##seedInput", &seed))
          seed_rnd_gen(seed);
      } else
        seed = -1;
      ImGui::TextWrapped("Controls the pool of enemies that can be selected from when randomizing");
      ImGui::Columns(2, NULL, false);
      ImGui::TextWrapped("Min enemy index:");
      UI::SliderInt("##minIndxSlider", &curMinIndx, 0, 39, "%d", 1.0F, ImGuiSliderFlags_AlwaysClamp);
      ImGui::NextColumn();
      ImGui::TextWrapped("Max enemy index:");
      UI::SliderInt("##maxIndxSlider", &curMaxIndx, 1, 40, "%d", 1.0F, ImGuiSliderFlags_AlwaysClamp);
      ImGui::Columns(1);
      if (curMinIndx >= curMaxIndx)
        curMinIndx = curMaxIndx - 1;
    } else {
      curMinIndx = minIndx;
      curMaxIndx = maxIndx;
    }

    
  }
  
}

void EnemySwapper::on_draw_debug_ui() {
  //ImGui::TextWrapped("Vector itemSize is: %d", swapDataAddrs.itemSize());
  /*ImGui::TextWrapped("nowFlow: %d", nowFlow);
  ImGui::Spacing();
  ImGui::TextWrapped("prevFlow: %d", prevFlow);
  ImGui::Spacing();*/
}

void EnemySwapper::reserveReswapVector(size_t newSize) {
  reservedForReswap = newSize;
  swapDataAddrs.resize(0);
  swapDataAddrs.reserve(newSize);
}

void EnemySwapper::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

void EnemySwapper::seed_rnd_gen(int seed) {
  if (seed == -1)
    gen.seed(rd());
  else
    gen.seed(seed);
}

void EnemySwapper::random_em_swap(uint32_t min, uint32_t max) {
  std::uniform_int_distribution<> distrib(min, max);
  for (int i = 0; i < 20; i++) {//Rand only for regular enemies
    set_swapper_setting(i, distrib(gen));
  }
}

std::optional<std::string> EnemySwapper::on_initialize() {
  init_check_box_info();
  auto base      = g_framework->get_module().as<HMODULE>(); // note HMODULE

  m_is_enabled = &cheaton;
  m_on_page         = Page_Balance;
  m_full_name_string   = "Enemy Swapper (+)";
  m_author_string      = "V.P.Zadov";
  m_description_string = "Swap enemy spawns. Affects normal spawns & hell judecca summons.";

  auto initAddr1 = m_patterns_cache->find_addr(base, "8B 71 10 48 85 C0 0F 84 43");// "DevilMayCry5.exe"+FE568B //For BP custom swap
  if (!initAddr1) {
    return "Unanable to find EnemySwapper pattern.";
  }

  auto initAddr2 = m_patterns_cache->find_addr(base, "44 8B 41 10 48 8B D7 48 8B CB E8 F8"); // DevilMayCry5.exe+FE57A9 //1st op
  if (!initAddr2) {
    return "Unanable to find EnemySwapper pattern.";
  }

  //auto tempcustomSpawnAddr = g_framework->get_module().as<uintptr_t>() + 0x11C58BC;

  /*auto initAddr4 = g_framework->get_module().as<uintptr_t>() + 0x259BBC9;
  setEnemyData4Jmp = g_framework->get_module().as<uintptr_t>() + 0x259B5D4;*/

   auto initAddr5 = g_framework->get_module().as<uintptr_t>() + 0xCA17A4;

  auto initAddr3 = m_patterns_cache->find_addr(base, "41 8B 4E 10 48 85 C0 74"); // DevilMayCry5.exe+F34255
  if (!initAddr3) {
    return "Unanable to find EnemySwapper.initAddr3 pattern.";
  }

  auto initAddr6 = m_patterns_cache->find_addr(base, "45 8B 76 10 4C 89 7C 24 40"); // DevilMayCry5.exe+BD4691
  if (!initAddr6) {
    return "Unanable to find EnemySwapper.initAddr6 pattern.";
  }

  /*auto initAddr7 = patterns->find_addr(base, "45 8B 76 10 4C 89 7C 24 40"); // 
  if (!initAddr6) {
    return "Unanable to find EnemySwapper.initAddr7 pattern.";
  }*/

  _mod = this;

  //uintptr_t swapIdAddr = g_framework->get_module().as<uintptr_t>() + 0xF34F6A;

  if (!install_hook_absolute(initAddr1.value(), m_enemy_swapper_hook1, &enemy_swap_detour1, &setEnemyDataRet1, 0x6)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr1"; 
  }

  if (!install_hook_absolute(initAddr2.value(), m_enemy_swapper_hook2, &enemy_swap_detour2, &setEnemyDataRet2, 0x7)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr2"; 
  }

  if (!install_hook_absolute(initAddr3.value(), m_enemy_swapper_hook3, &enemy_swap_detour3, &setEnemyDataRet3, 0x7)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.customSpawnAddr"; 
  }

  /*if (!install_hook_absolute(initAddr4, m_enemy_swapper_hook4, &enemy_swap_detour4, &setEnemyDataRet4, 0x8)) {//BadPointer
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr4"; 
  }*/

  if (!install_hook_absolute(initAddr5, m_enemy_swapper_hook5, &enemy_swap_detour5, &setEnemyDataRet5, 0x7)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr5"; 
  }

  if (!install_hook_absolute(initAddr6.value(), m_enemy_swapper_hook6, &enemy_swap_detour6, &setEnemyDataRet6, 0x9)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr6"; 
  }

  seed_rnd_gen(-1);
  //EnemySwapper::swapDataAddrs.reserve(reservedForReswap);
  //swapDataAddrs = new std::vector<uintptr_t>();
    return Mod::on_initialize();
}