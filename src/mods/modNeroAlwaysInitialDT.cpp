#include "modNeroAlwaysInitialDT.hpp"
bool NeroAlwaysInitialDT::cheaton{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry



// clang-format on

void NeroAlwaysInitialDT::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroAlwaysInitialDT::on_initialize() {
  init_check_box_info();

  ischecked = &NeroAlwaysInitialDT::cheaton;
  onpage    = nero;

  full_name_string   = "Nero always uses initial DT anim";
  author_string      = "SSSiyan";
  description_string = "\"Fuck you\" only plays the first time you use this in M20.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "0F 85 34 01 00 00 80 BF C2 18 00 00 00"); // DevilMayCry5.exe+2104DE9 Vergil exe
  if (!addr) {
    return "Unable to find NeroAlwaysInitialDT pattern.";
  }

  m_patch01 = Patch::create_nop(addr.value(), 6, false);
  m_patch02 = Patch::create_nop(addr.value()+13, 6, false);
  return Mod::on_initialize();
}

void NeroAlwaysInitialDT::on_draw_ui() {}
