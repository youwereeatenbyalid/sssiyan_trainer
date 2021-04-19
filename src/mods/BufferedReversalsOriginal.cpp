#include "BufferedReversalsOriginal.hpp"

uintptr_t OriginalReversals::jmp_ret{NULL};
bool OriginalReversals::cheaton{NULL};


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
		cmp byte ptr [OriginalReversals::cheaton], 1
		je cheatcode
		jmp code

	cheatcode:
		jmp qword ptr [OriginalReversals::jmp_ret]

	code:
		cmp byte ptr [rdi+00000ED0h],00
		jmp qword ptr [OriginalReversals::jmp_ret]
	}
}

// clang-format on

void OriginalReversals::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> OriginalReversals::on_initialize() {
  init_check_box_info();

  
  ischecked                  = &OriginalReversals::cheaton;
  onpage                     = mechanics;
  full_name_string           = "Original Reversals";
  author_string              = "SSSiyan";
  description_string		 = "Allows you to use directional moves in any direction.\n\n"
							   "During the recovery of a move, buffer a directional attack, then "
							   "push the stick in a new direction and let go of lock on before the "
							   "buffered attack comes out to change the direction it points.\n\n"
							   "When using only this checkbox, reversals will work identically to how they did in the demo.\n\n"
							   "This is added as a separate checkbox so you can stack it with the old reversals as there may be some unique possiblities.";
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "80 BF D0 0E 00 00 00 8B");
  if (!addr) {
    return "Unable to find OriginalReversals pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize OriginalReversals";
  }
  return Mod::on_initialize();
}

void OriginalReversals::on_draw_ui() {
}
