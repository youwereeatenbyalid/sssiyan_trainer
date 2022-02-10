#include "PetChargeNoInterrupt.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t PetChargeNoInterrupt::jmp_return{NULL};
uintptr_t PetChargeNoInterrupt::je_return{NULL};
bool PetChargeNoInterrupt::cheaton{NULL};
//variables
// clang-format off
// only in clang/icl mode on x64, sorry

  static naked void newmem_detour() {
__asm {
  validation:
    cmp [PlayerTracker::playerid], 2 //change this to the char number obviously
    jne code
    cmp byte ptr [PetChargeNoInterrupt::cheaton], 1
    je cheatcode
    jmp code
  code:
    test ecx,ecx
    je je_return
    mov rdx,rdi
    jmp qword ptr [PetChargeNoInterrupt::jmp_return]

  cheatcode:
  je_return:
	jmp qword ptr [PetChargeNoInterrupt::je_return]

  }
}


// clang-format on

void PetChargeNoInterrupt::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> PetChargeNoInterrupt::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &PetChargeNoInterrupt::cheaton;
  m_on_page    = gilver;
  m_full_name_string     = "Pet charge carries through interrupt ";
  m_author_string        = "The Hitchhiker";
  m_description_string   = "Griffon's charges will continue through a backstep or air hike. Shadow's hedgehog will continue to charge through forced movement";

  set_up_hotkey();
  
  auto INJECT_addr = patterns->find_addr(base, "74 10 48 8B D7 48 8B CB E8 C3");
  PetChargeNoInterrupt::je_return = INJECT_addr.value()+0x12;
  if (!INJECT_addr) {
    return "Unable to find INJECT pattern.";
  }
  if (!install_hook_absolute(INJECT_addr.value(), m_INJECT_hook, &newmem_detour, &jmp_return, 5)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize INJECT";
  }

  return Mod::on_initialize();
}

// during load
void PetChargeNoInterrupt::on_config_load(const utility::Config &cfg) {}
// during save
void PetChargeNoInterrupt::on_config_save(utility::Config &cfg) {}
// do something every frame
void PetChargeNoInterrupt::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void PetChargeNoInterrupt::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void PetChargeNoInterrupt::on_draw_ui() {}
