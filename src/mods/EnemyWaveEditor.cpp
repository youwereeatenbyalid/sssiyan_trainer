#include "EnemyWaveEditor.hpp"
using namespace WaveEditorMod;

std::mutex EnemyWaveEditor::mt;

bool EnemyWaveEditor::cheaton{false};
bool EnemyWaveEditor::isAllAllocCorrectly {false};
bool EnemyWaveEditor::forceLoadAll{true};
bool EnemyWaveEditor::isPfbLoadRequested{true};
bool EnemyWaveEditor::isBPFixRequested{false};
bool EnemyWaveEditor::isBpAllocation{false};
bool EnemyWaveEditor::isRequestEndBpStage{false};

bool isRtReady = true;

EnemyWaveEditor::Mode EnemyWaveEditor::mode = EnemyWaveEditor::Mode::Mod;

std::atomic_int EnemyWaveEditor::emReaderCounter{-1};
std::atomic_int EnemyWaveEditor::emSetterCounter{0};
int EnemyWaveEditor::emListId = 0;
int EnemyWaveEditor::selectedMimicListItem = 0;
int EnemyWaveEditor::addListInRangeMin = 0;
int EnemyWaveEditor::addListInRangeMax = 0;

uintptr_t EnemyWaveEditor::retJmp{NULL};
uintptr_t EnemyWaveEditor::retJl{NULL};
uintptr_t EnemyWaveEditor::curListAddr{NULL};
uintptr_t EnemyWaveEditor::prefabLoadJmp{NULL};
uintptr_t EnemyWaveEditor::bpRetJmp{NULL};
uintptr_t EnemyWaveEditor::fadeStaticBase{NULL};

std::vector<GameEmList> EnemyWaveEditor::gameDataList;

std::condition_variable EnemyWaveEditor::cv;

MimicMngObjManager EnemyWaveEditor::mimObjManager{};

EnemyWaveEditor::SetupEmMode EnemyWaveEditor::setupEmMode = EnemyWaveEditor::SetupEmMode::First;

int selectedMimObj = 0;
int selectedAllocObj = -1;
int newLoadId = 0;

void EnemyWaveEditor::set_em_data_asm(std::shared_ptr<HandleMimicObj> &curHandleObj, uintptr_t lstAddr)
{
    curHandleObj->mimicList.set_game_list_data(lstAddr);
    //uintptr_t obj = (uintptr_t)curHandleObj->mimicList.get_mimic_obj();
    uintptr_t temp = *(uintptr_t*)(curHandleObj->mimicList.get_game_mng_obj_ptr());
    uintptr_t emItem;
    int count = curHandleObj->mimicList.get_orig_list_count() - 1;
    switch (setupEmMode)
    {
        case SetupEmMode::First:
        {
            emItem = *(uintptr_t*)(curHandleObj->mimicList.get_game_mng_obj_ptr() + 0x20);
            break;
        }
        case SetupEmMode::Last:
        {
            emItem = *(uintptr_t*)(curHandleObj->mimicList.get_game_mng_obj_ptr() + 0x20 + 0x8 * count);
            break;
        }
        default:
        {
            emItem = *(uintptr_t*)(curHandleObj->mimicList.get_game_mng_obj_ptr() + 0x20);
            break;
        }
    }
    for (int i = 0; i < curHandleObj->mimicList.get_allocated_count(); i++)
    {
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<uintptr_t>(*(uintptr_t*)(emItem + 0x48), 0x48);//AddParamIndexDataListPtr
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<uintptr_t>(*(uintptr_t*)(emItem + 0x58), 0x58);//eventDataPtr
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<uintptr_t>(*(uintptr_t*)(emItem + 0x60), 0x60);//posDataListPtr
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<uintptr_t>(*(uintptr_t*)(emItem + 0x68), 0x68);//presetNoPtr
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<uintptr_t>(*(uintptr_t*)(emItem + 0x70), 0x70);//hpRatePtr
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<uintptr_t>(*(uintptr_t*)(emItem + 0x78), 0x78);//attackRatePtr
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<uintptr_t>(*(uintptr_t*)(emItem + 0x18), 0x18);//commentPtr
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<int>(*(int*)(emItem + 0x34), 0x34);//greenOrbAddType
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<int>(*(int*)(emItem + 0x38), 0x38);//whiteOrbAddType
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<int>(*(int*)(emItem + 0x3C), 0x3C);//addParamIndex
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<int>(*(int*)(emItem + 0x54), 0x54);//playerLimit
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<bool>(*(bool*)(emItem + 0x40), 0x40);//isAddParamSetPlayer
        curHandleObj->mimicList.get_allocated_base_data(i)->set_to_allocated_data<bool>(*(bool*)(emItem + 0x50), 0x50);//isNetworkPlayerLimit
    }
    //memcpy(curHandleObj->mimicList.get_mimic_obj(), (void*)temp, 0x18);
    curHandleObj->mimicList.swap_original_list_data();
    curHandleObj->isSwapped = true;
}

void EnemyWaveEditor::edit_gamelists_asm(uintptr_t lstAddr)
{
    //std::unique_lock<std::mutex> lck(mt);
    if (mimObjManager.is_bind_mode() && !isBpAllocation && EnemySwapper::gameMode != 3)
    {
        auto curEmData = get_game_emdata_asm(lstAddr, false);
        for (int i = 0; i < mimObjManager.count(); i++)
        {
            auto pMimObj = mimObjManager.get_mimic_list_ptr(i);
            if(pMimObj->isSwapped)
                continue;
            if (pMimObj->bindedEmData != nullptr)
            {
                if (*(pMimObj->bindedEmData) == curEmData)
                {
                    set_em_data_asm(pMimObj, lstAddr);
                    break;
                }
            }
        }
    }
    else
    {
        auto curHandleObj = mimObjManager.get_mimic_list_ptr(emSetterCounter, true);
        emSetterCounter.fetch_add(1);
        if (curHandleObj == nullptr)
            return;
        set_em_data_asm(curHandleObj, lstAddr);
    }
}

GameEmList WaveEditorMod::EnemyWaveEditor::get_game_emdata_asm(uintptr_t lstAddr, bool incReaderCounter = true)
{
    GameEmList emList;
    if(incReaderCounter)
        emReaderCounter.fetch_add(1);
    emList.listAddr = lstAddr;
    if(incReaderCounter)
        emList.loadId = emReaderCounter;
    else
        emList.loadId = -1;
    int count = *(int*)(lstAddr + 0x18);
    emList.count = count;
    uintptr_t addr = 0;
    uintptr_t emDataAddr = 0;
    EmData data;
    addr = *(uintptr_t*)(lstAddr + 0x10);//mngObj
    for (int i = 0; i < count; i++)
    {
        emDataAddr = addr + 0x20 + i * 0x8;
        emDataAddr = *(uintptr_t*)emDataAddr;
        data.emId = *(int*)(emDataAddr + 0x10);
        if (data.emId == 56)//Friendly Dante of mission 21
            data.emId--;
        if (data.emId == 55)//seems like they use smth else somewhere else to set actual enemy Dante num
            data.num = 1;
        else
            data.num = *(int*)(emDataAddr + 0x20);
        emList.emDataInfo.push_back(data);
    }
    return emList;
}

void EnemyWaveEditor::read_em_data_asm(uintptr_t lstAddr)
{
    auto emList = get_game_emdata_asm(lstAddr);
    gameDataList.push_back(emList);
}

void EnemyWaveEditor::handle_emlist_asm(uintptr_t lstAddr)
{
    switch (mode)
    {
        case Mode::Mod:
        {
            //if(!EnemySwapper::gamemode != 3)
            if(mimObjManager.is_all_allocated())
                edit_gamelists_asm(lstAddr);
            break;
        }
        case Mode::ViewUserData:
        {
            //if (EnemySwapper::gamemode != 3)
                edit_gamelists_asm(lstAddr);
            break;
        }
        case EnemyWaveEditor::Mode::ReadGameData:
        {
            std::unique_lock<std::mutex> lg(mt);
            read_em_data_asm(lstAddr);
            break;
        }
        default:
            break;
    }
}

uintptr_t rdx_reg;//for bind mode

static naked void emlist_detour()
{
    __asm {
        cmp dword ptr [rax + 0x18], 01
        jl ret_jl
        cmp byte ptr [EnemyWaveEditor::cheaton], 01
        je cheat

        originalcode:
        cmp dword ptr [rax+0x18], 01//This was here not just for fun, game acually has empty list, crash if try to swap it;
        jl ret_jl
        jmp qword ptr [EnemyWaveEditor::retJmp]

        cheat:
        //mov qword ptr [rdx_reg], rdx
        push rax
		push rcx
		push rdx
        push rsp
		push r8
		push r9
		push r10
		push r11
        mov rcx, rax//[EnemyWaveEditor::curListAddr]
        sub rsp, 32
        call qword ptr [EnemyWaveEditor::handle_emlist_asm]
        add rsp, 32
        pop r11
		pop r10
		pop r9
		pop r8
        pop rsp
		pop rdx
		pop rcx
		pop rax
        //mov rdx, qword ptr [rdx_reg]
        jmp originalcode

        ret_jl:
        jmp qword ptr [EnemyWaveEditor::retJl]
    }
}

void EnemyWaveEditor::load_prefabs_asm(uintptr_t prefabManagerAddr)
{
    //isPfbLoadRequested = false;
    if(EnemySwapper::gameMode == 3)//Crash after exit from BP stage
        return;
    if(!mimObjManager.is_all_allocated())
        return;
    if(mimObjManager.get_emids()->empty())
        return;
    uintptr_t prefabList = *(uintptr_t*)(prefabManagerAddr+0x10);
    uintptr_t mngObj = *(uintptr_t*)(prefabList + 0x10);
    int lstCount = *(int*)(prefabList+0x18);
    int curId = 0;
    uintptr_t prefabInfo = 0;
    uintptr_t prefabDataInfo = 0;
    for (const auto &id : *mimObjManager.get_emids())
    {
        for (int i = 0; i < lstCount - 1; i++)
        {
            prefabInfo = *(uintptr_t*)(mngObj + 0x20 + i * 8);
            prefabDataInfo = *(uintptr_t*)(prefabInfo + 0x10);
            curId = *(int*)(prefabDataInfo + 0x10);
            if (curId == id.first)
            {
                if (*(int*)(prefabInfo + 0x40) == 0 && *(uintptr_t*)(prefabInfo + 0x18) == NULL)
                {
                    *(int*)(prefabInfo + 0x30) += id.second;//LoadRequestCount;
                    //*(int*)(prefabInfo + 0x34) = 0;//UnloadRequestCount;
                    *(uintptr_t*)(prefabInfo + 0x18) = *(uintptr_t*)(prefabInfo + 0x20);
                    *(int*)(prefabInfo + 0x40) = 1;//LoadStep 1 - loading;
                }
            }
        }
    }
}

static naked void load_enemy_detour()
{
    __asm {
        cmp byte ptr [EnemyWaveEditor::cheaton], 0
        je originalcode
        cmp byte ptr [EnemyWaveEditor::isPfbLoadRequested], 0
        je originalcode

        cheat:
        push rax//
        push rcx//
        push rdx//
		push r8//
		push r9//
		push r10//
		push r11//
        mov rcx, rdx
        sub rsp, 32
        call qword ptr [EnemyWaveEditor::load_prefabs_asm]
        add rsp, 32
        pop r11//
		pop r10//
	    pop r9//
		pop r8//
        pop rdx//
		pop rcx//
		pop rax//

        originalcode:
        mov [rsp+0x40], rbx
        jmp qword ptr [EnemyWaveEditor::prefabLoadJmp]
    }
}

void EnemyWaveEditor::restore_list_data_asm() 
{ 
    mimObjManager.restore_all_data();
    emSetterCounter = 0;
}

static naked void bp_forceload_detour()
{
    __asm {
        cmp byte ptr [EnemyWaveEditor::cheaton], 0
        je originalcode
        cmp byte ptr [EnemyWaveEditor::isBPFixRequested], 0
        je originalcode

        cheat:
        cmp byte ptr [EnemyWaveEditor::isRequestEndBpStage], 1
        je endbpstage
        cmp byte ptr [rcx+0x60], 0x15 //21
        jne originalcode
        //mov byte ptr [EnemyWaveEditor::isBPFixRequested], 0
        mov dword ptr [rcx+0x60], 0x16 //22
        //----------FadeManager.RequestType-------------//
        mov rax, [EnemyWaveEditor::fadeStaticBase]
        mov rax, [rax]
        mov rax, [rax+0x158]
        mov rax, [rax+0x328]
        mov rax, [rax+0x60]
        mov rax, [rax+0x20]
        mov rax, [rax+0x40]
        mov dword ptr [rax+0x170], 0x1
        //----------FadeManager.RequestType-------------//

        originalcode:
        mov eax, [rcx+0x60]
        mov dword ptr [EnemyWaveEditor::bpFlowId], eax
        sub eax, 0x16
        jmp qword ptr [EnemyWaveEditor::bpRetJmp]

        endbpstage:
        mov byte ptr [EnemyWaveEditor::isRequestEndBpStage], 0
        mov dword ptr [rcx+0x60], 0x1E //30
        jmp originalcode
    }
}

static naked void boss_dante_crash_detour()//Ffs this shit happens only on some missions with some(!) enemies when trying to add Dante at the end of the list
{
    __asm {
        cmp byte ptr [EnemyWaveEditor::cheaton], 1
        je cheat

        originalcode:
        mov [r14+0x08],r15d
        mov [r14 + 0x00000098], dl
        jmp qword ptr [EnemyWaveEditor::bossDanteCrashRet]

        cheat:
        cmp r14, 0
        je skip
        jmp originalcode

        skip:
        jmp qword ptr[EnemyWaveEditor::bossDanteCrashSkip]
    }
}

std::optional<std::string> EnemyWaveEditor::on_initialize() {
  init_check_box_info();
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  m_is_enabled        = &cheaton;
  m_on_page           = Page_Balance;
  m_full_name_string = "Enemy Wave Editor (+)";
  m_author_string    = "VPZadov";
  m_description_string = "Actually enemy list swapper for now. Swap game's enemies lists with yours own. Uses spawn animation and position from original enemy list.";

  fadeStaticBase = g_framework->get_module().as<uintptr_t>() + 0x7E836F8;
  auto emDataLstAddr = m_patterns_cache->find_addr(base, "83 78 18 01 0F 8C 66 02 00 00");// DevilMayCry5.exe+FE5583
  if (!emDataLstAddr) {
    return "Unanable to find emDataLstAddr pattern.";
  }

  auto emPrefabLoad = m_patterns_cache->find_addr(base, "48 89 5C 24 40 E8 3B");// DevilMayCry5.exe+F34F8B
  if (!emPrefabLoad)
  {
      return "Unanable to find emPrefabLoad pattern.";
  }

  auto bpFlowIdAddr = m_patterns_cache->find_addr(base, "19 8B 41 60 83 E8 16");// DevilMayCry5.exe+1547B29
  if (!bpFlowIdAddr)
  {
      return "Unanable to find bpFlowIdAddr pattern.";
  }

  auto bossDanteCrashAddr = m_patterns_cache->find_addr(base, "45 89 7E 08 41 88 96 98 00 00 00");// DevilMayCry5.exe+25BBF28
  if (!bossDanteCrashAddr)
  {
      return "Unanable to find bossDanteCrashAddr pattern.";
  }

  retJl = emDataLstAddr.value() + 0x270;
  bossDanteCrashSkip = bossDanteCrashAddr.value() + 0x3E;

  if (!install_hook_absolute(emDataLstAddr.value(), m_emwave_hook, &emlist_detour, &retJmp, 0xA)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemyWaveEditor.emDataLstAddr"; 
  }

  if (!install_hook_absolute(emPrefabLoad.value(), m_loadall_hook, &load_enemy_detour, &prefabLoadJmp, 0x5)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemyWaveEditor.emDataLstAddr"; 
  }

  if (!install_hook_absolute(bpFlowIdAddr.value() + 0x1, m_bploadflow_hook, &bp_forceload_detour, &bpRetJmp, 0x6))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemyWaveEditor.bpFlowId";
  }

  if (!install_hook_absolute(bossDanteCrashAddr.value(), m_bossdante_crash_hook, &boss_dante_crash_detour, &bossDanteCrashRet, 0xB))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemyWaveEditor.bossDanteCrash";
  }

  mode = Mode::Mod;
  curCustomEmData.set_default();
  return Mod::on_initialize();
}

void EnemyWaveEditor::on_config_load(const utility::Config& cfg) 
{
    isPfbLoadRequested = cfg.get<bool>("EnemyWaveEditor.isPfbLoadRequested").value_or(true);
    isBPFixRequested = cfg.get<bool>("EnemyWaveEditor.isBPFixRequested").value_or(false);
    jsonEmDataPath = cfg.get("EnemyWaveEditor.jsonEmDataPath").value_or(std::filesystem::current_path().string() + "\\natives\\x64\\EmListEditor\\" + "EmData.json");
    setupEmMode = (SetupEmMode)(cfg.get<int>("EnemyWaveEditor.setupEmMode").value_or(0));
    cheaton = false;
}

void EnemyWaveEditor::on_config_save(utility::Config& cfg) 
{
    cfg.set<bool>("EnemyWaveEditor.isBPFixRequested", isBPFixRequested);
    cfg.set<int>("EnemyWaveEditor.setupEmMode", setupEmMode);
}

void EnemyWaveEditor::on_frame() 
{
    //if (cheaton)
    //{
    //    if (EnemySwapper::nowFlow == 0x17) //23
    //    {
    //        mimObjManager.restore_all_data();
    //        mimObjManager.dealloc_all();
    //        mimObjManager.remove_all_binds();
    //        emSetterCounter = 0;
    //        cheaton = false;
    //    }
    //}
}

void EnemyWaveEditor::on_draw_ui() {
  ImGui::Combo("Mode", (int*)&mode, modeNames.data(), modeNames.size());
  switch (mode) {
  case EnemyWaveEditor::Mod: 
  {
    ImGui::ShowHelpMarker("Create custom enemy lists for swap its with originals game's lists.");
    ImGui::Separator();
    if (ImGui::CollapsingHeader("How-to-use & Current issues"))
    {
        ImGui::BulletText("For missions");
        print_spacing("Get original enemy data of a mission: select \"Read game enemy data\" and start mission. Wait when info appears in trainer's window;");
        print_spacing("Setup custom enemy data for waves that you want to change;");
        print_spacing("Game can change an order of loading data for same mission. If you didn't get game data for mission, mod will sort custom data in ascending order by load id and swap data in that order. "
        "Otherwise your custom data will be \"bind\" to original game's enemy list by load id (wrong swap still can orrurs if mission have same enemy lists);");
        print_spacing("After you finish setup enemy data, press \"Allocate all custom enemy data\" button;");
        //print_spacing("(Optional) If you want to swap game's data manually during the loading process, use \"Run-time edit\" mode;");
        print_spacing("(Re)Load mission;");
        print_spacing("Dealloc memory when you want to change wawes data or disable mod.");
        ImGui::Separator();
        ImGui::BulletText("For BP (correctly works only on \"warm up\" mode)");
        print_spacing("Enable \"Bp alloc mode\", create 1 enemy list and setup set up it;");
        print_spacing("After you finish setup enemy data, press \"Allocate all custom enemy data\" button;");
        print_spacing("Select stage what you want. Some stages can't be loaded without \"bp load fix\";");
        ImGui::Separator();
        ImGui::BulletText("Current issues");
        //print_spacing("Some missions, like m13, have a \"puppet waves\" (last nobody encounter for ex). You need to kill all enemies before going to battle arena, or enemies will not load.");
        print_spacing("Spawning a few different bosses at once can crash the game.");
        print_spacing("In run-time mode some times game thread still freezed after pressing \"Skip all next\" button. Select run-time edit mode and press this button again.");
        print_spacing("M18: Shadow/Griffon can't be swapped correctly (red seal will not disappear). If you want to add enemies to Griffon list, add all familiars to 0 list and enemies that u want to have in Griffon fight. "
        "Also add them to Griffon list itself.");
        print_spacing("Some waves (like m2, 3 caina 2 death scrissors) must have > 1 enemies or read seal will not disappear after finish battle.");
        print_spacing("Restoring original data during fight can crash the game if not all enemies from list was spawned.");
        print_spacing("Bosses can block spawn if them placed not in the end of the list in BP/some missions;");
        print_spacing("Urizen 2 on m8 can not be swapped;");
        ImGui::TextWrapped("On Vergil mode m19 shares data with m20, so after finish m19 all enemy data will apply to m20 and game will be softlocked, if boss Dante is absent. Use coop trainer's \"Boss Swap\" feature "
        "to prevent that. And m19/20 always loads two waves: 1 with boss Vergil, 1 with boss Dante (at least if you have Vergil DLC).");
        ImGui::Separator();
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextWrapped("Choose an enemy from original game's list, whoose settings will be used for spawn position and animation for custom lists:");
    ImGui::ShowHelpMarker("Each enemy has a list with settings of spawn pos & spawn animation. You can choose from what enemy from original list mod should take settings. Desync may happend in coop if settings not shared between players.");
    ImGui::RadioButton("First", (int*)&setupEmMode, 0); ImGui::SameLine();
    ImGui::Spacing(); ImGui::SameLine();
    ImGui::RadioButton("Last", (int*)&setupEmMode, 1);
    ImGui::Separator();
    ImGui::Checkbox("BP load fix", &isBPFixRequested);
    ImGui::ShowHelpMarker("If you have blackscreen after load bp stage, but BGM started playing and enemies started action, try this one. May not help with some stages and some enemies. "
        "After finishing stage with this bug, you should quit from it and re-enter from warm up menu, otherwise you will get blackscreen again even with fix :(");
    if (isBPFixRequested)
    {
        if (ImGui::Button("Show end-stage menu"))
        {
            isRequestEndBpStage = true;
        }
        ImGui::ShowHelpMarker("Force game to end the stage and show menu with retry/quit options. Use this if you retried stage with black screen bug. Don't forget to restore list data.");
        if (isRequestEndBpStage)
            ImGui::TextWrapped("End stage requested...");
    }
    ImGui::Checkbox("Bp alloc mode", &isBpAllocation);
    ImGui::ShowHelpMarker("Select this before allocate data if want to go to BP. Creates 107 lists and copy data from first list to them.");

    ImGui::Separator();
    draw_mimic_list_ui();
    //ImGui::TextWrapped("Unique ids count: %d", mimObjManager.get_emids()->size());
    //ImGui::TextWrapped("Is bind mode: %d", mimObjManager.is_bind_mode());
    break;
  }
  case EnemyWaveEditor::ViewUserData: 
  {
    ImGui::ShowHelpMarker("View all custom enemy lists data.");
    ImGui::Separator();
    if (!isBpAllocation)
    {
        ImGui::RadioButton("List all", (int*)&viewUserDataState, 0); ImGui::SameLine();
        ImGui::Spacing(); ImGui::SameLine();
        ImGui::RadioButton("By selecting specific list", (int*)&viewUserDataState, 1);
        if (viewUserDataState == All)
        {
            for (int i = 0; i < mimObjManager.count(); i++)
            {
                print_mimiclist_items(i);
                ImGui::Separator();
                ImGui::Separator();
            }
        }
        else
        {
            if(mimObjManager.count() == 0)
                return;
            draw_emlist_combo();
            print_mimiclist_items(selectedMimicListItem);
        }
        
    }
    else if (mimObjManager.count() != 0)
        print_mimiclist_items(0);
    
    break;
  }
  case EnemyWaveEditor::ReadGameData: {
    ImGui::ShowHelpMarker("Check all original enemy lists data after load mission.");
    ImGui::Separator();
    emlist_btn();
    ImGui::Separator();
    print_emlist_data();
    break;
  }

  case EnemyWaveEditor::Serialization:
  {
      ImGui::ShowHelpMarker("Load enemy data from .json file.");
      ImGui::Separator();
      ImGui::TextWrapped(std::string("Place save-file here: " + jsonEmDataPath).c_str());
      ImGui::ShowHelpMarker("You can change path to save file (and it's name) in trainer's config file. Open \"DMC5_fw_config.txt\" and add line without quotes: \"EnemyWaveEditor.jsonEmDataPath = <YourPathToFileWithFileName.json>\"");
      if (ImGui::Button("Read enemy data settings"))
      {
          prflManager.read_data(jsonEmDataPath.c_str());
      }
      if (prflManager.is_data_read())
      {
          bool isSelected = false;
          if (ImGui::BeginCombo("Seleceted profile", prflManager.get_profile_name(selectedProfile)->c_str()))
          {
              for (int i = 0; i < prflManager.count(); i++)
              {
                  isSelected = (selectedProfile == i);
                  if (ImGui::Selectable(prflManager.get_profile_name(i)->c_str(), isSelected))
                      selectedProfile = i;
                  if (isSelected)
                      ImGui::SetItemDefaultFocus();
              }
              ImGui::EndCombo();
          }
          ImGui::Spacing();
          if (ImGui::Button("Load profile##EmWaveEditor.import_data()"))
          {
              prflManager.import_data(selectedProfile);
              mode = Mode::Mod;
          }
          ImGui::ShowHelpMarker("This will remove your current custom data and load data from selected profile. Memory shouldn't be allocated before this stage.");
      }
      break;
  }

  default:
    break;
  }
}

void EnemyWaveEditor::print_mimiclist_items(int i)
{
    ImGui::TextWrapped("List load id: %d", mimObjManager.get_mimic_list_ptr(i)->loadId);
    ImGui::Spacing();
    for (int j = 0; j < mimObjManager.get_mimic_list_ptr(i)->mimicList.get_count(); j++)
    {
        ImGui::TextWrapped("Enemy index in list: %d", j);
        ImGui::TextWrapped("Enemy: %s", EnemySwapper::emNames[get_em_name_indx(mimObjManager.get_mimic_list_ptr(i)->mimicList.get_allocated_base_data(j)->get_data()->emId)]);
        ImGui::TextWrapped("Enemy num: %d", mimObjManager.get_mimic_list_ptr(i)->mimicList.get_allocated_base_data(j)->get_data()->num);
        ImGui::TextWrapped("Odds: %.1f", mimObjManager.get_mimic_list_ptr(i)->mimicList.get_allocated_base_data(j)->get_data()->odds);
        ImGui::TextWrapped("Use boss hp bar and boss camera: %d", mimObjManager.get_mimic_list_ptr(i)->mimicList.get_allocated_base_data(j)->get_data()->isBoss);
        ImGui::TextWrapped("Enemy can't be killed: %d", mimObjManager.get_mimic_list_ptr(i)->mimicList.get_allocated_base_data(j)->get_data()->isNoDie);
        ImGui::TextWrapped("Wait time min: %.1f", mimObjManager.get_mimic_list_ptr(i)->mimicList.get_allocated_base_data(j)->get_data()->waitTimeMin);
        ImGui::TextWrapped("Wait time max: %.1f", mimObjManager.get_mimic_list_ptr(i)->mimicList.get_allocated_base_data(j)->get_data()->waitTimeMax);
        ImGui::TextWrapped("Don't set orbs: %d", mimObjManager.get_mimic_list_ptr(i)->mimicList.get_allocated_base_data(j)->get_data()->isDontSetOrb);
        ImGui::TextWrapped("Is near player?: %d", mimObjManager.get_mimic_list_ptr(i)->mimicList.get_allocated_base_data(j)->get_data()->isNearPlayer);
        ImGui::Separator();
    }
}

void EnemyWaveEditor::print_emdata_input(SetEmData &data) {
  ImGui::TextWrapped("Enemy:");
  ImGui::Combo("##SelectEmCombmoBox", &data.selectedItem, EnemySwapper::emNames.data(), EnemySwapper::emNames.size(), 20);
  ImGui::ShowHelpMarker("All \"Enemy Swapper\" enemies and spawn pos change fixes works with this mod even when swapper disabled. ");
  data.emId = get_em_id(data.selectedItem);
  ImGui::Spacing();
  ImGui::TextWrapped("Enemy num:");
  ImGui::InputInt("##EmNumInput", &data.num);
  ImGui::Spacing();
  ImGui::TextWrapped("Odds to appear (%%):");
  UI::SliderFloat("##OddsSlider", &data.odds, 0.0f, 100.f, "%.1f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
  ImGui::Spacing();
  ImGui::Checkbox("Use boss hp bar and boss camera", &data.isBoss);
  ImGui::Spacing();
  ImGui::Checkbox("Enemy can't be killed", &data.isNoDie);
  ImGui::Spacing();
  //ImGui::TextWrapped("Minimum spawn time & Maximum spawn times adjust in-engine settings controlling the delay between enemy spawns.");
  ImGui::TextWrapped("Wait time min:");
  ImGui::ShowHelpMarker("Minimum spawn time & Maximum spawn times adjust in-engine settings controlling the delay between enemy spawns.");
  ImGui::InputFloat("##WaitTimeMin", &data.waitTimeMin, 0.1f, 0.5f, "%.1f");
  if(data.waitTimeMin < 0)
      data.waitTimeMin = 0;
  ImGui::TextWrapped("Wait time max:");
  ImGui::InputFloat("##WaitTimeMax", &data.waitTimeMax, 0.1f, 0.5f, "%.1f");
  if (data.waitTimeMax < 0)
      data.waitTimeMax = 0;
  ImGui::Checkbox("Don't set orbs", &data.isDontSetOrb);
  ImGui::Spacing();
  ImGui::Checkbox("Is near player(?)", &data.isNearPlayer);
  ImGui::ShowHelpMarker("Sometimes it's work and place enemies near player and remove bloody portal spawn animation if it exist on current list. But sometimes it's not lol.");
  ImGui::Spacing();
  if(ImGui::Button("Set default"))
      data.set_default();
}

void EnemyWaveEditor::on_draw_debug_ui() {}

void EnemyWaveEditor::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

int EnemyWaveEditor::get_em_id(int selected) {
  if(selected == 39)//Dante go brrr
    return 55;
  if (selected >= 20)
    selected += 3;
  return selected;
}

int EnemyWaveEditor::get_em_name_indx(int emId) { 
  if (emId == 55)
    return 39;
  if (emId >=23)
  return emId - 3;
  return emId;
}

void EnemyWaveEditor::print_emlist_data()
{
    ImGui::TextWrapped("All lists count: %d", gameDataList.size());
    ImGui::Separator();
    for (const auto& item : gameDataList)
    {
        ImGui::TextWrapped("Load id: %d", item.loadId);
        //ImGui::TextWrapped("Enemy list address: %X", item.listAddr);
        ImGui::TextWrapped("Count: %d", item.count);
        ImGui::Spacing();
        for (const auto &data : item.emDataInfo)
        {
            ImGui::TextWrapped("Enemy: %s", EnemySwapper::emNames[get_em_name_indx(data.emId)]);
            ImGui::TextWrapped("Enemy num: %d", data.num);
            ImGui::Spacing();
        }
        ImGui::Separator();
        ImGui::Separator();
    }
}

void EnemyWaveEditor::emlist_btn()
{
    if (!gameDataList.empty())
    {
        if (ImGui::Button("Clear list and reset counter"))
            clear_emlist();
        ImGui::ShowHelpMarker("Data must be deallocated before that.");
        ImGui::Spacing();
        if (ImGui::Button("Copy to clipboard"))
        {
            std::string all = "";
            for (auto& item : gameDataList)
            {
                all += "Load id: ";
                all += std::string(std::to_string(item.loadId) + "\n");
                //all += std::string("Enemy list address: " + std::to_string(item.listAddr));
                all += "\n\n";
                for (const auto &data : item.emDataInfo)
                {
                    all += std::string("Enemy: " + std::string(EnemySwapper::emNames[get_em_name_indx(data.emId)]));
                    all += "\n";
                    all += std::string("Enemy num: " + std::to_string(data.num));
                    all +="\n\n";
                }
                all += "___________________________________________________\n\n";
                //all += "\n___________________________________________________\n\n";
            }
            to_clipboard(all);
        }
    }
}

void WaveEditorMod::EnemyWaveEditor::clear_emlist()
{
    if (!mimObjManager.is_all_allocated())
    {
        emReaderCounter = -1;
        mimObjManager.remove_all_binds();
        gameDataList.clear();
    }
}

void EnemyWaveEditor::draw_copy_list_data()
{
    ImGui::TextWrapped("Copy and append all of 1 list data to another. If you have \"-1\" in one of input fields after pressing 1 of copy buttons "
    "then you inputted wrong list id.");
    ImGui::InputInt("From list", &fromLst);
    ImGui::InputInt("To list", &toLst);
    if (ImGui::Button("Copy"))//had some crashes with it, need to check it later
    {
        if (fromLst == toLst || mimObjManager.get_mimic_list_ptr(fromLst, true) == nullptr || mimObjManager.get_mimic_list_ptr(toLst, true) == nullptr)
        {
            fromLst = -1;
            toLst = -1;
        }
        else
            mimObjManager.copy_to(fromLst, toLst);
    }
    ImGui::ShowHelpMarker("You need to input list's load id, not it's num on list of all custom lists. And you can't copy data to the same list.");
    if(ImGui::Button("Copy to all"))
    {
        if(mimObjManager.get_mimic_list_ptr(fromLst, true) == nullptr)
            fromLst = -1;
        else
        {
            for (int i = 0; i < mimObjManager.count(); i++)
            {
                if (mimObjManager.get_mimic_list_ptr(i)->loadId == fromLst)
                {
                    for (int j = 0; j < mimObjManager.count(); j++)
                    {
                        if (j == i)
                            continue;
                        mimObjManager.copy_to(i, j, false);
                    }
                    break;
                }
            }
        }
        
    }
    ImGui::ShowHelpMarker("Copy and append from 1 list to all another.");
}

void EnemyWaveEditor::draw_emlist_combo()
{
    bool isSelected = false;
    if (ImGui::BeginCombo("Current enemy list", mimObjManager.get_list_name(selectedMimicListItem)->c_str()))
    {
        for (int i = 0; i < mimObjManager.count(); i++)
        {
            if(mimObjManager.get_mimic_list_ptr(i)->isUsedByRtEdit)
                continue;
            isSelected = (selectedMimicListItem == i);
            if (ImGui::Selectable(mimObjManager.get_list_name(i)->c_str(), isSelected))
                selectedMimicListItem = i;
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

void EnemyWaveEditor::bpmode_data_setup()
{
    clear_emlist();
    int listCount = mimObjManager.count();
    if (listCount > 1)
    {
        for (int i = 1; i < listCount; i++)
        {
            mimObjManager.remove_at(1);
        }
    }
    for (int i = 0; i <= 108; i++)
        mimObjManager.add(i);
    for (int i = 1; i <= 108; i++)
        mimObjManager.copy_to(0, i, false);
}

void WaveEditorMod::EnemyWaveEditor::print_em_colums(int indx)
{
    bool isSelected = false;
    if(ImGui::BeginListBox("##EnemiesDataListBox"))
    {
        for (int row = 0; row < mimObjManager.get_mimic_list_ptr(selectedMimicListItem)->mimicList.get_count(); row++)
        {
            isSelected = (selectedEmDataItem == row);
            ImVec4 backgroundcolor = isSelected ? ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered) : SELECTABLE_STYLE_ACT;
            if (row == selectedEmDataItem)
                backgroundcolor = SELECTABLE_STYLE_HVR;
            ImGui::PushStyleColor(ImGuiCol_Header, backgroundcolor);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, SELECTABLE_STYLE_ACT_HVR);
            auto uniqStr = EnemySwapper::emNames[mimObjManager.get_mimic_list_ptr(selectedMimicListItem)->mimicList.get_allocated_base_data(row)->get_data()->selectedItem] + std::string(" x") + 
                std::to_string(mimObjManager.get_mimic_list_ptr(selectedMimicListItem)->mimicList.get_allocated_base_data(row)->get_data()->num);  //+ std::string("##") + std::to_string(row);
            auto addInfo = add_line_em_info(mimObjManager.get_mimic_list_ptr(selectedMimicListItem)->mimicList.get_allocated_base_data(row)->get_data());
            if (!addInfo.empty())
                uniqStr += ", ";
            uniqStr += addInfo + "##" + std::to_string(row);
            if(ImGui::Selectable(uniqStr.c_str(), isSelected))
                selectedEmDataItem = row;
            ImGui::PopStyleColor(2);
        }
        ImGui::EndListBox();
    }

}

std::string WaveEditorMod::EnemyWaveEditor::add_line_em_info(SetEmData* data)
{
    std::string res = "";
    if(data == nullptr)
        return res;
    if(data->isBoss)
        res += "boss, ";
    if(data->isNoDie)
        res += "no die, ";
    if (data->isNearPlayer)
        res += "near player, ";
    if(data->isDontSetOrb)
        res += "dont set orbs, ";
    std::stringstream stream;
    if (data->odds <= 99.02F)
    {
        stream << std::fixed << std::setprecision(1) << data->odds;
        res += "odds " + stream.str() + ", ";
    }
    stream.str("");
    if (data->waitTimeMin >= 0.01F)
    {
        stream << std::fixed <<std::setprecision(1) << data->waitTimeMin;
        res += "min wait " + stream.str() + ", ";
    }
    stream.str("");
    if (data->waitTimeMax >= 0.01F)
    {
        stream << std::fixed << std::setprecision(1) << data->waitTimeMax;
        res += "max wait " + stream.str() + ", ";
    }
    if (!res.empty())
    {
        res.pop_back();
        res.pop_back();
    }
    return res;
}

void EnemyWaveEditor::draw_mimic_list_ui()
{
    if (!mimObjManager.is_all_allocated())
    {
        ImGui::TextWrapped("Enemy list's load id:");
        ImGui::InputInt("##emListIdinput", &emListId);
        if (emListId < 0)
            emListId = 0;
        if (ImGui::Button("Create enemy list"))
        {
            mimObjManager.add(emListId);
            selectedMimicListItem = mimObjManager.count() - 1;
        }
        ImGui::Spacing();
        ImGui::InputInt("Begin id", &addListInRangeMin);
        ImGui::InputInt("End id", &addListInRangeMax);
        if (ImGui::Button("Create enemy lists in range"))
        {
            for (int i = addListInRangeMin; i <= addListInRangeMax; i++)
            {
                mimObjManager.add(i);
                selectedMimicListItem = mimObjManager.count() - 1;
            }
        }
        ImGui::ShowHelpMarker("Create a few enemy lists with identificators on setted range");
        ImGui::Separator();
        ImGui::Spacing();
        if (mimObjManager.count() != 0)
        {
            if (!mimObjManager.is_all_allocated() && mimObjManager.count() != 0)
            {
                draw_emlist_combo();
                ImGui::Spacing();
                if (!gameDataList.empty())
                {
                    ImGui::Separator();
                    ImGui::InputInt("New load id", &newLoadId);
                    ImGui::ShowHelpMarker("Load id must be >=0. If not all id's is unique, after pressing allocate button this field will be \"-2\".");
                    if (ImGui::Button("Change load id"))
                    {
                        if(newLoadId >= 0)
                            mimObjManager.change_loadid(selectedMimicListItem, newLoadId);
                        else newLoadId = -1;
                    }
                    ImGui::Separator();
                    ImGui::Spacing();
                }

                if (ImGui::Button("Remove selected enemy list"))
                {
                    mimObjManager.remove_at(selectedMimicListItem);
                    if (selectedMimicListItem != 0)
                        selectedMimicListItem--;
                    else
                        selectedMimicListItem = 0;
                    return;
                }
                ImGui::Spacing();
                if (ImGui::Button("Clear all"))
                {
                    selectedMimicListItem = 0;
                    mimObjManager.remove_all();
                    return;
                }
                ImGui::Separator();
                ImGui::TextWrapped("Enemy data editor");
                ImGui::Spacing();

                ImGui::RadioButton("Add new enemy data", &emChangeState, 0); ImGui::SameLine();
                ImGui::Spacing(); ImGui::SameLine();
                ImGui::RadioButton("Edit existed enemy data", &emChangeState, 1);
                if (mimObjManager.get_mimic_list_ptr(selectedMimicListItem)->mimicList.get_count() == 0)
                {
                    emChangeState = 0;
                    selectedEmDataItem = 0;
                }
                ImGui::Separator();
                if(emChangeState == 0  )
                {
                    print_emdata_input(curCustomEmData);
                    ImGui::Spacing();
                    if (ImGui::Button("Add enemy data to enemy list"))
                    {
                        mimObjManager.add_data(new AllocatedEmData(&curCustomEmData, SET_ENEMY_DATA_SIZE), selectedMimicListItem);
                    }
                }
                else
                {
                    print_em_colums(selectedMimicListItem);
                    if (selectedEmDataItem < 0)//if first item was removed
                        selectedEmDataItem = 0;
                    print_emdata_input(*(mimObjManager.get_mimic_list_ptr(selectedMimicListItem)->mimicList.get_allocated_base_data(selectedEmDataItem)->get_data()));
                    ImGui::Spacing();
                    if (ImGui::Button("Remove selected enemy data item from list"))
                    {
                        mimObjManager.get_mimic_list_ptr(selectedMimicListItem)->mimicList.remove(selectedEmDataItem--);
                    }
                }
                ImGui::Separator();
                ImGui::TextWrapped("Current enemy list count = %d;", mimObjManager.get_mimic_list_ptr(selectedMimicListItem)->mimicList.get_count());
                ImGui::Separator();
                
                if(ImGui::CollapsingHeader("Copy data"))
                    draw_copy_list_data();
                ImGui::Separator();
            }
            if (ImGui::Button("Allocate memory for all custom enemy data"))
            {
                if(isBpAllocation)
                    bpmode_data_setup();
                if (!gameDataList.empty())
                {
                    if (mimObjManager.is_all_id_uniq())
                        mimObjManager.alloc_all(&gameDataList);
                    else newLoadId = -2;
                }
                else mimObjManager.alloc_all();
            }
            ImGui::ShowHelpMarker("Allocate memory for custom enemy lists. After that you can't edit list's data. All load ids must be unique before allocation.");
        }
    }
    else
    {
        ImGui::TextWrapped("Memory for all custom data was allocated successfully, edit mode disabled. You can (re)load a mission.");
        ImGui::Spacing();
        if (ImGui::Button("Dealloc all memory"))
        {
            mimObjManager.restore_all_data();
            mimObjManager.dealloc_all();
            emSetterCounter = 0;
        }
        ImGui::ShowHelpMarker("After pressing this all original data also will be restored if this didnt happen already. You will get softlock/crash if continue mission after restoring data "
        "without restarting.");
    }
}

void EnemyWaveEditor::to_clipboard(const std::string& s)
{
    OpenClipboard(0);
    EmptyClipboard();
    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size());
    if (!hg)
    {
        CloseClipboard();
        return;
    }
    memcpy(GlobalLock(hg), s.c_str(), s.size());
    GlobalUnlock(hg);
    SetClipboardData(CF_TEXT, hg);
    CloseClipboard();
    GlobalFree(hg);
}

void EnemyWaveEditor::print_spacing(const char* ch)
{
    ImGui::TextWrapped(ch);
    ImGui::Spacing();
}
