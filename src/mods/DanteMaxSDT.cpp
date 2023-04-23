#include "DanteMaxSDT.hpp"
#include "PlayerTracker.hpp"
#include "DanteSDTRework.hpp"
uintptr_t DanteMaxSDT::jmp_ret{NULL};
bool DanteMaxSDT::cheaton{NULL};

    // clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
    validation:
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code
        cmp byte ptr [DanteSDTRework::cheaton], 1
        je code
        cmp byte ptr [DanteMaxSDT::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		mov dword ptr [rdi+0x00001A14], 0x461c4000 // 10,000
        movss xmm1, [rdi+0x00001A14]
		jmp qword ptr [DanteMaxSDT::jmp_ret]

    code:
        movss xmm1,[rdi+0x00001A14]
        jmp qword ptr [DanteMaxSDT::jmp_ret]
	}
}

// clang-format on

void DanteMaxSDT::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteMaxSDT::on_initialize() {
  init_check_box_info();

  m_is_enabled           = &DanteMaxSDT::cheaton;
  m_on_page              = Page_DanteSDT;
  m_depends_on           = { "PlayerTracker" };
  m_full_name_string     = "Infinite SDT";
  m_author_string        = "SSSiyan";
  m_description_string   = "Freezes the SDT Bar at maximum.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = m_patterns_cache->find_addr(base, "F3 0F 10 8F 14 1A 00 00 BA");
  if (!addr) {
    return "Unable to find DanteMaxSDT pattern.";
  }

  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteMaxSDT";
  }
  return Mod::on_initialize();
}

// void DanteMaxSDT::on_draw_ui() {
  //ImGui::Checkbox("Dante Max SDT", &dantemaxsdtcheck);
// }
