
#include "VergilDirectionalMappings.hpp"
#include "utility/Scan.hpp"

uintptr_t VergilDirectionalMappings::jmp_ret{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [rdi+10h], 4
        je lockonback
        jmp code

    lockonback:
        mov edx, 1000h
        jmp raxret

    code:
        mov edx, [rdi+10h]
        // jmp raxret

    raxret:
        test rax,rax
		jmp qword ptr [VergilDirectionalMappings::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> VergilDirectionalMappings::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "8B 57 10 48 85 C0 0F 84 67");
  if (!addr) {
    return "Unable to find VergilDirectionalMappings pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilDirectionalMappings";
  }
  return Mod::on_initialize();
}

// during load
// void MoveID::on_config_load(const utility::Config &cfg) {}
// during save
// void MoveID::on_config_save(utility::Config &cfg) {}
// do something every frame
// void MoveID::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void DeepTurbo::on_draw_debug_ui() {
// ImGui::Text("Deep Turbo : %.0f", turbospeed);
// }
// will show up in main window, dump ImGui widgets you want here
// void MoveID::on_draw_ui() {}
