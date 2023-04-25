
#include "FileFrameCuts.hpp"
#include "PlayerTracker.hpp"
uintptr_t FileFrameCuts::jmp_ret{NULL};
bool FileFrameCuts::cheaton{NULL};
bool dantefasterguard;
bool dantefasterhatgatling;

float danteguardgroundstartlength = 1.0f;
float danteairguardstartlength    = 5.0f;
float dantegatlingstartlength     = 1.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() { // DevilMayCry5.exe+294AA6D 
	__asm {
        cmp byte ptr [FileFrameCuts::cheaton], 1
        jne code
        cmp [PlayerTracker::playerid],1
        je dantecuts
        jmp code

    dantecuts:
        push r11

        mov r11, 27866499442802754 // 'B.l.o.c.' (Block)
        cmp r11, [rdx+98h]
        je guardgroundstartcheck

        mov r11, 18577838096056385 // 'A.i.r.B.' (AirBlock)
        cmp r11, [rdx+92h]
        je guardairstartcheck

        mov r11, 30399795707314247 // G.a.t.l. (Ground)
        cmp r11, [rdx+9Ah]
        je hatturretstartcheck

        mov r11, 30399795707314247 // G.a.t.l. (Air)
        cmp r11, [rdx+98h] 
        je hatturretairstartcheck

        jmp popcode

    guardgroundstartcheck:
        mov r11, 32088563964444755 //'S.t.a.r.' (Start)
        cmp r11, [rdx+0xA4]
        jne popcode
        cmp byte ptr [dantefasterguard], 1
        jne popcode
        movss xmm0, [danteguardgroundstartlength]
        jmp popret

    guardairstartcheck:
        mov r11, 32088563964444755 //'S.t.a.r.' (StartFly)
        cmp r11, [rdx+0xAA]
        jne popcode
        cmp byte ptr [dantefasterguard], 1
        jne popcode
        movss xmm0, [danteairguardstartlength]
        jmp popret

    hatturretstartcheck:
        mov r11, 32088563964444755 // 'S.t.a.r.' (Start)
        cmp r11, [rdx+0xAA]
        jne popcode
        cmp byte ptr [dantefasterhatgatling], 1
        jne popcode
        cmp byte ptr [rdx],1 // TEST PLS REMOVE
        movss xmm0, [dantegatlingstartlength]
        jmp popret

    hatturretairstartcheck:
        mov r11, 32088563964444755 // 'S.t.a.r.' (Start)
        cmp r11, [rdx+0xA8]
        jne popcode
        cmp byte ptr [dantefasterhatgatling], 1
        jne popcode
        cmp byte ptr [rdx],1 // TEST PLS REMOVE
        movss xmm0, [dantegatlingstartlength]
        jmp popret

    popret:
        pop r11
        jmp qword ptr [FileFrameCuts::jmp_ret]

    popcode:
        pop r11
    code:
        movss xmm0, [rdx+58h]
		jmp qword ptr [FileFrameCuts::jmp_ret]
	}
}

// clang-format on

void FileFrameCuts::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> FileFrameCuts::on_initialize() {
  init_check_box_info();

  m_is_enabled           = &FileFrameCuts::cheaton;
  m_on_page              = Page_Animation;
  m_depends_on           = { "PlayerTracker" };
  m_full_name_string     = "Faster Move Startups (+)";
  m_author_string        = "SSSiyan";
  m_description_string   = "Cuts frames from various moves.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = m_patterns_cache->find_addr(base, "F3 0F 10 42 58 66 85 C0 74 0E");
  if (!addr) {
    return "Unable to find FileFrameCuts pattern.";
  }

  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize FileFrameCuts";
  }
  return Mod::on_initialize();
}

void FileFrameCuts::on_config_load(const utility::Config& cfg) {
  dantefasterguard = cfg.get<bool>("dante_faster_guard").value_or(true);
  dantefasterhatgatling = cfg.get<bool>("dante_faster_hat_gatling").value_or(true);
}
void FileFrameCuts::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("dante_faster_guard", dantefasterguard);
  cfg.set<bool>("dante_faster_hat_gatling", dantefasterhatgatling);
}

void FileFrameCuts::on_draw_ui() {
  ImGui::Text("Dante");
  ImGui::Checkbox("Faster Guard", &dantefasterguard);
  ImGui::Checkbox("Faster Faust Gatling", &dantefasterhatgatling);
}
