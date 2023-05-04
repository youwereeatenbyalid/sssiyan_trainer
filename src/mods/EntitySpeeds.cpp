#include "EntitySpeeds.hpp"
#include "PlayerTracker.hpp"

uintptr_t EntitySpeeds::jmp_ret{NULL};
bool EntitySpeeds::cheaton{NULL};

bool dantemillionstabspeedup;
bool danteskystarspeedup;
bool nerosnatchspeedup;

float dantemillionstabstartspeed = 3.0f;
float danteskystarspeed          = 1.5f;
float nerosnatchspeed            = 1.5f;
float nerosnatchpullspeed        = 2.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [EntitySpeeds::cheaton], 1
        jne code
        //jmp playercheck

    playercheck:
        push r14
        mov r14, [rsi + 0x58]//gameModel nullptr check
        test r14, r14
        je regpop
        cmp dword ptr [r14 + 0x108], 1 //player or enemy
        je regpop
        cmp dword ptr [r14 + 0xE64], 0 //plId
        je nerospeeds
        cmp dword ptr [r14 + 0xE64], 1
        je dantespeeds
        jmp regpop

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
        jmp regpop

    dantespeeds:
    // million stab startup
        cmp dword ptr [PlayerTracker::playermoveid], 00DC00C8h // dante ms start reb
        je dantemstab
        cmp dword ptr [PlayerTracker::playermoveid], 00D200FAh // dante ms start sparda
        je dantemstab
        cmp dword ptr [PlayerTracker::playermoveid], 00DC00F0h // dante ms start dsd
        je dantemstab
        cmp dword ptr [PlayerTracker::playermoveid], 157C01F4h // dante sky star
        je danteskystar
        jmp regpop

// nero mults
    nerosnatchp1:
        cmp byte ptr [nerosnatchspeedup], 1
        jne regpop
        mulss xmm2, [nerosnatchspeed]
        jmp regpop
    nerosnatchp2:
        cmp byte ptr [nerosnatchspeedup], 1
        jne regpop
        mulss xmm2, [nerosnatchpullspeed]
        jmp regpop

// dante mults
    dantemstab:
        cmp byte ptr [dantemillionstabspeedup], 1
        jne regpop
        mulss xmm2, [dantemillionstabstartspeed]
        jmp regpop

    danteskystar:
        cmp byte ptr [danteskystarspeedup], 1
        jne regpop
        mulss xmm2, [danteskystarspeed]
        jmp regpop

// code
    regpop:
    pop r14

    code:
        movss [rsi+0x000000B4], xmm2
		jmp qword ptr [EntitySpeeds::jmp_ret]
	}
}

// clang-format on

void EntitySpeeds::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> EntitySpeeds::on_initialize() {
  init_check_box_info();

  m_is_enabled         = &EntitySpeeds::cheaton;
  m_on_page            = Page_Animation;
  m_depends_on         = { "PlayerTracker" };
  m_full_name_string   = "Faster Moves (+)";
  m_author_string      = "SSSiyan";
  m_description_string = "Adjust the speed of various moves.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = m_patterns_cache->find_addr(base, "F3 0F 11 96 B4 00 00 00 48");
  if (!addr) {
    return "Unable to find EntitySpeeds pattern.";
  }

  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EntitySpeeds";
  }
  return Mod::on_initialize();
}

void EntitySpeeds::on_config_load(const utility::Config& cfg) {
  nerosnatchspeedup = cfg.get<bool>("nero_snatch_speedup").value_or(false);
  dantemillionstabspeedup = cfg.get<bool>("dante_million_stab_startup_speedup").value_or(false);
  danteskystarspeedup = cfg.get<bool>("dante_sky_star_speedup").value_or(false);
}

void EntitySpeeds::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("nero_snatch_speedup", nerosnatchspeedup);
  cfg.set<bool>("dante_million_stab_startup_speedup", dantemillionstabspeedup);
  cfg.set<bool>("dante_sky_star_speedup", danteskystarspeedup);
}

void EntitySpeeds::on_draw_ui() {
  ImGui::Text("Nero");
  ImGui::Checkbox("Snatch Speedup", &nerosnatchspeedup);
  ImGui::Separator();
  ImGui::Text("Dante");
  ImGui::Checkbox("Million Stab Startup Speedup", &dantemillionstabspeedup);
  ImGui::Checkbox("Sky Star Speedup", &danteskystarspeedup);
}


#if 0
#include "EntitySpeeds.hpp"
#include "PlayerTracker.hpp"

uintptr_t EntitySpeeds::jmp_ret{ NULL };
bool EntitySpeeds::cheaton{ NULL };

bool dantemillionstabspeedup;
bool danteskystarspeedup;
bool nerosnatchspeedup;

float dantemillionstabstartspeed = 3.0f;
float danteskystarspeed = 1.5f;
float nerosnatchspeed = 1.5f;
float nerosnatchpullspeed = 2.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
    __asm {
        cmp byte ptr[EntitySpeeds::cheaton], 1
        jne code
        jmp playercheck

        playercheck :
        cmp[PlayerTracker::playerid], 0
            je nerocheck
            cmp[PlayerTracker::playerid], 1
            je dantecheck
            jmp code

            nerocheck :
        cmp dword ptr[rcx - 40h], 7078000    // 'pl' (play)
            je nerospeeds
            jmp code

            dantecheck :
        cmp byte ptr[rcx + 144h], 40h        // Hitch prbably has better compares
            je dantespeeds
            cmp dword ptr[rcx - 342h], 7077954   // 'Bl' (Bloood)
            je dantespeeds
            jmp code

            nerospeeds :
        // snatch
        cmp dword ptr[PlayerTracker::playermoveid], 1F410064h // Ground Snatch (Mid)
            je nerosnatchp1
            cmp dword ptr[PlayerTracker::playermoveid], 1F450064h // Ground Snatch (Low)
            je nerosnatchp1
            cmp dword ptr[PlayerTracker::playermoveid], 1F460064h // Ground Snatch (High)
            je nerosnatchp1
            cmp dword ptr[PlayerTracker::playermoveid], 0BB80046h // Air Snatch (Mid)
            je nerosnatchp1
            cmp dword ptr[PlayerTracker::playermoveid], 0BBA0046h // Air Snatch (Low)
            je nerosnatchp1
            cmp dword ptr[PlayerTracker::playermoveid], 0BB90046h // Air Snatch (High)
            je nerosnatchp1
            // snatch pull
            cmp dword ptr[PlayerTracker::playermoveid], 0BC20046h // Air Pull (Mid)
            je nerosnatchp2
            cmp dword ptr[PlayerTracker::playermoveid], 0BC40046h // Air Pull (Low)
            je nerosnatchp2
            cmp dword ptr[PlayerTracker::playermoveid], 0BC30046h // Air Pull (High)
            je nerosnatchp2
            cmp dword ptr[PlayerTracker::playermoveid], 07DA0046h // Ground Pull (Mid)
            je nerosnatchp2
            cmp dword ptr[PlayerTracker::playermoveid], 07DC0046h // Ground Pull (Low)
            je nerosnatchp2
            cmp dword ptr[PlayerTracker::playermoveid], 07DB0046h // Ground Pull (High)
            je nerosnatchp2
            jmp code

            dantespeeds :
        // million stab startup
        cmp dword ptr[PlayerTracker::playermoveid], 00DC00C8h // dante ms start reb
            je dantemstab
            cmp dword ptr[PlayerTracker::playermoveid], 00D200FAh // dante ms start sparda
            je dantemstab
            cmp dword ptr[PlayerTracker::playermoveid], 00DC00F0h // dante ms start dsd
            je dantemstab
            cmp dword ptr[PlayerTracker::playermoveid], 157C01F4h // dante sky star
            je danteskystar
            jmp code

            // nero mults
            nerosnatchp1 :
        cmp byte ptr[nerosnatchspeedup], 1
            jne code
            mulss xmm0, [nerosnatchspeed]
            jmp code
            nerosnatchp2 :
        cmp byte ptr[nerosnatchspeedup], 1
            jne code
            mulss xmm0, [nerosnatchpullspeed]
            jmp code

            // dante mults
            dantemstab :
        cmp byte ptr[dantemillionstabspeedup], 1
            jne code
            mulss xmm0, [dantemillionstabstartspeed]
            jmp code

            danteskystar :
        cmp byte ptr[danteskystarspeedup], 1
            jne code
            mulss xmm0, [danteskystarspeed]
            jmp code

            // code
            code :
        movss[rcx + 4Ch], xmm0
            jmp qword ptr[EntitySpeeds::jmp_ret]
    }
}

// clang-format on

void EntitySpeeds::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> EntitySpeeds::on_initialize() {
    init_check_box_info();

    m_is_enabled = &EntitySpeeds::cheaton;
    m_on_page = Page_Animation;
    m_full_name_string = "Faster Moves (+)";
    m_author_string = "SSSiyan";
    m_description_string = "Adjust the speed of various moves.";

    set_up_hotkey();

    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
    auto addr = m_patterns_cache->find_addr(base, "F3 0F 11 41 4C 48 8B 5C");
    if (!addr) {
        return "Unable to find EntitySpeeds pattern.";
    }

    if (!i/nstall_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize EntitySpeeds";
    }
    return Mod::on_initialize();
}

void EntitySpeeds::on_config_load(const utility::Config& cfg) {
    nerosnatchspeedup = cfg.get<bool>("nero_snatch_speedup").value_or(false);
    dantemillionstabspeedup = cfg.get<bool>("dante_million_stab_startup_speedup").value_or(false);
    danteskystarspeedup = cfg.get<bool>("dante_sky_star_speedup").value_or(false);
}

void EntitySpeeds::on_config_save(utility::Config& cfg) {
    cfg.set<bool>("nero_snatch_speedup", nerosnatchspeedup);
    cfg.set<bool>("dante_million_stab_startup_speedup", dantemillionstabspeedup);
    cfg.set<bool>("dante_sky_star_speedup", danteskystarspeedup);
}

void EntitySpeeds::on_draw_ui() {
    ImGui::Text("Nero");
    ImGui::Checkbox("Snatch Speedup", &nerosnatchspeedup);
    ImGui::Separator();
    ImGui::Text("Dante");
    ImGui::Checkbox("Million Stab Startup Speedup", &dantemillionstabspeedup);
    ImGui::Checkbox("Sky Star Speedup", &danteskystarspeedup);
}
#endif