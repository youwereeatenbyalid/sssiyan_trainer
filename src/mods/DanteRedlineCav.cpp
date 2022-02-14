
#include "DanteRedlineCav.hpp"
#include "PlayerTracker.hpp"
uintptr_t DanteRedlineCav::jmp_ret{ NULL };
uintptr_t DanteRedlineCav::jmp_jne{ NULL };
bool DanteRedlineCav::cheaton{ NULL };

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
    __asm {
        cmp [PlayerTracker::playerid], 1
        jne code
        cmp byte ptr[DanteRedlineCav::cheaton], 1
        je cheatcode

    code:
        cmp dword ptr [rdx+0x000018B0], 06
        jne jnecode
    cheatcode:
        jmp qword ptr[DanteRedlineCav::jmp_ret]
    jnecode:
        jmp qword ptr[DanteRedlineCav::jmp_jne]
    }
}

// clang-format on

void DanteRedlineCav::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteRedlineCav::on_initialize() {
    init_check_box_info();

    m_is_enabled = &DanteRedlineCav::cheaton;
    m_on_page = dantecheat;

    m_full_name_string = "Redline On Cavaliere";
    m_author_string = "The Hitchhiker";
    m_description_string = "Allows you to use the Cavaliere R exclusive move on regular Cavaliere.";

    set_up_hotkey();

    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
    auto addr = patterns->find_addr(base, "83 BA B0 18 00 00 06");
    if (!addr) {
        return "Unable to find DanteRedlineCav pattern.";
    }

    if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 9)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize DanteRedlineCav";
    }

    DanteRedlineCav::jmp_jne = addr.value() - 13;

    return Mod::on_initialize();
}

// void DanteRedlineCav::on_draw_ui() {}
