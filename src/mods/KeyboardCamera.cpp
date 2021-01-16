#include "KeyboardCamera.hpp"
#include "PlayerTracker.hpp"
uintptr_t KeyboardCamera::jmp_ret{NULL};
uintptr_t KeyboardCamera::jmp_je{NULL};
uintptr_t KeyboardCamera::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        push rax
        mov rax, [KeyboardCamera::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [KeyboardCamera::jmp_je]

    code:
        test edx,edx
        je jejmp
        mov [rbx+00000298h], ecx
        jmp qword ptr [KeyboardCamera::jmp_ret]

    jejmp:
        jmp qword ptr [KeyboardCamera::jmp_je]
	}
}

// clang-format on

std::optional<std::string> KeyboardCamera::on_initialize() {
  ischecked          = false;
  onpage             = commonpage;
  full_name_string   = "Allow Keyboard Camera Movement While Locked On";
  author_string      = "SSSiyan";
  description_string = "Allows you to move the camera horizontally while locked on when using keyboard.";
  KeyboardCamera::cheaton = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = utility::scan(base, "74 17 89 8B 98 02 00 00");
  if (!addr) {
    return "Unable to find KeyboardCamera pattern.";
  }

  KeyboardCamera::jmp_je = addr.value() + 25;

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize KeyboardCamera";
  }
  return Mod::on_initialize();
}

void KeyboardCamera::on_draw_ui() {}
