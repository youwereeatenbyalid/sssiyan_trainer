#include "InfHP.hpp"
#include "PlayerTracker.hpp"
uintptr_t InfHP::jmp_ret{ NULL };
bool InfHP::cheaton{ NULL };


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
    __asm {
        //cmp[PlayerTracker::playerid], 1 //change this to the char number obviously
        //jne code
        cmp byte ptr [InfHP::cheaton], 1
        jne code
        cmp rdx, [PlayerTracker::playerentity]
        jne code
        movss xmm6, [rbx+0x14]
    code:
        movss [rbx+0x10], xmm6
        jmp qword ptr[InfHP::jmp_ret]
    }
}

// clang-format on

void InfHP::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> InfHP::on_initialize() {
    init_check_box_info();

    m_is_enabled         = &InfHP::cheaton;
    m_on_page            = Page_CommonCheat;
    m_depends_on         = { "PlayerTracker" };
    m_full_name_string   = "Infinite HP";
    m_author_string      = "SSSiyan";
    m_description_string = "Freeze Player HP to max.";

    set_up_hotkey();

    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
    auto addr = m_patterns_cache->find_addr(base, "F3 0F 11 73 10 48 8B 5C 24 40 48");
    if (!addr) {
        return "Unable to find InfHP pattern.";
    }

    //if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //    //  return a error string in case something goes wrong
    //    spdlog::error("[{}] failed to initialize", get_name());
    //    return "Failed to initialize InfHP";
    //}

    //m_detour = std::make_shared<Detour_t>(*addr, &detour, 5);
    //m_detours.push_back(m_detour);
    //if (!m_detour->is_valid()) {
    //    spdlog::error("[{}] failed to initialize", get_name());
    //    return "Failed to initialize InfHP";
    //}
    //jmp_ret = m_detour->get_return_address();

    if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 5)) {
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize InfHP";
    }

    return Mod::on_initialize();
}
