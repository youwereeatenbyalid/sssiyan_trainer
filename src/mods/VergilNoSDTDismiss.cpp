#include "VergilNoSDTDismiss.hpp"
#include "mods/PlayerTracker.hpp"

uintptr_t VergilNoSDTDismiss::jmp_ret{NULL};
bool VergilNoSDTDismiss::cheaton{NULL};
uintptr_t VergilNoSDTDismiss::jmp_je{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry
static naked void detour() {
	__asm {
	    validation:
            cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
            jne code
            cmp byte ptr [VergilNoSDTDismiss::cheaton], 1
            je cheatcode
            jmp code

        cheatcode:
            jmp doppeloff

        code:
            cmp byte ptr [rdi+0x000018A8],00
            je doppeloff
            jmp qword ptr [VergilNoSDTDismiss::jmp_ret]    

        doppeloff:
            jmp qword ptr [VergilNoSDTDismiss::jmp_je]
	}
}
// clang-format on

void VergilNoSDTDismiss::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilNoSDTDismiss::on_initialize() {
  init_check_box_info();

   auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
   ischecked = &VergilNoSDTDismiss::cheaton;
   onpage               = vergildoppel;
   full_name_string     = "No SDT Doppelganger Dismiss";
   author_string        = "Dr.penguin";
   description_string   = "Disables doppelganger despawn when deactivating SDT.";

   auto addr = utility::scan(base, "80 BF A8 18 00 00 00 74 1D");
   if (!addr) {
    return "Unable to find VergilNoSDTDismiss pattern.";
  }
    if (!install_hook_absolute(addr.value(), m_function_hook, &detour,&jmp_ret, 9)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilNoSDTDismiss";
  }

  VergilNoSDTDismiss::jmp_je = addr.value() + 38;

  return Mod::on_initialize();
}

// during load
// void VergilNoSDTDismiss::on_config_load(const utility::Config &cfg) {}
// during save
// void VergilNoSDTDismiss::on_config_save(utility::Config &cfg) {}
// do something every frame
// void VergilNoSDTDismiss::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void VergilNoSDTDismiss::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void VergilNoSDTDismiss::on_draw_ui() {}
