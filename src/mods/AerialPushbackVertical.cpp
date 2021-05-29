#include "AerialPushbackVertical.hpp"
#include "PlayerTracker.hpp"
uintptr_t AerialPushbackVertical::jmp_ret{NULL};
bool AerialPushbackVertical::cheaton{NULL};
float verticalPushResolveTest = 0.97f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() { // speed at which the character returns to 0 vertical inertia
	__asm {
        cmp byte ptr [AerialPushbackVertical::cheaton], 1
        jne code
        cmp [PlayerTracker::playerid], 1
        je cheatcode
        jmp code

        cheatcode:
        cmp dword ptr [PlayerTracker::playermoveid], 157C012Ch // rainstorm
        jne code
        mulss xmm14, [verticalPushResolveTest]
        jmp code

        code:
        movss [rdi+24h], xmm14
        jmp qword ptr [AerialPushbackVertical::jmp_ret]
	}
}

// clang-format on

void AerialPushbackVertical::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> AerialPushbackVertical::on_initialize() {
  init_check_box_info();

  ischecked = &AerialPushbackVertical::cheaton;
  onpage    = mechanics;

  full_name_string   = "Rainstorm pop";
  author_string      = "SSSiyan";
  description_string = "Dev test.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr =
      utility::scan(base, "F3 44 0F 11 77 24 F3 44 0F 11 7F"); // DevilMayCry5.exe+144566A Vergil exe
  if (!addr) {
    return "Unable to find AerialPushbackVertical pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize AerialPushbackVertical";
  }
  return Mod::on_initialize();
}

void AerialPushbackVertical::on_draw_ui() {}
