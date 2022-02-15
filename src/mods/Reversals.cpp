#include "Reversals.hpp"

uintptr_t Reversals::jmp_ret{NULL};
uintptr_t Reversals::jmp_ret2{ NULL };
bool Reversals::cheaton{NULL};

bool demoReversals;
bool bufferedReversals;
bool bufferlessReversals;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() { // demo reversals
	__asm {
		cmp byte ptr [Reversals::cheaton], 1
		je cheatcode
		jmp code

	cheatcode:
		cmp byte ptr [demoReversals], 1
		jne code
		jmp qword ptr [Reversals::jmp_ret]

	code:
		cmp byte ptr [rdi+00000ED0h],00
		jmp qword ptr [Reversals::jmp_ret]
	}
}

static naked void detour2() { // modded  reversals
	__asm {
		cmp byte ptr [Reversals::cheaton], 1
		je cheatcode
		jmp code

	cheatcode:
		cmp byte ptr [bufferlessReversals], 1
		je retcode
		cmp byte ptr [bufferedReversals], 1
		jne code
		add [rdi+0x1740], rax
		jmp qword ptr [Reversals::jmp_ret2]

	code:
		mov [rdi+0x1740], rax
	retcode:
		jmp qword ptr [Reversals::jmp_ret2]
	}
}

// clang-format on

void Reversals::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> Reversals::on_initialize() {
  init_check_box_info();

  
  m_is_enabled               = &Reversals::cheaton;
  m_on_page                  = mechanics;
  m_full_name_string         = "Reversals (+)";
  m_author_string            = "SSSiyan, Nekupaska, socks";
  m_description_string		 = "Allows you to use directional moves in any direction.\n\n"
							   "During the recovery of a move, buffer a directional attack, then "
							   "push the stick in a new direction and let go of lock on before the "
							   "buffered attack comes out to change the direction it points.";
  set_up_hotkey();
							   
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "80 BF D0 0E 00 00 00 8B");
  if (!addr) {
    return "Unable to find Reversals pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize Reversals";
  }

  auto addr2 = patterns->find_addr(base, "48 89 87 40 17 00 00");
  if (!addr2) {
    return "Unable to find BufferedReversals pattern.";
  }

  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize BufferedReversals";
  }

  return Mod::on_initialize();
}

void Reversals::on_config_load(const utility::Config& cfg) {
	demoReversals = cfg.get<bool>("demo_reversals").value_or(true);
	bufferedReversals = cfg.get<bool>("buffered_reversals").value_or(false);
	bufferlessReversals = cfg.get<bool>("bufferless_reversals").value_or(false);
}

void Reversals::on_config_save(utility::Config& cfg) {
	cfg.set<bool>("demo_reversals", demoReversals);
	cfg.set<bool>("buffered_reversals", bufferedReversals);
	cfg.set<bool>("bufferless_reversals", bufferlessReversals);
}

void Reversals::on_draw_ui() {
	ImGui::Text("When using only this checkbox, reversals will work identically to how they did in the demo.");
	ImGui::Checkbox("Demo Reversals", &demoReversals);
	ImGui::Separator();
	ImGui::Text("The following checkboxes are for our old modded reversals, kept for legacy's sake.\nBoth have their advantages and can even be stacked.");
	if (ImGui::Checkbox("Modded Reversals", &bufferedReversals) == true) {
		bufferlessReversals = false;
	}
	if (ImGui::Checkbox("Bufferless Reversals", &bufferlessReversals) == true) {
		bufferedReversals = false;
	}
}
