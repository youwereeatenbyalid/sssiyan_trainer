
#include "EntitySpeeds.hpp"
#include "MoveID.hpp"

uintptr_t EntitySpeeds::jmp_ret{NULL};
uintptr_t EntitySpeeds::cheaton{NULL};
bool dantemillionstabspeedup;

float dantemillionstabstartspeed = 3.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        push rax
        mov rax, [EntitySpeeds::cheaton]
        cmp byte ptr [rax],1
        pop rax
        jne code
        cmp byte ptr [rcx+144h], 40h // Hitch prbably has better compares
        je movecheck
        cmp byte ptr [rcx-342h], 7077954 // 'Blood'
        je movecheck
        jmp code

    movecheck:
    part1:
        cmp dword ptr [MoveID::playermoveid], 00DC00C8h // dante ms start reb
        je dantemstab
        cmp dword ptr [MoveID::playermoveid], 00D200FAh // dante ms start sparda
        je dantemstab
        cmp dword ptr [MoveID::playermoveid], 00DC00F0h // dante ms start dsd
        je dantemstab
        jmp code

    dantemstab:
        cmp byte ptr [dantemillionstabspeedup], 0
        je code
        mulss xmm0, [dantemillionstabstartspeed]
        jmp code

    code:
        movss [rcx+4Ch], xmm0
		jmp qword ptr [EntitySpeeds::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> EntitySpeeds::on_initialize() {
  ischecked          = false;
  onpage             = commonpage;
  full_name_string   = "Entity Speeds";
  author_string      = "SSSiyan";
  description_string = "Adjust the speed of various moves.";
  EntitySpeeds::cheaton = (uintptr_t)&ischecked;

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "F3 0F 11 41 4C 48 8B 5C");
  if (!addr) {
    return "Unable to find EntitySpeeds pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EntitySpeeds";
  }
  return Mod::on_initialize();
}

void EntitySpeeds::on_config_load(const utility::Config& cfg) {
  dantemillionstabspeedup = cfg.get<bool>("dante_million_stab_startup_speedup").value_or(false);
}
void EntitySpeeds::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("dante_million_stab_startup_speedup", dantemillionstabspeedup);
}

void EntitySpeeds::on_draw_ui() {
  ImGui::Checkbox("Dante Million Stab Startup Speed Up", &dantemillionstabspeedup);
  ImGui::Text("Test");
  ImGui::Spacing();
  ImGui::Text("Test after spacing");
  ImGui::Text("Test without spacing");
  ImGui::Text("Test");
  ImGui::SameLine(200);
  ImGui::Text("Sameline");
  ImGui::Separator();
  ImGui::Text("Test");
}
