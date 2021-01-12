#include "EnemyNumber.hpp"

// clang-format off
// only in clang/icl mode on x64, sorry
uintptr_t EnemyNumber::jmp_ret{NULL};
uint32_t EnemyNumber::number{0};
uint32_t EnemyNumber::hardlimit{30};
uint32_t EnemyNumber::softlimit{20};
uint32_t EnemyNumber::limittype{0};
static naked void detour() {
	__asm {
		originalcode:
		mov eax,[rax+0x70]
		mov [rdi+0x00000750],eax
		mov [EnemyNumber::number], eax


		hardcheck:
		cmp eax, [EnemyNumber::hardlimit]
		jb softcheck

		mov [EnemyNumber::limittype], 2
		jmp exit

		softcheck:
		cmp eax, [EnemyNumber::softlimit]
		jb belowsoftcapacity

		mov [EnemyNumber::limittype], 1
		jmp exit

		belowsoftcapacity:
		mov [EnemyNumber::limittype], 0
		exit:
		jmp qword ptr[EnemyNumber::jmp_ret]
	}
}
// clang-format on

std::optional<std::string> EnemyNumber::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "8B 40 70 89 87 50 07 00 00");
  if (!addr) {
    return "Unable to find EnemyNumber pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 9)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemyNumber";
  }
  return Mod::on_initialize();
}
// during load
void EnemyNumber::on_config_load(const utility::Config &cfg) {}
// during save
void EnemyNumber::on_config_save(utility::Config &cfg) {}
// do something every frame
//void EnemyNumber::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void EnemyNumber::on_draw_debug_ui() {
  ImGui::Text("Enemy Limit type is currently %X", EnemyNumber::limittype);
  ImGui::Text("Hard limit is: %X", EnemyNumber::hardlimit);
  ImGui::Text("Soft limit is: %X", EnemyNumber::softlimit);
  ImGui::Selectable("Select me?? IDK lol", true);
}

// will show up in main window, dump ImGui widgets you want here
void EnemyNumber::on_draw_ui() {
  ImGui::SliderInt("Enemy Hard Limit", (int*)&EnemyNumber::hardlimit, 1, 50);
  ImGui::SliderInt("Enemy Soft Limit", (int*)&EnemyNumber::softlimit, 1, 50);
}
