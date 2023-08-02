#include "HoldToMash.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t HoldToMash::jmp_return{NULL};
bool HoldToMash::cheaton{NULL};
//variables
// clang-format off
// only in clang/icl mode on x64, sorry


//Issue with this right now is the renda check on the rising dragon is a single button, making it way too easy to get dragon punch.
//actual field is app.RendaChecker.<count>k__BackingField at 0x14, set to 0.
//Easiest solution is just requiring the hold button be held a few extra frames before copying to press and starting the recka,
//but that might mess with other stuff. 
//Also, I want to redo the mash stuff for ebony and ivory, so it might be worth redoing this as lua or with SDK.
  static naked void newmem_detour() {
__asm {
  validation:
    //cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
    //jne code
    cmp byte ptr [HoldToMash::cheaton], 1
    je cheatcode
    jmp code
  code:
    test [r8+0x48], eax
    seta al
    jmp qword ptr [HoldToMash::jmp_return]

  cheatcode:
    test [r8+0x3C], eax
    seta al
    jmp qword ptr [HoldToMash::jmp_return]
  }
}

// clang-format on

void HoldToMash::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> HoldToMash::on_initialize() {
  init_check_box_info();

  auto base              = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled           = &HoldToMash::cheaton;
  m_on_page              = Page_QOL;
  //m_depends_on           = { "PlayerTracker" };
  m_scripts = { "HoldToMash.lua" };//scripts here
  m_full_name_string     = "Hold To Mash";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Hold button for inputs like twosometime, million stab, rising dragon, etc.";

  set_up_hotkey();
  //auto holdtomash_addr = m_patterns_cache->find_addr(base, "41 85 40 48 0F 97 C0");
  //
  //if (!holdtomash_addr) {
  //  return "Unable to find holdtomash pattern.";
  //}
  //if (!install_new_detour(holdtomash_addr.value(), m_holdtomash_detour, &newmem_detour, &jmp_return, 7)) {
  //  //return a error string in case something goes wrong
  //  spdlog::error("[{}] failed to initialize", get_name());
  //  return "Failed to initialize holdtomash";
  //}

  return Mod::on_initialize();
}


void HoldToMash::on_load_lua_mod()
{
    API::LuaLock _{};
    load_scripts();
    //running lua on load here
}

void HoldToMash::on_unload_lua_mod()
{
    API::LuaLock _{};
}

void HoldToMash::on_lua_mod_update()
{
    API::LuaLock _{};
    sol::state_view mod_state_view{ m_mod_state };
    //lua actions on update here
}



void HoldToMash::on_draw_ui()
{
    draw_lua_ui();
}



// during load
// void HoldToMash::on_config_load(const utility::Config &cfg) {}
// during save
// void HoldToMash::on_config_save(utility::Config &cfg) {}
// do something every frame
//void HoldToMash::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void HoldToMash::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void HoldToMash::on_draw_ui() {}