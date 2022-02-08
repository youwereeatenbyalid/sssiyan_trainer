#include "BufferedReversals.hpp"

uintptr_t BufferedReversals::jmp_ret{NULL};
bool BufferedReversals::cheaton{NULL};
bool BufferedReversals::bufferless{NULL};


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
		cmp byte ptr [BufferedReversals::cheaton], 1
		je cheatcode
		jmp code

	cheatcode:
		cmp byte ptr [BufferedReversals::bufferless],1
		je retcode
		add [rdi+0x1740], rax
		jmp qword ptr [BufferedReversals::jmp_ret]

	code:
		mov [rdi+0x1740], rax
	retcode:
		jmp qword ptr [BufferedReversals::jmp_ret]
	}
}

// clang-format on

void BufferedReversals::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> BufferedReversals::on_initialize() {
  init_check_box_info();

  
  ischecked                  = &BufferedReversals::cheaton;
  onpage                     = mechanics;
  full_name_string           = "Reversals (+)";
  author_string              = "Nekupaska, socks";
  description_string		 = "Allows you to use directional moves in any direction.\n\n"
							   "During the recovery of a move, buffer a directional attack, then "
							   "push the stick in a new direction and let go of lock on before the "
							   "buffered attack comes out to change the direction it points.\n\n"
							   "if 'Bufferless Reversals' is unticked, you will get buffered reversals instead.";
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "48 89 87 40 17 00 00");
  if (!addr) {
    return "Unable to find BufferedReversals pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize BufferedReversals";
  }
  return Mod::on_initialize();
}

void BufferedReversals::on_config_load(const utility::Config& cfg) {
  bufferless = cfg.get<bool>("bufferless_reversals").value_or(false);
}

void BufferedReversals::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("bufferless_reversals", bufferless);
}

void BufferedReversals::on_draw_ui() {
  ImGui::Checkbox("Bufferless Reversals", &bufferless);
}
