#include "EnemyWaveEditor.hpp"
#include "EnemyDataSettings.hpp"
#include "EnemyFixes.hpp"

using namespace WaveEditorMod;

std::mutex EnemyWaveEditor::mtx;

bool EnemyWaveEditor::cheaton{false};
bool EnemyWaveEditor::isBpWarmUpMode{false};

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

EnemyWaveEditor::SetupEmMode EnemyWaveEditor::setupEmMode = EnemyWaveEditor::SetupEmMode::Last;

int selectedMimObj = 0;
int selectedAllocObj = -1;
int newLoadId = 0;

void EnemyWaveEditor::set_em_data_asm(const std::shared_ptr<MimicListData> &curHandleObj, uintptr_t lstAddr)
{
    if(curHandleObj->isSwapped)
        return;
    curHandleObj->mimicList.set_list_data(lstAddr);
    uintptr_t emItem;
    int count = curHandleObj->mimicList.get_net_list_count() - 1;
    switch (setupEmMode)
    {
        case SetupEmMode::First:
        {
            emItem = *(uintptr_t*)(curHandleObj->mimicList.get_items_ptr() + 0x20);
            break;
        }
        case SetupEmMode::Last:
        {
            emItem = *(uintptr_t*)(curHandleObj->mimicList.get_items_ptr() + 0x20 + 0x8 * count);
            break;
        }
        default:
        {
            emItem = *(uintptr_t*)(curHandleObj->mimicList.get_items_ptr() + 0x20);
            break;
        }
    }
    for (int i = 0; i < curHandleObj->mimicList.get_mimic_count(); i++)
    {
        curHandleObj->mimicList[i]->AddParamIndexDataListPtr = *(uintptr_t*)(emItem + 0x48);//AddParamIndexDataListPtr
        curHandleObj->mimicList[i]->eventDataPtr = *(uintptr_t*)(emItem + 0x58);//eventDataPtr
        curHandleObj->mimicList[i]->posDataListPtr = *(uintptr_t*)(emItem + 0x60);//eventDataPtr
        curHandleObj->mimicList[i]->presetNoPtr = *(uintptr_t*)(emItem + 0x68);//eventDataPtr
        curHandleObj->mimicList[i]->hpRatePtr = *(uintptr_t*)(emItem + 0x70);//eventDataPtr
        curHandleObj->mimicList[i]->attackRatePtr = *(uintptr_t*)(emItem + 0x78);//eventDataPtr
        curHandleObj->mimicList[i]->strComment = *(uintptr_t*)(emItem + 0x18);//commentPtr
        curHandleObj->mimicList[i]->greenOrbAddType = *(int*)(emItem + 0x34);//greenOrbAddType
        curHandleObj->mimicList[i]->whiteOrbAddType = *(int*)(emItem + 0x38);//whiteOrbAddType
        curHandleObj->mimicList[i]->addParamIndex = *(int*)(emItem + 0x3C);//addParamIndex
        curHandleObj->mimicList[i]->playerLimit = *(int*)(emItem + 0x54);//playerLimit
        curHandleObj->mimicList[i]->isAddParamSetPlayer = *(bool*)(emItem + 0x40);//isAddParamSetPlayer
        curHandleObj->mimicList[i]->isNetworkPlayerLimit = *(bool*)(emItem + 0x50);//isNetworkPlayerLimit
    }
    curHandleObj->mimicList.swap();
    curHandleObj->isSwapped = true;
    mimListManager->set_is_any_swapped(true);
}

void EnemyWaveEditor::edit_gamelists_asm(uintptr_t lstAddr)
{
    std::unique_lock<std::mutex> lck(mtx);
    int missionN = -1;

    if (prflManager->is_custom_gamemode())
    {
        auto gmMng = sdk::get_managed_singleton<REManagedObject>("app.GameManager");
        if (gmMng != nullptr)
        {
            missionN = *(int*)((uintptr_t)gmMng + 0x80);
            if (auto gameData = &mimListManager->get_game_data_list(); missionN == 20 && !gameData->empty() && IsEmDataStartedLoading && !(*gameData)[0].emDataInfo.empty())//This shit should be rewrite with MissionSettingsData but meh...
            {
                if (*(bool*)(gmMng + 0x94))
                {
                    missionN == 23;
                    IsEmDataStartedLoading = false;

                }
                else if ((*gameData)[0].emDataInfo[0].emId == 3 || (*gameData)[0].emDataInfo[0].emId == 55 || (*gameData)[0].emDataInfo[0].emId == 42)
                {
                    auto curEmData = get_game_emdata_asm(lstAddr, false, false);
                    switch (curEmData.emDataInfo[0].emId)
                    {
                        case 3:
                        {
                            missionN = 21;
                            if(prflManager->get_last_misison_num() == 21)
                                break;
                            IsEmDataStartedLoading = false;
                            break;
                        }
                        case 42:
                        {
                            missionN = 22;
                            if (prflManager->get_last_misison_num() == 22)
                                break;
                            IsEmDataStartedLoading = false;
                            break;
                        }
                        default:
                        {
                            missionN = prflManager->get_last_misison_num();
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!IsEmDataStartedLoading)
    {
        if (prflManager->is_custom_gamemode())
            prflManager->load_custom_mission_data(missionN);
        mimListManager->recheck_emids();
    }

    if (mimListManager->count() != 0)
    {
        if (mimListManager->is_bind_mode() && !isBpWarmUpMode)
        {
            auto curEmData = get_game_emdata_asm(lstAddr, false);
            for (int i = 0; i < mimListManager->count(); i++)
            {
                auto pMimObj = mimListManager->get_mimic_list_data(i);
                if (pMimObj->isSwapped)
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
            if (isBpWarmUpMode)
            {
                if (!mimListManager->is_any_swapped(false))
                {
                    if (mimListManager->count() > 1)
                    {
                        for (int i = 1; i < mimListManager->count(); i++)
                            mimListManager->remove_at(1);
                    }
                    for (int i = 1; i <= 108; i++)
                        mimListManager->add(i);
                    for (int i = 1; i < mimListManager->count(); i++)
                        mimListManager->copy_to(0, i, false);
                }
            }

            auto curHandleObj = mimListManager->get_mimic_list_data(emSetterCounter, true);
            emSetterCounter.fetch_add(1);
            if (curHandleObj == nullptr)
                return;
            set_em_data_asm(curHandleObj, lstAddr);
        }
    }
    IsEmDataStartedLoading = true;
}

GameEmList WaveEditorMod::EnemyWaveEditor::get_game_emdata_asm(uintptr_t lstAddr, bool incReaderCounter = true, bool autoIdFix)
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
        if (autoIdFix)
        {
            if (data.emId == 56)//Friendly Dante of mission 21
                data.emId--;
        }
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
    mimListManager->add_game_em_data(emList);
}

void EnemyWaveEditor::handle_emlist_asm(uintptr_t lstAddr)
{
    switch (mode)
    {
        case Mode::Mod:
        {
            edit_gamelists_asm(lstAddr);
            break;
        }
        case Mode::ViewUserData:
        {
            edit_gamelists_asm(lstAddr);
            break;
        }
        case EnemyWaveEditor::Mode::ReadGameData:
        {
            std::unique_lock<std::mutex> lg(mtx);
            read_em_data_asm(lstAddr);
            break;
        }
        default:
            break;
    }
}

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
        jmp originalcode

        ret_jl:
        jmp qword ptr [EnemyWaveEditor::retJl]
    }
}

void EnemyWaveEditor::load_prefabs_asm(uintptr_t prefabManagerAddr)
{
    std::unique_lock<std::mutex> lck(prefab_mtx);
    if(mimListManager->count() == 0)
        return;
    if(mimListManager->get_emids()->empty())
        return;
    uintptr_t prefabList = *(uintptr_t*)(prefabManagerAddr+0x10);
    uintptr_t mngObj = *(uintptr_t*)(prefabList + 0x10);
    int lstCount = *(int*)(prefabList+0x18);
    int curId = 0;
    uintptr_t prefabInfo = 0;
    uintptr_t prefabDataInfo = 0;
    for (const auto &id : *mimListManager->get_emids())
    {
        for (int i = 0; i < lstCount - 1; i++)
        {
            prefabInfo = *(uintptr_t*)(mngObj + 0x20 + i * 8);
            prefabDataInfo = *(uintptr_t*)(prefabInfo + 0x10);
            curId = *(int*)(prefabDataInfo + 0x10);
            if (curId == id.first)
            {
                if (*(int*)(prefabInfo + 0x40) == 0 /*&& *(uintptr_t*)(prefabInfo + 0x18) == NULL*/)
                {
                    *(int*)(prefabInfo + 0x30) += id.second + 1;//LoadRequestCount;
                    //*(int*)(prefabInfo + 0x34) = 0;//UnloadRequestCount;
                    *(uintptr_t*)(prefabInfo + 0x18) = *(uintptr_t*)(prefabInfo + 0x20);
                    sdk::call_object_func_easy<void*>((REManagedObject*)prefabManagerAddr, "requestLoad(app.EnemyID, System.Boolean)", curId, false);
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

        cheat:
        push rax//
        push rcx//
        push rdx//
        push rsp
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
        pop rsp
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
    mimListManager->restore_all_data();
    emSetterCounter = 0;
}

static naked void boss_dante_crash_detour()//Ffs this shit happens only on some missions with some(!) enemies when trying to add Dante at the end of the list
{
    __asm {
       /* cmp byte ptr [EnemyWaveEditor::cheaton], 1
        je cheat*/

        cheat:
        cmp r14, 0
        je skip
        jmp originalcode

        originalcode:
        mov [r14+0x08],r15d
        mov [r14 + 0x00000098], dl
        jmp qword ptr [EnemyWaveEditor::bossDanteCrashRet]

        skip:
        jmp qword ptr[EnemyWaveEditor::bossDanteCrashSkip]
    }
}

static naked void bp_fadein_detour()
{
    __asm {
        cmp byte ptr [EnemyWaveEditor::cheaton], 1
        je cheat
        cmp byte ptr [EnemyDataSettings::cheaton], 1
        je cheat

        originalcode:
        cmp ecx, [rax+0x74]
        jl jlret

        cheat:
        jmp qword ptr [EnemyWaveEditor::updateBpFadeInRet]

        jlret:
        jmp qword ptr [EnemyWaveEditor::updateBpFadeInJlRet]
    }
}

std::optional<std::string> EnemyWaveEditor::on_initialize() {
  init_check_box_info();
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  m_is_enabled        = &cheaton;
  m_on_page           = Page_Encounters;
  m_full_name_string = "Enemy Wave Editor (+)";
  m_author_string    = "V.P.Zadov";
  m_description_string = "Replace a mission's set of encounters with your own. Uses the spawn animation and position from the original mission.";

  auto emDataLstAddr = m_patterns_cache->find_addr(base, "83 78 18 01 0F 8C 66 02 00 00");// DevilMayCry5.exe+FE5583
  if (!emDataLstAddr) {
    return "Unanable to find emDataLstAddr pattern.";
  }

  auto emPrefabLoad = m_patterns_cache->find_addr(base, "48 89 5C 24 40 E8 3B");// DevilMayCry5.exe+F34F8B
  if (!emPrefabLoad)
  {
      return "Unanable to find emPrefabLoad pattern.";
  }

  auto bossDanteCrashAddr = m_patterns_cache->find_addr(base, "45 89 7E 08 41 88 96 98 00 00 00");// DevilMayCry5.exe+25BBF28
  if (!bossDanteCrashAddr)
  {
      return "Unanable to find bossDanteCrashAddr pattern.";
  }

  auto bpFadeInFix = m_patterns_cache->find_addr(base, "3B 48 74 0F 8C 9F 01 00 00");// DevilMayCry5.exe+36ED34
  if (!bpFadeInFix)
  {
      return "Unanable to find bpFadeInFix pattern.";
  }

  retJl = emDataLstAddr.value() + 0x270;
  bossDanteCrashSkip = bossDanteCrashAddr.value() + 0x3E;
  updateBpFadeInJlRet = bpFadeInFix.value() + 0x1A8;

  if (!install_hook_absolute(emDataLstAddr.value(), m_emwave_hook, &emlist_detour, &retJmp, 0xA)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemyWaveEditor.emDataLst"; 
  }

  if (!install_hook_absolute(emPrefabLoad.value(), m_loadall_hook, &load_enemy_detour, &prefabLoadJmp, 0x5)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemyWaveEditor.emPrefabLoad"; 
  }

  if (!install_hook_absolute(bossDanteCrashAddr.value(), m_bossdante_crash_hook, &boss_dante_crash_detour, &bossDanteCrashRet, 0xB))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemyWaveEditor.bossDanteCrash";
  }

  if (!install_hook_absolute(bpFadeInFix.value(), m_bp_fadein_hook, &bp_fadein_detour, &updateBpFadeInRet, 0x9))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemyWaveEditor.bpFadeIn";
  }
  _txtHelperSize = ImGui::CalcTextSize(_txtHelper).x;

  mimListManager = std::make_unique<MimicListManager>();
  mode = Mode::Mod;
  curCustomEmData.set_default();
  prflManager = std::make_unique<ProfileManager>();
  rndWaveGen = std::make_unique<WaveRandomGenerator>(this);
  return Mod::on_initialize();
}

void EnemyWaveEditor::on_config_load(const utility::Config& cfg) 
{
    cheaton = false;
    prflManager->on_config_load(cfg);
    setupEmMode = SetupEmMode::Last;//(SetupEmMode)(cfg.get<int>("EnemyWaveEditor.setupEmMode").value_or(1));
    rndWaveGen->on_config_load(cfg);
}

void EnemyWaveEditor::on_config_save(utility::Config& cfg) 
{
    //cfg.set<int>("EnemyWaveEditor.setupEmMode", setupEmMode);
    prflManager->on_config_save(cfg);
    rndWaveGen->on_config_save(cfg);
}

void EnemyWaveEditor::on_frame() 
{
}

void EnemyWaveEditor::on_draw_ui() {
  ImGui::Combo("Mode", (int*)&mode, modeNames.data(), modeNames.size());
  switch (mode) {
  case EnemyWaveEditor::Mod: 
  {
    ImGui::ShowHelpMarker("Create custom enemy lists to swap with the originals game's lists.");
    ImGui::Separator();
    if (ImGui::CollapsingHeader("How-to-use & Current issues"))
    {
        ImGui::BulletText("For missions/BP");
        print_spacing("Get original enemy data of a mission: select \"Get game's enemy data\" and start mission. Wait when info appears in trainer's window;");
        print_spacing("Setup custom enemy data for waves that you want to change;");
        print_spacing("Game can change an order of loading data for same mission. If you didn't get game data for mission, mod will sort custom data in ascending order by load id and swap data in that order. "
        "Otherwise your custom data will be \"link\" to original game's enemy list by load id (wrong swap still can orrurs if mission have same enemy lists);");
        print_spacing("(Re)Load mission;");
        ImGui::Separator();
        ImGui::BulletText("For BP warm up mode");
        print_spacing("Enable \"Bp alloc mode\", create 1 enemy list and setup set up it;");
        ImGui::Separator();
        ImGui::BulletText("Custom gamemodes");
        ImGui::TextWrapped(_txtHelper); ImGui::SameLine(0, ImGui::GetCursorPosX() - _txtHelperSize);
        ImGui::TextHyperlink("enemy list exporter shit from nexus", "https://www.nexusmods.com/devilmaycry5/mods/2181", ImVec4{ 0.46f, 0.46f, 0.92f, 1.0f });
        ImGui::SameLine(0, 0);
        ImGui::TextWrapped(" to serialize custom enemy data to .json and load it here;");
        ImGui::Spacing();
        print_spacing("Create profiles with names M01, M02, M03, ... M21 for each mission that you want to edit;");
        print_spacing("Bind default game data to profile (note that different default game modes (like human, sos, dmd) and modded LDK has different enemy lists, + Vergil mode also may have changes for some missions);");
        print_spacing(std::string("Save .json file in " + prflManager->get_gm_data_path()).c_str());
        print_spacing("In trainer window: select \"Import profile\", choose \"Custom game mode\" and select file with custom game mode;");
        ImGui::Separator();
        ImGui::BulletText("Current issues");
        print_spacing("Spawning a few different bosses at once can crash the game.");
        print_spacing("M18: Shadow/Griffon can't be swapped correctly (red seal will not disappear). If you want to add enemies to Griffon list, add all familiars to 0 list and enemies that u want to have in Griffon fight. "
        "Also add them to Griffon list itself.");
        print_spacing("Restoring original data during enemy spawn might crash the game;");
        print_spacing("Bosses can block spawn if them placed not in the end of the list or don't using \"near player\" flag in BP/some missions;");
        print_spacing("Urizen 2 on m8 can not be swapped correctly (softlock happens);");
        print_spacing("Changing \"boss wave\" on m7 LDK breaks Nico's van (softlock after exiting from van menu) for some reason...");
        print_spacing("You need to manually delete all enemies after fight if shadow/griffon/Dante was spawned in a custom wave and their bodies wasn't disappeared after death.");
        ImGui::TextWrapped("On Vergil mode m19 shares data with m20, so after finish m19 all enemy data will apply to m20 and game will be softlocked, if boss Dante is absent. Use coop trainer's \"Boss Swap\" feature "
        "to prevent that. And m19/20 always loads two waves: 1 with boss Vergil, 1 with boss Dante (at least if you have Vergil DLC).");
        ImGui::Separator();
    }

    /*if (ImGui::CollapsingHeader("Enemy fixes"))
    {
        ImGui::Spacing();
        EnemyFixes::draw_em_fixes();
        ImGui::Separator();
        ImGui::Spacing();
    }
    ImGui::ShowHelpMarker("Same stuff that enemy swapper has, just doubled here for comfort. Enable spawns offset for sure if you are adding flying enemies.");*/
    if (ImGui::CollapsingHeader("Active fixes"))
    {
        if (ImGui::Button("Request portal to next bp stage"))
        {
            if (GameplayStateTracker::gameMode == 3 && GameplayStateTracker::bpFlowId == 29)
            {
                auto bpManager = sdk::get_managed_singleton<REManagedObject*>("app.BloodyPalaceManager");
                if (bpManager != nullptr)
                    *(int*)((uintptr_t)bpManager + 0x60) = 26;
            }
        }
        ImGui::ShowHelpMarker("There is a bug when portal doesn't apeears after continue bp run from main menu with wave editor. Press this button to force to open a portal.");
        ImGui::Spacing();
        if (ImGui::Button("Kill all enemies in current wave"))
        {
            auto emManager = sdk::get_managed_singleton<REManagedObject>("app.EnemyManager");
            if (emManager != nullptr)
                sdk::call_object_func_easy<void*>(emManager, "killAllEnemy()");
        }
        ImGui::ShowHelpMarker("If you got softlocked 'cause Dante, parrot or cat don't despawning, or some another shit happened liek enemies was spawned behind the wall - here you go. "
        "Also always prees this after beat cat/parrot/Dante in completed enemy wave.");
    }
    //ImGui::Spacing();
    ImGui::ShowHelpMarker("Use these buttons to troubleshoot if something goes wrong.");
    ImGui::Separator();

    if (prflManager->is_custom_gamemode())
    {
        ImGui::TextWrapped("Custom game mode are enabled. After start a mission mod will automatically load enemy data from .json for current mission if it exists.");
        if(ImGui::Button("Go to default mode"))
            prflManager->reset_custom_gm();
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();
    }
    else
    {
        if (ImGui::CollapsingHeader("Wave editor randomizer"))
        {
            ImGui::TextWrapped("Enemies that can appear in randomized missions (click on enemy to select/remove):");
            rndWaveGen->show_enemy_selection();
            ImGui::Spacing();
            rndWaveGen->print_settings();
            if (ImGui::Button("Generate waves"))
            {
                if (mimListManager->get_game_data_list().empty())
                    prflManager->import_bp_default();
                rndWaveGen->generate_random_waves();
            }
            ImGui::ShowHelpMarker("Pressing this will remove all current setup and generate random waves. If there is no scanned enemy data, default BP data will be loaded and random waves for BP will be created. "
                "\nFor coop all setup must be shared between all players for correct sync. Also better disable spawn near player parameter.");
        }
        ImGui::ShowHelpMarker("Use this to randomize enemy waves. Add enemies to random pool (think that Nidhogg and Gilgamesh are here as a joke), setup other stuff that you want to be random or not and prees \"Generate waves\" button. "
            "You need to scan enemy data if you want random waves for missions/secret missions, otherwise 108 enemy lists for Bloody Palace will be generated. "
            "Better enable height spawn offset option for flying enemies in \"Enemy fixes\".");

        ImGui::Separator();
        /*ImGui::TextWrapped("Choose an enemy from original game's list, whoose settings will be used for spawn position and animation for custom lists:");
        ImGui::ShowHelpMarker("Each enemy has a list with settings of spawn pos & spawn animation. You can choose from what enemy from original list mod should take settings. Desync may happend in coop if settings not shared between players.");
        ImGui::RadioButton("First", (int*)&setupEmMode, 0); ImGui::SameLine();
        ImGui::Spacing(); ImGui::SameLine();
        ImGui::RadioButton("Last", (int*)&setupEmMode, 1);
        ImGui::Separator();*/
        ImGui::Checkbox("Bp warm up mode", &isBpWarmUpMode);
        ImGui::ShowHelpMarker("Select this if you want to go to warm up BP. Create 1 custom list and customize it. It will be copied to all bp game's list automatically.");
       
        ImGui::Separator();
    }
    
    draw_mimic_list_ui();
    break;
  }
  case EnemyWaveEditor::ViewUserData: 
  {
    ImGui::ShowHelpMarker("View all custom enemy lists data.");
    ImGui::Separator();
    if (!isBpWarmUpMode)
    {
        ImGui::RadioButton("List all", (int*)&viewUserDataState, 0); ImGui::SameLine();
        ImGui::Spacing(); ImGui::SameLine();
        ImGui::RadioButton("By selecting specific list", (int*)&viewUserDataState, 1);
        if (viewUserDataState == All)
        {
            for (int i = 0; i < mimListManager->count(); i++)
            {
                print_mimiclist_items(i);
                ImGui::Separator();
                ImGui::Separator();
            }
        }
        else
        {
            if(mimListManager->count() == 0)
                return;
            draw_emlist_combo();
            print_mimiclist_items(selectedMimicListItem);
        }
        
    }
    else if (mimListManager->count() != 0)
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
      prflManager->print_setup();
      break;
  }

  default:
    break;
  }
}

void EnemyWaveEditor::print_mimiclist_items(int i)
{
    ImGui::TextWrapped("List load id: %d", mimListManager->get_mimic_list_data(i)->loadId);
    ImGui::Spacing();
    for (int j = 0; j < mimListManager->get_mimic_list_data(i)->mimicList.get_mimic_count(); j++)
    {
        ImGui::TextWrapped("Enemy index in list: %d", j);
        ImGui::TextWrapped("Enemy: %s", (*_emNames)[EnemyData::id_to_indx(mimListManager->get_mimic_list_data(i)->mimicList[j]->emId)]);
        ImGui::TextWrapped("Enemy num: %d", mimListManager->get_mimic_list_data(i)->mimicList[j]->num);
        ImGui::TextWrapped("Odds: %.1f", mimListManager->get_mimic_list_data(i)->mimicList[j]->odds);
        ImGui::TextWrapped("Use boss hp bar and boss camera: %d", mimListManager->get_mimic_list_data(i)->mimicList[j]->isBoss);
        ImGui::TextWrapped("Enemy can't be killed: %d", mimListManager->get_mimic_list_data(i)->mimicList[j]->isNoDie);
        ImGui::TextWrapped("Wait time min: %.1f", mimListManager->get_mimic_list_data(i)->mimicList[j]->waitTimeMin);
        ImGui::TextWrapped("Wait time max: %.1f", mimListManager->get_mimic_list_data(i)->mimicList[j]->waitTimeMax);
        ImGui::TextWrapped("Don't set orbs: %d", mimListManager->get_mimic_list_data(i)->mimicList[j]->isDontSetOrb);
        ImGui::TextWrapped("Is near player?: %d", mimListManager->get_mimic_list_data(i)->mimicList[j]->isNearPlayer);
        ImGui::Separator();
    }
}

void EnemyWaveEditor::print_emdata_input(SetEmData &data) {
  ImGui::TextWrapped("Enemy:");
  ImGui::Combo("##SelectEmCombmoBox", &data.selectedItem, _emNames->data(), _emNames->size(), 20);
  ImGui::ShowHelpMarker("All \"Enemy Swapper\" enemies and spawn pos change fixes works with this mod even when swapper disabled. ");
  data.emId = EnemyData::indx_to_id(data.selectedItem);
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

void EnemyWaveEditor::print_emlist_data()
{
    ImGui::TextWrapped("All lists count: %d", mimListManager->get_game_data_list().size());
    ImGui::Separator();
    for (const auto& item : *&mimListManager->get_game_data_list())
    {
        ImGui::TextWrapped("Load id: %d", item.loadId);
        //ImGui::TextWrapped("Enemy list address: %X", item.listAddr);
        ImGui::TextWrapped("Count: %d", item.count);
        ImGui::Spacing();
        for (const auto &data : item.emDataInfo)
        {
            ImGui::TextWrapped("Enemy: %s", (*_emNames)[EnemyData::id_to_indx(data.emId)]);
            ImGui::TextWrapped("Enemy num: %d", data.num);
            ImGui::Spacing();
        }
        ImGui::Separator();
        ImGui::Separator();
    }
}

void EnemyWaveEditor::emlist_btn()
{
    if (!mimListManager->get_game_data_list().empty())
    {
        if (ImGui::Button("Clear list"))
            clear_emlist();
        ImGui::ShowHelpMarker("Custom data should not be swapped before doing this.");
        ImGui::Spacing();
        if (ImGui::Button("Copy to clipboard"))
        {
            std::string all = "";
            for (const auto& item : *&mimListManager->get_game_data_list())
            {
                all += "Load id: ";
                all += std::string(std::to_string(item.loadId) + "\n");
                //all += std::string("Enemy list address: " + std::to_string(item.listAddr));
                all += "\n\n";
                for (const auto &data : item.emDataInfo)
                {
                    all += std::string("Enemy: " + std::string((*_emNames)[EnemyData::id_to_indx(data.emId)]));
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
    if (!mimListManager->is_any_swapped(false))
    {
        emReaderCounter = -1;
        mimListManager->clear_game_em_data();
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
        if (fromLst == toLst || mimListManager->get_mimic_list_data(fromLst, true) == nullptr || mimListManager->get_mimic_list_data(toLst, true) == nullptr)
        {
            fromLst = -1;
            toLst = -1;
        }
        else
            mimListManager->copy_to(fromLst, toLst);
    }
    ImGui::ShowHelpMarker("You need to input list's load id, not it's num on list of all custom lists. And you can't copy data to the same list.");
    if(ImGui::Button("Copy to all"))
    {
        if(mimListManager->get_mimic_list_data(fromLst, true) == nullptr)
            fromLst = -1;
        else
        {
            for (int i = 0; i < mimListManager->count(); i++)
            {
                if (mimListManager->get_mimic_list_data(i)->loadId == fromLst)
                {
                    for (int j = 0; j < mimListManager->count(); j++)
                    {
                        if (j == i)
                            continue;
                        mimListManager->copy_to(i, j, false);
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
    if (ImGui::BeginCombo("Current enemy list", mimListManager->get_list_name(selectedMimicListItem)->c_str()))
    {
        for (int i = 0; i < mimListManager->count(); i++)
        {
            isSelected = (selectedMimicListItem == i);
            if (ImGui::Selectable(mimListManager->get_list_name(i)->c_str(), isSelected))
                selectedMimicListItem = i;
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    std::string s = "Bineded game's data will be showed here if it exists;\n\n";
    if (mimListManager->get_mimic_list_data(selectedMimicListItem)->bindedEmData != nullptr)
    {
        for (const auto& emDt : mimListManager->get_mimic_list_data(selectedMimicListItem)->bindedEmData->emDataInfo)
        {
            s += "Enemy: " + std::string((*_emNames)[EnemyData::id_to_indx(emDt.emId)]) + ";\n";
            s += "Enemy num: " + std::to_string(emDt.num) + ";\n\n";
        }
    }
    ImGui::ShowHelpMarker(s.c_str());
}

void EnemyWaveEditor::bpmode_data_setup()
{
    clear_emlist();
    int listCount = mimListManager->count();
    if (listCount > 1)
    {
        for (int i = 1; i < listCount; i++)
        {
            mimListManager->remove_at(1);
        }
    }
    for (int i = 0; i <= 108; i++)
        mimListManager->add(i);
    for (int i = 1; i <= 108; i++)
        mimListManager->copy_to(0, i, false);
}

void WaveEditorMod::EnemyWaveEditor::print_em_colums(int indx)
{
    bool isSelected = false;
    if(ImGui::BeginListBox("##EnemiesDataListBox"))
    {
        for (int row = 0; row < mimListManager->get_mimic_list_data(selectedMimicListItem)->mimicList.get_mimic_count(); row++)
        {
            isSelected = (selectedEmDataItem == row);
            ImVec4 backgroundcolor = isSelected ? ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered) : SELECTABLE_STYLE_ACT;
            if (row == selectedEmDataItem)
                backgroundcolor = SELECTABLE_STYLE_HVR;
            ImGui::PushStyleColor(ImGuiCol_Header, backgroundcolor);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, SELECTABLE_STYLE_ACT_HVR);
            auto uniqStr = (*_emNames)[mimListManager->get_mimic_list_data(selectedMimicListItem)->mimicList[row]->selectedItem] + std::string(" x") +
                std::to_string(mimListManager->get_mimic_list_data(selectedMimicListItem)->mimicList[row]->num);  //+ std::string("##") + std::to_string(row);
            auto addInfo = add_line_em_info(mimListManager->get_mimic_list_data(selectedMimicListItem)->mimicList[row]);
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
    if (!mimListManager->is_any_swapped(false))
    {
        ImGui::TextWrapped("Enemy list's load id:");
        ImGui::InputInt("##emListIdinput", &emListId);
        if (emListId < 0)
            emListId = 0;
        if (ImGui::Button("Create enemy list"))
        {
            mimListManager->add(emListId);
            selectedMimicListItem = mimListManager->count() - 1;
        }
        ImGui::Spacing();
        ImGui::InputInt("Begin id", &addListInRangeMin);
        ImGui::InputInt("End id", &addListInRangeMax);
        if (ImGui::Button("Create enemy lists in range"))
        {
            for (int i = addListInRangeMin; i <= addListInRangeMax; i++)
            {
                mimListManager->add(i);
                selectedMimicListItem = mimListManager->count() - 1;
            }
        }
        ImGui::ShowHelpMarker("Create a few enemy lists with identificators on setted range");
        ImGui::Separator();
        ImGui::Spacing();
        if (mimListManager->count() != 0)
        {
            if (!mimListManager->is_any_swapped(false) && mimListManager->count() != 0)
            {
                draw_emlist_combo();
                ImGui::Spacing();
                if (!mimListManager->get_game_data_list().empty())
                {
                    ImGui::Separator();
                    ImGui::InputInt("New load id", &newLoadId);
                    ImGui::ShowHelpMarker("Load id must be >=0. If not all id's is unique, after pressing allocate button this field will be \"-2\".");
                    if (ImGui::Button("Change load id"))
                    {
                        if(newLoadId >= 0)
                            mimListManager->change_loadid(selectedMimicListItem, newLoadId);
                        else newLoadId = -1;
                    }
                    ImGui::Separator();
                    ImGui::Spacing();
                }

                if (ImGui::Button("Remove selected enemy list"))
                {
                    mimListManager->remove_at(selectedMimicListItem);
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
                    mimListManager->remove_all();
                    return;
                }
                ImGui::Separator();
                ImGui::TextWrapped("Enemy data editor");
                ImGui::Spacing();

                ImGui::RadioButton("Add new enemy data", &emChangeState, 0); ImGui::SameLine();
                ImGui::Spacing(); ImGui::SameLine();
                ImGui::RadioButton("Edit existed enemy data", &emChangeState, 1);
                if (mimListManager->get_mimic_list_data(selectedMimicListItem)->mimicList.get_mimic_count() == 0)
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
                        mimListManager->add_data(new SetEmData(curCustomEmData), selectedMimicListItem);
                    }
                }
                else
                {
                    print_em_colums(selectedMimicListItem);
                    if (selectedEmDataItem < 0)//if first item was removed
                        selectedEmDataItem = 0;
                    print_emdata_input(*(mimListManager->get_mimic_list_data(selectedMimicListItem)->mimicList[selectedEmDataItem]));
                    ImGui::Spacing();
                    if (ImGui::Button("Remove selected enemy data item from list"))
                    {
                        mimListManager->get_mimic_list_data(selectedMimicListItem)->mimicList.remove_at(selectedEmDataItem--);
                    }
                }
                ImGui::Separator();
                ImGui::TextWrapped("Current enemy list count = %d;", mimListManager->get_mimic_list_data(selectedMimicListItem)->mimicList.get_mimic_count());
                ImGui::Separator();
                
                if(ImGui::CollapsingHeader("Copy data"))
                    draw_copy_list_data();
                ImGui::Separator();
            }
        }
    }
    else
    {
        ImGui::TextWrapped("Original game's enemy data was successfully swapped with custom data.");
        ImGui::Spacing();
        if (ImGui::Button("Restore original data"))
        {
            mimListManager->restore_all_data();
            emSetterCounter = 0;
        }
        ImGui::ShowHelpMarker("If you want to change custom enemy data during gameplay, press this button, make changes and reload level.");
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

void WaveEditorMod::EnemyWaveEditor::WaveRandomGenerator::show_enemy_selection()
{
    bool isSelected = false;

    if (ImGui::BeginTable("##RndTableEnemy", 3))
    {
        for (int i = 0; i < _emNames->size(); i++)
        {
            ImGui::TableNextColumn();
            ImVec4 backgroundcolor = isSelected ? ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered) : SELECTABLE_STYLE_ACT;
            isSelected = is_in_list(i);
            if (isSelected)
                backgroundcolor = SELECTABLE_STYLE_HVR;
            ImGui::PushStyleColor(ImGuiCol_Header, backgroundcolor);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, SELECTABLE_STYLE_ACT_HVR);
            
            if (!isSelected)
            {
                if (ImGui::Selectable((*_emNames)[i], isSelected))
                {
                    availableEnemies.push_back(i);
                    sort();
                }
                else
                    backgroundcolor = SELECTABLE_STYLE_HVR;
            }
            else
            {
                if (ImGui::Selectable((*_emNames)[i], isSelected))
                {
                    remove_item(i);
                    sort();
                }
            }
            ImGui::PopStyleColor(2);
        }
        ImGui::EndTable();
    }
}

inline void EnemyWaveEditor::WaveRandomGenerator::on_config_load(const utility::Config &cfg)
{
    set_seed(cfg.get<unsigned int>("EnemyWaveEditor.WaveRandomGenerator.seed").value_or(mt.default_seed));
    minGen = cfg.get<unsigned int>("EnemyWaveEditor.WaveRandomGenerator.minGen").value_or(1);
    maxGen = cfg.get<unsigned int>("EnemyWaveEditor.WaveRandomGenerator.maxGen").value_or(4);
    minNum = cfg.get<unsigned int>("EnemyWaveEditor.WaveRandomGenerator.minNum").value_or(1);
    maxNum = cfg.get<unsigned int>("EnemyWaveEditor.WaveRandomGenerator.maxNum").value_or(3);

    minOdds = cfg.get<float>("EnemyWaveEditor.WaveRandomGenerator.minOdds").value_or(100.0f);
    maxOdds = cfg.get<float>("EnemyWaveEditor.WaveRandomGenerator.maxOdds").value_or(100.0f);
    /*waitTimeMinRndMin = cfg.get<float>("EnemyWaveEditor.WaveRandomGenerator.waitTimeMinRndMin").value_or(0);
    waitTimeMinRndMax = cfg.get<float>("EnemyWaveEditor.WaveRandomGenerator.waitTimeMinRndMax").value_or(0);
    waitTimeMaxRndMin = cfg.get<float>("EnemyWaveEditor.WaveRandomGenerator.waitTimeMaxRndMin").value_or(0);
    waitTimeMaxRndMax = cfg.get<float>("EnemyWaveEditor.WaveRandomGenerator.waitTimeMaxRndMax").value_or(0);*/
    waitTimeMin = cfg.get<float>("EnemyWaveEditor.WaveRandomGenerator.waitTimeMin").value_or(0);
    waitTimeMax = cfg.get<float>("EnemyWaveEditor.WaveRandomGenerator.waitTimeMax").value_or(0);

    isRandomNumGen = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isRandomNumGen").value_or(true);
    isRandomEmNumGen = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isRandomEmNumGen").value_or(true);
    isRndOdds = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isRndOdds").value_or(false);
    isNoOrbs = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isNoOrbs").value_or(false);
    isRndNoOrbs = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isRndNoOrbs").value_or(true);
    isNearPl = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isNearPl").value_or(false);
    isRndNearPl = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isRndNearPl").value_or(false);
    isAlwaysBossHUDForBosses = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isAlwaysBossHUDForBosses").value_or(true);
    //isRndWaitTime = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isRndWaitTime").value_or(true);
    isAddEnemiesToBosses = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isAddEnemiesToBosses").value_or(false);
    isBossNumAlwaysOne = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isBossNumAlwaysOne").value_or(true);
    isAlwaysSingleGenBossInWave = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isAlwaysSingleGenBossInWave").value_or(true);
    isAlwaysSingleBossInWave = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isAlwaysSingleBossInWave").value_or(true);
    isNearPlIfBoss = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isNearPlIfBoss").value_or(false);
    isRndOnRestart = false;//cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.isRndOnRestart").value_or(false);

    std::string emNameParam = "EnemyWaveEditor.WaveRandomGenerator.availableEnemies.";
    for (int i = 0; i < _emNames->size(); i++)
    {
        if (cfg.get<bool>(emNameParam + (*_emNames)[i]).value_or(false))
            availableEnemies.push_back(i);
    }
    for (auto& em : heavyEmList)
        em.on_config_load(cfg);
}

inline void EnemyWaveEditor::WaveRandomGenerator::on_config_save(utility::Config& cfg) const
{
    cfg.set<unsigned int>("EnemyWaveEditor.WaveRandomGenerator.seed", seed);
    cfg.set<unsigned int>("EnemyWaveEditor.WaveRandomGenerator.minGen", minGen);
    cfg.set<unsigned int>("EnemyWaveEditor.WaveRandomGenerator.maxGen", maxGen);
    cfg.set<unsigned int>("EnemyWaveEditor.WaveRandomGenerator.minNum", minNum);
    cfg.set<unsigned int>("EnemyWaveEditor.WaveRandomGenerator.maxNum", maxNum);

    cfg.set<float>("EnemyWaveEditor.WaveRandomGenerator.minOdds", minOdds);
    cfg.set<float>("EnemyWaveEditor.WaveRandomGenerator.maxOdds", maxOdds);
  /*  cfg.set<float>("EnemyWaveEditor.WaveRandomGenerator.waitTimeMinRndMin", waitTimeMinRndMin);
    cfg.set<float>("EnemyWaveEditor.WaveRandomGenerator.waitTimeMinRndMax", waitTimeMinRndMax);
    cfg.set<float>("EnemyWaveEditor.WaveRandomGenerator.waitTimeMaxRndMin", waitTimeMaxRndMin);
    cfg.set<float>("EnemyWaveEditor.WaveRandomGenerator.waitTimeMaxRndMax", waitTimeMaxRndMax);*/
    cfg.set<float>("EnemyWaveEditor.WaveRandomGenerator.waitTimeMin", waitTimeMin);
    cfg.set<float>("EnemyWaveEditor.WaveRandomGenerator.waitTimeMax", waitTimeMax);

    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isRandomNumGen", isRandomNumGen);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isRandomEmNumGen", isRandomEmNumGen);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isRndOdds", isRndOdds);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isNoOrbs", isNoOrbs);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isRndNoOrbs", isRndNoOrbs);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isNearPl", isNearPl);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isRndNearPl", isRndNearPl);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isAlwaysBossHUDForBosses", isAlwaysBossHUDForBosses);
    //cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isRndWaitTime", isRndWaitTime);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isAddEnemiesToBosses", isAddEnemiesToBosses);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isBossNumAlwaysOne", isBossNumAlwaysOne);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isAlwaysSingleGenBossInWave", isAlwaysSingleGenBossInWave);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isAlwaysSingleBossInWave", isAlwaysSingleBossInWave);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isNearPlIfBoss", isNearPlIfBoss);
    //cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.isRndOnRestart", isRndOnRestart);

    std::string emNameParam = "EnemyWaveEditor.WaveRandomGenerator.availableEnemies.";
    for (int i : availableEnemies)
    {
        cfg.set<bool>(emNameParam + (*_emNames)[i], true);
    }
    for(const auto &em : heavyEmList)
        em.on_config_save(cfg);
}

void EnemyWaveEditor::WaveRandomGenerator::EmRestriction::print_settings()
{
    ImGui::Checkbox(("Special settings for " + std::string((*_emNames)[curEmId])).c_str(), &isEnabled);
    if (isEnabled)
    {
        ImGui::TextWrapped(("Max " + std::string((*_emNames)[curEmId]) + " in enemy list:").c_str());
        wrg->input_int(("##MaxGenSpecEm" + std::string((*_emNames)[curEmId])).c_str(), maxGenInList);
        ImGui::TextWrapped(("Min num for each " + std::string((*_emNames)[curEmId])).c_str());
        wrg->input_int(("##SpecEnemyMin" + std::string((*_emNames)[curEmId])).c_str(), minNum);
        ImGui::TextWrapped(("Max num for each " + std::string((*_emNames)[curEmId])).c_str());
        wrg->input_int(("##SpecEnemyMax" + std::string((*_emNames)[curEmId])).c_str(), maxNum);
    }
}

inline void WaveEditorMod::EnemyWaveEditor::WaveRandomGenerator::EmRestriction::on_config_load(const utility::Config& cfg)
{
    auto emName = std::string((*_emNames)[curEmId]);
    minNum = cfg.get<int>("EnemyWaveEditor.WaveRandomGenerator.EmRestriction.minNum_" + emName).value_or(1);
    maxNum = cfg.get<int>("EnemyWaveEditor.WaveRandomGenerator.EmRestriction.maxNum_" + emName).value_or(2);
    maxGenInList = cfg.get<int>("EnemyWaveEditor.WaveRandomGenerator.EmRestriction.maxGenInList_" + emName).value_or(2);
    isEnabled = cfg.get<bool>("EnemyWaveEditor.WaveRandomGenerator.EmRestriction.isEnabled_" + emName).value_or(false);
}

inline void WaveEditorMod::EnemyWaveEditor::WaveRandomGenerator::EmRestriction::on_config_save(utility::Config& cfg) const
{
    auto emName = std::string((*_emNames)[curEmId]);
    cfg.set<bool>("EnemyWaveEditor.WaveRandomGenerator.EmRestriction.isEnabled_" + emName, isEnabled);
    cfg.set<int>("EnemyWaveEditor.WaveRandomGenerator.EmRestriction.maxGenInList_" + emName, maxGenInList);
    cfg.set<int>("EnemyWaveEditor.WaveRandomGenerator.EmRestriction.minNum_" + emName, minNum);
    cfg.set<int>("EnemyWaveEditor.WaveRandomGenerator.EmRestriction.maxNum_" + emName, maxNum);
}
