#include "NeroInfCalibur.hpp"
#include "PlayerTracker.hpp"
uintptr_t NeroInfCalibur::jmp_ret{ NULL };
bool NeroInfCalibur::cheaton{ NULL };

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
    __asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [NeroInfCalibur::cheaton], 1
        je retjmp
    code:
        mov byte ptr [rdi+0x000018FA], 01
    retjmp:
        jmp qword ptr[NeroInfCalibur::jmp_ret]
    }
}

// clang-format on

void NeroInfCalibur::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroInfCalibur::on_initialize() {
    init_check_box_info();

    m_is_enabled         = &NeroInfCalibur::cheaton;
    m_on_page            = Page_Nero;
    m_depends_on         = { "PlayerTracker" };
    m_full_name_string   = "Infinite Caliburs";
    m_author_string      = "SSSiyan";
    m_description_string = "Allows Nero to perform multiple Caliburs.";

    set_up_hotkey();

    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
    auto addr = m_patterns_cache->find_addr(base, "C6 87 FA 18 00 00 013");
    if (!addr) {
        return "Unable to find NeroInfCalibur pattern.";
    }

    if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 7)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize NeroInfCalibur";
    }
    return Mod::on_initialize();
}

// void NeroInfCalibur::on_draw_ui() {}
