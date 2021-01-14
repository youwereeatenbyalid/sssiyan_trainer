
#include "VergilAirTrickNoSS.hpp"

uintptr_t VergilAirTrickNoSS::jmp_ret{NULL};
uintptr_t VergilAirTrickNoSS::jmp_je{NULL};
uintptr_t VergilAirTrickNoSS::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        push rax
        mov rax, [VergilAirTrickNoSS::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

     cheatcode:
		jmp qword ptr [VergilAirTrickNoSS::jmp_ret]

    code:
        test ecx, ecx
        je jecode
        jmp qword ptr [VergilAirTrickNoSS::jmp_ret]

    jecode:
        jmp qword ptr [VergilAirTrickNoSS::jmp_je]
	}
}

// clang-format on

std::optional<std::string> VergilAirTrickNoSS::on_initialize() {
  ischecked            = false;
  onpage               = vergilpage;
  full_name_string     = "No Summoned Swords on Air Trick";
  author_string        = "SSSiyan";
  description_string   = "I honestly have no clue what this does yet.";
  VergilAirTrickNoSS::cheaton = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "85 C9 0F 84 BE 01 00 00 F3 0F 10 87");
  if (!addr) {
    return "Unable to find VergilAirTrickNoSS pattern.";
  }

  VergilAirTrickNoSS::jmp_je = utility::scan(base, "F3 0F 10 15 93 D6 09 04").value();

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilAirTrickNoSS";
  }
  return Mod::on_initialize();
}

void VergilAirTrickNoSS::on_draw_ui() {
}
