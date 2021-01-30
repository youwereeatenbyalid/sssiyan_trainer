
#include "FileFrameCuts.hpp"

uintptr_t FileFrameCuts::jmp_ret{NULL};
bool FileFrameCuts::cheaton{NULL};
bool dantefasterguard;
bool dantefasterhatgatling;

float danteguardgroundstartlength = 1.0f;
float danteairguardstartlength    = 5.0f;
float dantegatlingstartlength     = 1.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [FileFrameCuts::cheaton], 1
        jne code
        cmp dword ptr [rdx+9Ah], 7274604 // 'lo' (Block)
        je guardgroundstartcheck
        cmp dword ptr [rdx+9Ah], 7471209 // 'ir' (AirBlock)
        je guardairstartcheck
        cmp dword ptr [rdx+9Eh], 7078004 // 'tl' (Gatling)
        je hatturretstartcheck
        cmp dword ptr [rdx+9Ch], 7078004 // 'tl' (Gatling)
        je hatturretairstartcheck
        jmp code

    guardgroundstartcheck:
        cmp dword ptr [rdx+0xA4], 7602259 //'St' (Start)
        jne code
        cmp byte ptr [dantefasterguard], 1
        jne code
        movss xmm0, [danteguardgroundstartlength]
        jmp qword ptr [FileFrameCuts::jmp_ret]

    guardairstartcheck:
        cmp dword ptr [rdx+0xAC], 6357108 //'ta' (StartFly)
        jne code
        cmp byte ptr [dantefasterguard], 1
        jne code
        movss xmm0, [danteairguardstartlength]
        jmp qword ptr [FileFrameCuts::jmp_ret]

    hatturretstartcheck:
        cmp dword ptr [rdx+0xAC], 6357108 // 'ta' (Start)
        jne code
        cmp byte ptr [dantefasterhatgatling], 1
        jne code
        movss xmm0, [dantegatlingstartlength]
        jmp qword ptr [FileFrameCuts::jmp_ret]

    hatturretairstartcheck:
        cmp dword ptr [rdx+0xAA], 6357108 // 'ta' (Start)
        jne code
        cmp byte ptr [dantefasterhatgatling], 1
        jne code
        movss xmm0, [dantegatlingstartlength]
        jmp qword ptr [FileFrameCuts::jmp_ret]

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

  ischecked            = &FileFrameCuts::cheaton;
  onpage               = animation;

  full_name_string     = "Frame Cuts (+)";
  author_string        = "SSSiyan";
  description_string   = "Cuts frames from various moves.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "F3 0F 10 42 58 66 85 C0 74 0E");
  if (!addr) {
    return "Unable to find FileFrameCuts pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
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
  ImGui::Checkbox("Faster Guard", &dantefasterguard);
  ImGui::Checkbox("Faster Faust Gatling", &dantefasterhatgatling);
}
