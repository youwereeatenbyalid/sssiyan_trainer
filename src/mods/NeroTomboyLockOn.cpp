
#include "NeroTomboyLockOn.hpp"
#include "PlayerTracker.hpp"
uintptr_t NeroTomboyLockOn::jmp_ret{NULL};
uintptr_t NeroTomboyLockOn::jmp_jne{NULL};
bool NeroTomboyLockOn::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [NeroTomboyLockOn::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        jmp retcode

    code:
        test ecx, ecx
        jne jnejmp
    retcode:
        mov rdx, rbx
        jmp qword ptr [NeroTomboyLockOn::jmp_ret]

    jnejmp:
        jmp qword ptr [NeroTomboyLockOn::jmp_jne]
	}
}

// clang-format on

std::optional<std::string> NeroTomboyLockOn::on_initialize() {
  ischecked          = &NeroTomboyLockOn::cheaton;
  onpage             = neropage;

  full_name_string   = "Lock On With Tomboy";
  author_string      = "SSSiyan";
  description_string        = "Allows you to lock on while using Tomboy.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "20 85 C9 75 15 48 8B D3");
  if (!addr) {
    return "Unable to find NeroTomboyLockOn pattern.";
  }

  if (!install_hook_absolute(addr.value()+1, m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroTomboyLockOn";
  }

  NeroTomboyLockOn::jmp_jne = addr.value() + 26;

  return Mod::on_initialize();
}

void NeroTomboyLockOn::on_draw_ui() {}
