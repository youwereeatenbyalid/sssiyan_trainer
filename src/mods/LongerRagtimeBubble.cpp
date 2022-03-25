#include "LongerRagtimeBubble.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t LongerRagtimeBubble::jmp_ragtimebreakeradjust_return{NULL};
bool LongerRagtimeBubble::cheaton{NULL};
//variables
float LongerRagtimeBubble::decrementamount{0.50};
// clang-format off
// only in clang/icl mode on x64, sorry

  static naked void newmem_detour() {
__asm {
  validation:
    cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
    jne code
    cmp byte ptr [LongerRagtimeBubble::cheaton], 1
    je cheatcode
    jmp code
  code:
    mov rcx,rbx
    cvtps2pd xmm1, xmm1
    jmp qword ptr [LongerRagtimeBubble::jmp_ragtimebreakeradjust_return]

  cheatcode:
    movss xmm14, [LongerRagtimeBubble::decrementamount]
    subss xmm1, xmm14
    movss xmm14, xmm15
    movss [rdx+0x00000334], xmm1
    mov rcx, rbx
    cvtps2pd xmm1, xmm1
    jmp qword ptr [LongerRagtimeBubble::jmp_ragtimebreakeradjust_return]
  }
}


// clang-format on
void LongerRagtimeBubble::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> LongerRagtimeBubble::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &LongerRagtimeBubble::cheaton;
  m_on_page = Page_Nero;
  m_full_name_string     = "Longer Ragtime Bubbles";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Nero's Ragtime Bubbles will last longer.";

  set_up_hotkey();

  auto ragtimebreakeradjust_addr = utility::scan(base, "48 8B CB 0F 5A C9 0F 5A C6");

  if (!ragtimebreakeradjust_addr) {
    return "Unable to find ragtimebreakeradjust pattern.";
  }
  if (!install_hook_absolute(ragtimebreakeradjust_addr.value(), m_ragtimebreakeradjust_hook, &newmem_detour, &jmp_ragtimebreakeradjust_return, 6)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize ragtimebreakeradjust";
  }
  return Mod::on_initialize();
}

// during load
void LongerRagtimeBubble::on_config_load(const utility::Config &cfg) {
    decrementamount = cfg.get<float>("additional_ragtime_decrement_amount").value_or(0.50);
}
// during save
void LongerRagtimeBubble::on_config_save(utility::Config &cfg) {
    cfg.set<float>("additional_ragtime_decrement_amount", decrementamount);
}
// do something every frame
//void LongerRagtimeBubble::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void LongerRagtimeBubble::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void LongerRagtimeBubble::on_draw_ui() {
    UI::SliderFloat("Additional Time: 0 is vanilla, 1 is infinite", &decrementamount, 0, 1, "%.2f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
}