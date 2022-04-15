#include "GameplayStateTracker.hpp"
#include "EnemyWaveEditor.hpp"

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
        mov eax, [rdx+0x88]
        mov [GameplayStateTracker::isInMission], ah
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
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 0
        je originalcode
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::isBPFixRequested], 0
        je originalcode

        cheat:
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::isRequestEndBpStage], 1
        je endbpstage
        cmp byte ptr [rcx+0x60], 0x15 //21
        jne originalcode
        //mov byte ptr [EnemyWaveEditor::isBPFixRequested], 0
        mov dword ptr [rcx+0x60], 0x16 //22
        //----------FadeManager.RequestType-------------//
        mov rax, [WaveEditorMod::EnemyWaveEditor::fadeStaticBase]
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
        mov dword ptr [GameplayStateTracker::bpFlowId], eax
        sub eax, 0x16
        jmp qword ptr [GameplayStateTracker::bpRetJmp]

        endbpstage:
        mov byte ptr [WaveEditorMod::EnemyWaveEditor::isRequestEndBpStage], 0
        mov dword ptr [rcx+0x60], 0x1E //30
        jmp originalcode
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


std::optional<std::string> GameplayStateTracker::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

    auto nowFlowAddr = m_patterns_cache->find_addr(base, "48 8B DA 48 8B F9 83 F8 1A"); // DevilMayCry5.exe+89429E 
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