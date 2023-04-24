#include "GameplayStateTracker.hpp"
#include "EnemyWaveEditor.hpp"

void GameplayStateTracker::pfb_info_add_hook(uintptr_t threadCtxt, uintptr_t pfbInfoList, uintptr_t pfbInfo)
{
    _mod->m_after_pfbmanager_init->get_original<decltype(pfb_info_add_hook)>()(threadCtxt, pfbInfoList, pfbInfo);
    if (pfbInfoList != 0 && GameFunctions::ListController::get_list_count(pfbInfoList) >= 43)
        _mod->_pfbManagerInited.invoke();
}

void GameplayStateTracker::ui3500Gui_do_on_open_hook(uintptr_t threadCntx, uintptr_t ui3500Gui)
{
    _mod->_onUi3500GuiOpen.invoke(threadCntx, ui3500Gui);
    _mod->_ui3500GuiDoOnOpenHook->get_original<decltype(ui3500Gui_do_on_open_hook)>()(threadCntx, ui3500Gui);
}

void GameplayStateTracker::ui3500Gui_do_on_closed_hook(uintptr_t threadCntx, uintptr_t ui3500Gui)
{
    _mod->_onUi3500GuiClosed.invoke(threadCntx, ui3500Gui);
    _mod->_ui3500GuiDoOnClosedHook->get_original<decltype(ui3500Gui_do_on_closed_hook)>()(threadCntx, ui3500Gui);
}

static naked void gamemode_detour()
{
    __asm {
        push rcx
        mov ecx, dword ptr [rax + 0x90]
        mov dword ptr [GameplayStateTracker::gameMode], ecx
        pop rcx
        cmp [rax + 0x00000090], edi
        jmp qword ptr [GameplayStateTracker::gameModeRet]
    }
}

static naked void now_flow_detour() {
    __asm {
        mov dword ptr [GameplayStateTracker::nowFlow], eax
        mov al, byte ptr [rdx+0x88]
        mov [GameplayStateTracker::isInMission], al
        mov eax, dword ptr [rdx + 0x54]//nowFlow
        cmp [GameplayStateTracker::flowTmp], eax
        jne changeprev
        jmp originalcode

        changeprev:
        mov eax, [GameplayStateTracker::flowTmp]
        mov [GameplayStateTracker::prevFlow], eax
        mov eax, dword ptr [GameplayStateTracker::nowFlow]
        mov [GameplayStateTracker::flowTmp], eax
        jmp originalcode

        originalcode:
        mov rbx,rdx
        mov rdi,rcx
        jmp qword ptr [GameplayStateTracker::nowFlowRet]
  }
}

static naked void bp_forceload_detour()
{
    __asm {
        originalcode:
        mov eax, [rcx+0x60]
        mov dword ptr [GameplayStateTracker::bpFlowId], eax
        sub eax, 0x16
        jmp qword ptr [GameplayStateTracker::bpRetJmp]
    }
}

static naked void is_cutscene_detour()
{
    __asm {
        mov byte ptr [GameplayStateTracker::isCutscene], cl
        mov [rsi + 0x00000094], cl
        jmp qword ptr [GameplayStateTracker::isCutsceneRet]
    }
}

static naked void is_pause_exe_detour()
{
    __asm {
        mov byte ptr [GameplayStateTracker::isExecutePause], al
        mov byte ptr [r9 + 0x24], al
        setne r8b //r8l original
        jmp qword ptr [GameplayStateTracker::isPauseExeRet]
    }
}

std::optional<std::string> GameplayStateTracker::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

    auto nowFlowAddr = m_patterns_cache->find_addr(base, "48 8B DA 48 8B F9 83 F8 1A"); // DevilMayCry5.exe+8A6B1E
    if (!nowFlowAddr)
    {
        return "Unanable to find GameplayStateTracker.nowFlowAddr pattern.";
    }

    auto gameModeAddr = m_patterns_cache->find_addr(base, "39 B8 90 00 00 00 48"); // DevilMayCry5.exe+132B592
    if (!gameModeAddr)
    {
        return "Unanable to find GameplayStateTracker.gameMode pattern.";
    }

    auto bpFlowIdAddr = m_patterns_cache->find_addr(base, "19 8B 41 60 83 E8 16");// DevilMayCry5.exe+1547B29
    if (!bpFlowIdAddr)
    {
        return "Unanable to find bpFlowIdAddr pattern.";
    }

    auto isCutsceneAddr = m_patterns_cache->find_addr(base, "88 8E 94 00 00 00");//DevilMayCry5.exe+FD9606
    if (!isCutsceneAddr)
    {
        return "Unable to find GameplayStateTracker.isCutsceneAddr.";
    }

    auto isPauseAddr = m_patterns_cache->find_addr(base, "41 88 41 24 41 0F 95 C0");//DevilMayCry5.exe+1330642
    if (!isPauseAddr)
    {
        return "Unable to find GameplayStateTracker.isPauseAddr.";
    }

    auto pfbManagerAddItemAddr = m_patterns_cache->find_addr(base, "24 01 48 8B 5C 24 30 48 83 C4 20 5F C3 CC CC 48 89 5C 24 08 48");
    //DevilMayCry5.System_Collections_Generic_List_1_app_EnemyManager_EnemyPrefabManager_EnemyPrefabInfo___Add160796 (-0xF)
    if (!pfbManagerAddItemAddr)
    {
        return "Unable to find GameplayStateTracker.pfbManagerAddItemAddr.";
    }

    auto ui3500GuiDoOnOpenAddr = m_patterns_cache->find_addr(base, "40 55 53 57 48 8D AC 24 40 FC FF FF");
    //DevilMayCry5.app_ui3500GUI__doOnOpen289005
    if (!ui3500GuiDoOnOpenAddr)
    {
        return "Unable to find GameplayStateTracker.ui3500GuiDoOnOpenAddr.";
    }

    auto ui3500GuiDoOnClosedAddr = m_patterns_cache->find_addr(base, "5B 5D C3 CC CC CC CC CC CC CC CC CC CC CC 48 89 5C 24 10 56");
    //DevilMayCry5.app_ui3500GUI__doOnOpen289005 (-0xE)
    if (!ui3500GuiDoOnClosedAddr)
    {
        return "Unable to find GameplayStateTracker.ui3500GuiDoOnClosedAddr.";
    }

    _mod = this;

    if (!install_hook_absolute(nowFlowAddr.value(), m_now_flow_hook, &now_flow_detour, &nowFlowRet, 0x6))
    {
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize GameplayStateTracker.nowFlowAddr";
    }

    if (!install_hook_absolute(gameModeAddr.value(), m_gamemode_hook, &gamemode_detour, &gameModeRet, 0x6))
    {
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize GameplayStateTracker.gameModeAddr";
    }

    if (!install_hook_absolute(bpFlowIdAddr.value() + 0x1, m_bploadflow_hook, &bp_forceload_detour, &bpRetJmp, 0x6))
    {
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize GameplayStateTracker.bpFlowId";
    }

    if (!install_hook_absolute(isCutsceneAddr.value(), m_cutscene_hook, &is_cutscene_detour, &isCutsceneRet, 6))
    {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize GameplayStateTracker.isCutscene";
    }

    if (!install_hook_absolute(isPauseAddr.value(), m_pause_hook, &is_pause_exe_detour, &isPauseExeRet, 0x8))
    {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize GameplayStateTracker.isPause";
    }

    m_after_pfbmanager_init = std::make_unique<FunctionHook>(pfbManagerAddItemAddr.value() + 0xF, &pfb_info_add_hook);
    m_after_pfbmanager_init->create();

    _ui3500GuiDoOnOpenHook = std::make_unique<FunctionHook>(ui3500GuiDoOnOpenAddr.value(), &ui3500Gui_do_on_open_hook);
    if (!_ui3500GuiDoOnOpenHook->create())
        return "Can't create _ui3500GuiDoOnOpenHook.";

    _ui3500GuiDoOnClosedHook = std::make_unique<FunctionHook>(ui3500GuiDoOnClosedAddr.value() + 0xE, &ui3500Gui_do_on_closed_hook);
    if (!_ui3500GuiDoOnClosedHook->create())
        return "Can't create _ui3500GuiDoOnClosedHook.";

	return Mod::on_initialize();
}

void GameplayStateTracker::on_config_load(const utility::Config& cfg)
{
}

void GameplayStateTracker::on_config_save(utility::Config& cfg)
{
}

void GameplayStateTracker::on_frame()
{
}

void GameplayStateTracker::on_draw_ui()
{

}

void GameplayStateTracker::on_draw_debug_ui()
{
}

void GameplayStateTracker::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}


