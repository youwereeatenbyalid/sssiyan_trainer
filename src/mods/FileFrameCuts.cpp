
#include "FileFrameCuts.hpp"

uintptr_t FileFrameCuts::jmp_ret{NULL};
uintptr_t FileFrameCuts::cheaton{NULL};
bool dantefasterguard;

float danteguardgroundstartlength = 1.0f;
float danteairguardstartlength    = 3.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        push rax
        mov rax, [FileFrameCuts::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        cmp dword ptr [rdx+9Ah], 7274604 //'lo' (Block)
        je guardgroundstartcheck
        cmp dword ptr [rdx+9Ah], 7471209 //'ir' (AirBlock)
        je guardairstartcheck
        jmp code

    guardgroundstartcheck:
        cmp byte ptr [rdx+0xA4], 7602259 //'St' (Start)
        jne code
        movss xmm0, [danteguardgroundstartlength]
        jmp qword ptr [FileFrameCuts::jmp_ret]

    guardairstartcheck:
        cmp byte ptr [rdx+0xAC], 6357108 //'ta' (StartFly)
        jne code
        cmp byte ptr [dantefasterguard],0
        je code
        movss xmm0, [danteairguardstartlength]
        jmp qword ptr [FileFrameCuts::jmp_ret]

    code:
        movss xmm0,[rdx+58h]
		jmp qword ptr [FileFrameCuts::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> FileFrameCuts::on_initialize() {
  ischecked            = false;
  onpage               = gamepage;
  full_name_string     = "Frame Cuts (+)";
  author_string        = "SSSiyan";
  description_string   = "Cuts frames from various moves.";
  FileFrameCuts::cheaton = (uintptr_t)&ischecked;
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
}
void FileFrameCuts::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("dante_faster_guard", dantefasterguard);
}

void FileFrameCuts::on_draw_ui() {
  ImGui::Checkbox("Faster Guard", &dantefasterguard);
}
