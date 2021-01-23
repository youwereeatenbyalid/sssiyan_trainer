
#include "EntitySpeeds.hpp"
#include "PlayerTracker.hpp"

uintptr_t EntitySpeeds::jmp_ret{NULL};
bool EntitySpeeds::cheaton{NULL};
bool dantemillionstabspeedup;
bool nerosnatchspeedup;

float dantemillionstabstartspeed = 3.0f;
float nerosnatchspeed            = 1.5f;
float nerosnatchpullspeed        = 2.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [EntitySpeeds::cheaton],1
        jne code
        jmp playercheck

    playercheck:
        cmp [PlayerTracker::playerid], 0
        je nerocheck
        cmp [PlayerTracker::playerid], 1
        je dantecheck
        jmp code

    nerocheck:
        cmp dword ptr [rcx-40h], 7078000    // 'pl' (play)
        je nerospeeds
        jmp code

    dantecheck:
        cmp byte ptr [rcx+144h], 40h        // Hitch prbably has better compares
        je dantespeeds
        cmp dword ptr [rcx-342h], 7077954   // 'Bl' (Bloood)
        je dantespeeds
        jmp code

    nerospeeds:
    // snatch
        cmp dword ptr [PlayerTracker::playermoveid], 1F410064h // Ground Snatch (Mid)
        je nerosnatchp1
        cmp dword ptr [PlayerTracker::playermoveid], 1F450064h // Ground Snatch (Low)
        je nerosnatchp1
        cmp dword ptr [PlayerTracker::playermoveid], 1F460064h // Ground Snatch (High)
        je nerosnatchp1
        cmp dword ptr [PlayerTracker::playermoveid], 0BB80046h // Air Snatch (Mid)
        je nerosnatchp1
        cmp dword ptr [PlayerTracker::playermoveid], 0BBA0046h // Air Snatch (Low)
        je nerosnatchp1
        cmp dword ptr [PlayerTracker::playermoveid], 0BB90046h // Air Snatch (High)
        je nerosnatchp1
    // snatch pull
        cmp dword ptr [PlayerTracker::playermoveid], 0BC20046h // Air Pull (Mid)
        je nerosnatchp2
        cmp dword ptr [PlayerTracker::playermoveid], 0BC40046h // Air Pull (Low)
        je nerosnatchp2
        cmp dword ptr [PlayerTracker::playermoveid], 0BC30046h // Air Pull (High)
        je nerosnatchp2
        cmp dword ptr [PlayerTracker::playermoveid], 07DA0046h // Ground Pull (Mid)
        je nerosnatchp2
        cmp dword ptr [PlayerTracker::playermoveid], 07DC0046h // Ground Pull (Low)
        je nerosnatchp2
        cmp dword ptr [PlayerTracker::playermoveid], 07DB0046h // Ground Pull (High)
        je nerosnatchp2
        jmp code

    dantespeeds:
    // million stab startup
        cmp dword ptr [PlayerTracker::playermoveid], 00DC00C8h // dante ms start reb
        je dantemstab
        cmp dword ptr [PlayerTracker::playermoveid], 00D200FAh // dante ms start sparda
        je dantemstab
        cmp dword ptr [PlayerTracker::playermoveid], 00DC00F0h // dante ms start dsd
        je dantemstab
        jmp code

// nero mults
    nerosnatchp1:
        cmp byte ptr [nerosnatchspeedup], 1
        jne code
        mulss xmm0, [nerosnatchspeed]
        jmp code
    nerosnatchp2:
        cmp byte ptr [nerosnatchspeedup], 1
        jne code
        mulss xmm0, [nerosnatchpullspeed]
        jmp code

// dante mults
    dantemstab:
        cmp byte ptr [dantemillionstabspeedup], 1
        jne code
        mulss xmm0, [dantemillionstabstartspeed]
        jmp code

// code
    code:
        movss [rcx+4Ch], xmm0
		jmp qword ptr [EntitySpeeds::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> EntitySpeeds::on_initialize() {
  ischecked          = &EntitySpeeds::cheaton;
  onpage             = gamepage;
  full_name_string   = "Entity Speeds (+)";
  author_string      = "SSSiyan";
  description_string = "Adjust the speed of various moves.";

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
  nerosnatchspeedup = cfg.get<bool>("nero_snatch_speedup").value_or(true);
  dantemillionstabspeedup = cfg.get<bool>("dante_million_stab_startup_speedup").value_or(true);
}

void EntitySpeeds::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("nero_snatch_speedup", nerosnatchspeedup);
  cfg.set<bool>("dante_million_stab_startup_speedup", dantemillionstabspeedup);
}

void EntitySpeeds::on_draw_ui() {
  ImGui::Text("Nero");
  ImGui::Checkbox("Nero Snatch Speed Up", &nerosnatchspeedup);
  ImGui::Separator();
  ImGui::Text("Dante");
  ImGui::Checkbox("Dante Million Stab Startup Speed Up", &dantemillionstabspeedup);
}
