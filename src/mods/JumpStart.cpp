#include "JumpStart.hpp"
#include "AllStart.hpp"
#include "PlayerTracker.hpp"

uintptr_t JumpStart::jmp_ret{NULL};
bool JumpStart::cheaton{NULL};

bool nerodtcancel;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [JumpStart::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp [PlayerTracker::playerid], 0
        je nerocancels
        cmp [PlayerTracker::playerid], 1
        je code
        cmp [PlayerTracker::playerid], 2
        je code
        cmp [PlayerTracker::playerid], 4
        je code
        jmp code

    nerocancels:
        cmp dword ptr [PlayerTracker::playermoveid], 0x00000A // Nero Ground DT
        je nerodtcheck
        cmp dword ptr [PlayerTracker::playermoveid], 0x64000A // Nero Air DT
        je nerodtcheck
        jmp code

    nerodtcheck:
        cmp byte ptr [nerodtcancel], 1
        je cancellable
        jmp code

    cancellable:
        mov word ptr [rdi+58h], 0100h
		jmp qword ptr [JumpStart::jmp_ret]

    code:
        mov word ptr [rdi+58h], 0000h
        jmp qword ptr [JumpStart::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> JumpStart::on_initialize() {
  ischecked          = &JumpStart::cheaton ;
  onpage             = enemystep;

  full_name_string   = "JumpStart";
  author_string      = "SSSiyan";
  description_string = "Allows you to cancel out of a selection of moves with any jump action.";

  // auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  // auto addr  = AllStart::jmp_initial - 6;
  // if (!addr) {
  //   return "Unable to find JumpStart pattern.";
  // }

  if (!install_hook_absolute(AllStart::jmp_initial-6, m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize JumpStart";
  }
  return Mod::on_initialize();
}

void JumpStart::on_config_load(const utility::Config& cfg) {
  nerodtcancel = cfg.get<bool>("nero_dt_cancel").value_or(true);
}
void JumpStart::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("nero_dt_cancel", nerodtcancel);
}

void JumpStart::on_draw_ui() {
  ImGui::Checkbox("Nero DT Cancel", &nerodtcancel);
}
