#include "NeoBalrog.hpp"
bool NeoBalrog::cheaton{NULL};
uintptr_t NeoBalrog::jmp_ret{ NULL };
void NeoBalrog::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}


static naked void neobalrog_ui_detour() {
    __asm {
        mov edx, 1
        jmp qword ptr[NeoBalrog::jmp_ret]
    }
}



std::optional<std::string> NeoBalrog::on_initialize() {
    init_check_box_info();
    m_scripts = {"BalrogScript.lua"};//scripts here
    m_is_enabled = &NeoBalrog::cheaton;
    m_on_page = Page_DanteStrive;
    m_full_name_string = "Neo Balrog";
    m_author_string = "The HitchHiker";
    m_description_string = "A modified moveset that unifies balrog's punch and kick movesets.\nTap attacks for kick moves and hold to enter welter stance.";

    set_up_hotkey();
    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
    auto addr = m_patterns_cache->find_addr(base, "8B 91 CC 18 00 00 48 85 C0 0F 84 81");
    if (!addr) {
        return "Unable to find NeoBalrogUI pattern.";
    }
    if (!install_new_detour(addr.value(), m_detour, &neobalrog_ui_detour, &jmp_ret, 6)) {
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize NeoBalrogUI";
    }
    return Mod::on_initialize();
}

void NeoBalrog::on_load_lua_mod()
{
    API::LuaLock _{};
    load_scripts();
    //running lua on load here
}

void NeoBalrog::on_unload_lua_mod()
{
    API::LuaLock _{};
}

void NeoBalrog::on_lua_mod_update()
{
    API::LuaLock _{};
    sol::state_view mod_state_view{ m_mod_state };
    //lua actions on update here
}

void NeoBalrog::on_draw_ui()
{
    //draw_lua_ui();
}

