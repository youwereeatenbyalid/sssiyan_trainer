#include "DanteSDTRework.hpp"
#include "PlayerTracker.hpp"
// referenced in MaxSDT and DanteAlwaysQ4SDT
uintptr_t DanteSDTRework::jmp_ret1{ NULL };
uintptr_t DanteSDTRework::jmp_jne1{ NULL };
uintptr_t DanteSDTRework::jmp_ret2{ NULL };
uintptr_t DanteSDTRework::jmp_jae2{ NULL };
uintptr_t DanteSDTRework::jmp_ret3{ NULL };
uintptr_t DanteSDTRework::jmp_ret4{ NULL };
uintptr_t DanteSDTRework::jmp_ret5{ NULL };
uintptr_t DanteSDTRework::jmp_ret6{ NULL };
uintptr_t DanteSDTRework::jmp_ret7{ NULL };
uintptr_t DanteSDTRework::jmp_jne7{ NULL };
uintptr_t DanteSDTRework::jmp_ret8{ NULL };

bool DanteSDTRework::cheaton{ NULL };
float minSDTEnterFloat{ 3000.0f };
float maxSDTEnterFloat{ 10000.0f };
float newSDTSpeed{ 5.0f }; // quicksdt
float dTTick{ 5.0f };      // dt drain rate

// clang-format off
// only in clang/icl mode on x64, sorry

float xmm1backup{ NULL };
float xmm2backup{ NULL };

static naked void detour1() { // DrainDTInsteadOfSDT
    __asm {
        cmp [PlayerTracker::playerid], 1
        jne code
        cmp byte ptr [DanteSDTRework::cheaton], 1
        je cheatcode
    code:
        cmp eax, 01
        jne jnecode
    retcode:
        jmp qword ptr[DanteSDTRework::jmp_ret1]

    cheatcode:
        cmp byte ptr [rdx+0x9B0], 2 // Drain DT faster if in SDT
        jne code
        movss [xmm1backup], xmm1
        movss [xmm2backup], xmm2
        movss xmm1, [rdx+0x1110] // dt
        movss xmm2, [dTTick]
        mulss xmm2, [rdx+0x128]  // delta
        subss xmm1, xmm2
        movss [rdx+0x1110], xmm1
        movss xmm1, [xmm1backup]
        movss xmm2, [xmm2backup]
        jmp qword ptr [DanteSDTRework::jmp_ret1]

    jnecode:
        jmp qword ptr [DanteSDTRework::jmp_jne1]
    }
}

static naked void detour2() { // DisableChargingSDTWithDT
    __asm {
        cmp [PlayerTracker::playerid], 1
        jne code
        cmp byte ptr [DanteSDTRework::cheaton], 1
        je cheatcode
    code:
        comisd xmm2, xmm1
        jae jaecode
    cheatcode:
        jmp qword ptr [DanteSDTRework::jmp_ret2]

    jaecode:
        jmp qword ptr [DanteSDTRework::jmp_jae2]
    }
}

static naked void detour3() { // EnterSDTWithMoreThan3kMeter
    __asm {
        cmp [PlayerTracker::playerid], 1
        jne code
        cmp byte ptr [DanteSDTRework::cheaton], 1
        je cheatcode
    code:
        movss xmm1, [rdi+0x00001A14]
        movss xmm0, [maxSDTEnterFloat]
        jmp qword ptr [DanteSDTRework::jmp_ret3]

    cheatcode:
        movss xmm1, [rdi+0x00001110]
        movss xmm0, [minSDTEnterFloat]
        jmp qword ptr [DanteSDTRework::jmp_ret3]
    }
}

static naked void detour4() { // CurrentSDTFillGraphic
    __asm {
        cmp [PlayerTracker::playerid], 1
        jne code
        cmp byte ptr [DanteSDTRework::cheaton], 1
        je cheatcode
    code:
        movss xmm1, [rdx+0x00001A14]
        jmp qword ptr [DanteSDTRework::jmp_ret4]

    cheatcode:
        mov dword ptr [rdx+0x00001A14], 0 // write real sdt 0
        movss xmm1, [rdx+0x00001110]
        jmp qword ptr [DanteSDTRework::jmp_ret4]
    }
}

static naked void detour5() { // ShowSDTEnterGraphicAtMoreThan3kMeter
    __asm {
        cmp [PlayerTracker::playerid], 1
        jne code
        cmp byte ptr [DanteSDTRework::cheaton], 1
        je cheatcode
    code:
        movss xmm1, [rdx+0x00001A14]
        movss xmm0, [maxSDTEnterFloat]
        jmp qword ptr [DanteSDTRework::jmp_ret5]

    cheatcode:
        movss xmm1, [rdx+0x00001110]
        movss xmm0, [minSDTEnterFloat]
        jmp qword ptr [DanteSDTRework::jmp_ret5]
    }
}

static naked void detour6() { // QuickSDT
    __asm {
        cmp [PlayerTracker::playerid], 1
        jne code
        cmp byte ptr [DanteSDTRework::cheaton], 1
        je cheatcode
    code:
        movss xmm0, [rdi+00000128h]
        jmp qword ptr [DanteSDTRework::jmp_ret6]

    cheatcode:
        movss xmm0, [rdi+00000128h]
        mulss xmm0, [newSDTSpeed]
		jmp qword ptr [DanteSDTRework::jmp_ret6]
    }
}

static naked void detour7() { // DrainDTDuringJudgement
    __asm {
        cmp [PlayerTracker::playerid], 1
        jne code
        cmp byte ptr [DanteSDTRework::cheaton], 1
        je cheatcode
    code:
        test ecx, ecx
        jne jnecode
    cheatcode:
		jmp qword ptr [DanteSDTRework::jmp_ret7]

    jnecode:
        jmp qword ptr [DanteSDTRework::jmp_jne7]
    }
}

static naked void detour8() { // 3k SDT bar max
    __asm {
        cmp [PlayerTracker::playerid], 1
        jne code
        cmp byte ptr [DanteSDTRework::cheaton], 1
        je cheatcode
    code:
        movss xmm2, [maxSDTEnterFloat]
        jmp qword ptr[DanteSDTRework::jmp_ret8]
    cheatcode:
        movss xmm2, [minSDTEnterFloat]
		jmp qword ptr [DanteSDTRework::jmp_ret8]
    }
}

// clang-format on

void DanteSDTRework::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteSDTRework::on_initialize() {
    init_check_box_info();

    m_is_enabled = &DanteSDTRework::cheaton;
    m_on_page = dantesdt;

    m_full_name_string = "SDT Rework";
    m_author_string = "SSSiyan, VPZadov";
    m_description_string = "DT and SDT are a shared resource. Tap to DT, Hold to SDT. At least 3 bars of DT are required to enter SDT.";

    set_up_hotkey();
    
    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
    
    auto addr1 = patterns->find_addr(base, "83 F8 01 75 12 48 8B CF");
    if (!addr1) {
        return "Unable to find DanteSDTRework pattern1.";
    }
    if (!install_hook_absolute(addr1.value(), m_function_hook1, &detour1, &jmp_ret1, 5)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize DanteSDTRework1";
    }
    DanteSDTRework::jmp_jne1 = addr1.value() + 23; // DevilMayCry5.exe+196A4F4 copyright update
    
    auto addr2 = patterns->find_addr(base, "66 0F 2F D1 73 25 F3 0F 10 8F 14");
    if (!addr2) {
        return "Unable to find DanteSDTRework pattern2.";
    }
    if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 6)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize DanteSDTRework2";
    }
    DanteSDTRework::jmp_jae2 = addr2.value() + 43; // DevilMayCry5.exe+19703D7 copyright update
    
    auto addr3 = patterns->find_addr(base, "D9 F3 0F 10 8F 14 1A 00 00");
    if (!addr3) {
        return "Unable to find DanteSDTRework pattern3.";
    }
    if (!install_hook_absolute(addr3.value() + 1, m_function_hook3, &detour3, &jmp_ret3, 16)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize DanteSDTRework3";
    }
    
    auto addr4 = patterns->find_addr(base, "F3 0F 10 8A 14 1A 00 00 0F");
    if (!addr4) {
        return "Unable to find DanteSDTRework pattern4.";
    }
    if (!install_hook_absolute(addr4.value(), m_function_hook4, &detour4, &jmp_ret4, 8)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize DanteSDTRework4";
    }
    
    auto addr5 = patterns->find_addr(base, "1D F3 0F 10 8A 14 1A 00 00");
    if (!addr5) {
        return "Unable to find DanteSDTRework pattern5.";
    }
    if (!install_hook_absolute(addr5.value() + 1, m_function_hook5, &detour5, &jmp_ret5, 16)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize DanteSDTRework5";
    }

    auto addr6 = patterns->find_addr(base, "05 05 00 00 F3 0F 10 8F 24 11 00 00 F3 0F 10 87 28 01 00 00");
    if (!addr6) {
        return "Unable to find DanteSDTRework pattern6.";
    }
    if (!install_hook_absolute(addr6.value() + 12, m_function_hook6, &detour6, &jmp_ret6, 8)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize DanteSDTRework6";
    }
    
    auto addr7 = patterns->find_addr(base, "85 C9 0F 85 8A 00 00 00 48 8B 03");
    if (!addr7) {
        return "Unable to find DanteSDTRework pattern7.";
    }
    if (!install_hook_absolute(addr7.value(), m_function_hook7, &detour7, &jmp_ret7, 8)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize DanteSDTRework7";
    }
    DanteSDTRework::jmp_jne7 = addr7.value() + 146; // DevilMayCry5.exe+16A212B copyright update

    auto addr8 = patterns->find_addr(base, "F3 0F 10 15 39 0F 10 06");
    if (!addr8) {
        return "Unable to find DanteSDTRework pattern7.";
    }
    if (!install_hook_absolute(addr8.value(), m_function_hook8, &detour8, &jmp_ret8, 8)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize DanteSDTRework7";
    }

    return Mod::on_initialize();
}

// void DanteSDTRework::on_draw_ui() {}
