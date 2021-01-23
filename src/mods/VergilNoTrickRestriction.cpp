#include "VergilNoTrickRestriction.hpp"
#include "PlayerTracker.hpp"
uintptr_t VergilNoTrickRestriction::jmp_ret{NULL};
bool VergilNoTrickRestriction::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [VergilNoTrickRestriction::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        mov byte ptr [rcx+000000A8h], 00
        cmp byte ptr [rcx+000000A8h], 00
		jmp qword ptr [VergilNoTrickRestriction::jmp_ret]

    code:
        cmp byte ptr [rcx+000000A8h], 00
        jmp qword ptr [VergilNoTrickRestriction::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> VergilNoTrickRestriction::on_initialize() {
  ischecked          = &VergilNoTrickRestriction::cheaton;
  onpage             = vergilpage;

  full_name_string   = "No Trick Restriction";
  author_string      = "SSSiyan";
  description_string = "Allows you to trick during things like World of V startup.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = utility::scan(base, "80 B9 A8 00 00 00 00 0F");
  if (!addr) {
    return "Unable to find VergilNoTrickRestriction pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilNoTrickRestriction";
  }
  return Mod::on_initialize();
}

void VergilNoTrickRestriction::on_draw_ui() {}
