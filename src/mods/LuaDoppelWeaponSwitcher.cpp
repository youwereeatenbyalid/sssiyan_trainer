#include "LuaDoppelWeaponSwitcher.hpp"
bool LuaDoppelWeaponSwitcher::cheaton{NULL};
uintptr_t LuaDoppelWeaponSwitcher::jmp_ret_1{ NULL };
uintptr_t LuaDoppelWeaponSwitcher::jmp_ret_2{ NULL };
void LuaDoppelWeaponSwitcher::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}

naked void LuaDoppelWeaponSwitcher::init_detour_1()
{
    __asm {
        cmp rax, rax
        jmp qword ptr[LuaDoppelWeaponSwitcher::jmp_ret_1]
    }
}

naked void LuaDoppelWeaponSwitcher::init_detour_2()
{
    __asm {
        cmp rax, rax
        jmp qword ptr[LuaDoppelWeaponSwitcher::jmp_ret_2]
    }
}


std::optional<std::string> LuaDoppelWeaponSwitcher::on_initialize() {
    init_check_box_info();
    m_scripts = {"doppelweaponswitcher.lua"};//scripts here
    m_is_enabled = &LuaDoppelWeaponSwitcher::cheaton;
    m_on_page = Page_VergilDoppel;
    m_full_name_string = "Doppel Weapon Switcher";
    m_author_string = "The HitchHiker";
    m_description_string = "Assign different weapons to the Doppelganger.\n"
        "The Doppelganger will use the weapon you had equipped when you summoned it."
        "Recalling the Doppelganger using the dpad will update its weapon.\n"
        "Just Judgement Cuts can be performed at any time regardless of the weapon the"
        "Doppelganger has equipped.\n"
        "Must be enabled before entering a mission.";

    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
    auto addr_1 = m_patterns_cache->find_addr(base, "44 38 BF F0 17 00 00 75 5A");
    auto addr_2 = m_patterns_cache->find_addr(base, "80 BA F0 17 00 00 00 48 8B BF");
    if (!addr_1) {
        return "Unable to find LuaDoppelWeaponSwitcher init1 pattern";
    }

    if (!addr_2) {
        return "Unable to find LuaDoppelWeaponSwitcher init2 pattern";
    }
    install_new_detour(addr_1.value(), m_init_hook_1, &init_detour_1, &jmp_ret_1, 7);
    install_new_detour(addr_2.value(), m_init_hook_2, &init_detour_2, &jmp_ret_2, 7);
    set_up_hotkey();

    return Mod::on_initialize();
}

void LuaDoppelWeaponSwitcher::on_load_lua_mod()
{
    API::LuaLock _{};
    load_scripts();
    //running lua on load here
}

void LuaDoppelWeaponSwitcher::on_unload_lua_mod()
{
    API::LuaLock _{};
}

void LuaDoppelWeaponSwitcher::on_lua_mod_update()
{
    API::LuaLock _{};
    sol::state_view mod_state_view{ m_mod_state };
    //lua actions on update here
}



void LuaDoppelWeaponSwitcher::on_draw_ui()
{
    draw_lua_ui();
}

