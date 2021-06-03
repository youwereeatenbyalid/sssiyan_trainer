#include "AerialPushbackVertical.hpp"
#include "PlayerTracker.hpp"
uintptr_t AerialPushbackVertical::jmp_ret{NULL};
bool AerialPushbackVertical::cheaton{NULL};

bool shouldRainstormRise{NULL};
bool shouldBlasterRise{NULL};

//float verticalPushResolveTest = 0.97f;

float rainstormFloat = 0.97f;
float blasterFloat = 0.80f;


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
        je rainstorm
        cmp dword ptr [PlayerTracker::playermoveid], 157C0141h // blaster
        je blaster
        jmp code

    rainstorm:
        cmp byte ptr [shouldRainstormRise], 1
        jne code
        mulss xmm14, [rainstormFloat]
        jmp code

    blaster:
        cmp byte ptr [shouldBlasterRise], 1
        jne code
        mulss xmm14, [blasterFloat]
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
  onpage    = dantecheat;

  full_name_string   = "Move Height Edits";
  author_string      = "SSSiyan";
  description_string = "Your character will rise more when performing certain moves.";

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

void AerialPushbackVertical::on_config_load(const utility::Config& cfg) {
  shouldRainstormRise = cfg.get<bool>("rainstorm_rise").value_or(true);
  shouldBlasterRise = cfg.get<bool>("blaster_rise").value_or(true);
}
void AerialPushbackVertical::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("rainstorm_rise", shouldRainstormRise);
  cfg.set<bool>("blaster_rise", shouldBlasterRise);
}

void AerialPushbackVertical::on_draw_ui() {
  ImGui::Checkbox("Rainstorm", &shouldRainstormRise);
  ImGui::Checkbox("Blaster (KA Air Gunslinger)", &shouldBlasterRise);
}
