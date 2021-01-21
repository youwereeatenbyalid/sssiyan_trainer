#include "CameraSettings.hpp"

uintptr_t CameraSettings::jmp_retFoV{NULL};
uintptr_t CameraSettings::jmp_retHorizontalSensClockwise{NULL};
uintptr_t CameraSettings::jmp_retHorizontalSensAntiClockwise{NULL};

uintptr_t CameraSettings::jmp_retKeyboardHorizontalEnable{NULL};
uintptr_t CameraSettings::jmp_jeKeyboardHorizontalEnable{NULL};

uintptr_t CameraSettings::cheaton{NULL};

float fov = 65.0;

float horizontalmult = 100.0;
float horizontalsens = 3.25;
bool keyboardhorizontalenable;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detourFoV() {
	__asm {
        push rax
        mov rax, [CameraSettings::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        movss xmm2, [fov]
        jmp qword ptr [CameraSettings::jmp_retFoV]

    code:
        movss xmm2, [rdi+30h]
        jmp qword ptr [CameraSettings::jmp_retFoV]
	}
}

static naked void detourHorizontalSensClockwise() {
	__asm {
        push rax
        mov rax, [CameraSettings::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        movss xmm6, [horizontalsens]
        divss xmm6, [horizontalmult]
		jmp retcode

    code:
        movss xmm6, [rdx+000000B0h]
    retcode:
		jmp qword ptr [CameraSettings::jmp_retHorizontalSensClockwise]
	}
}

static naked void detourHorizontalSensAntiClockwise() {
	__asm {
        push rax
        mov rax, [CameraSettings::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        movss xmm6, [horizontalsens]
        divss xmm6, [horizontalmult]
		jmp retcode

    code:
        movss xmm6, [rdx+000000B0h]
    retcode:
		jmp qword ptr [CameraSettings::jmp_retHorizontalSensAntiClockwise]
	}
}

static naked void detourKeyboardHorizontalEnable() {
	__asm {
        push rax
        mov rax, [CameraSettings::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [keyboardhorizontalenable], 1
        jne code
		jmp qword ptr [CameraSettings::jmp_jeKeyboardHorizontalEnable]

    code:
        test edx,edx
        je jejmp
        mov [rbx+00000298h], ecx
        jmp qword ptr [CameraSettings::jmp_retKeyboardHorizontalEnable]

    jejmp:
        jmp qword ptr [CameraSettings::jmp_jeKeyboardHorizontalEnable]
	}
}

// clang-format on

std::optional<std::string> CameraSettings::on_initialize() {
  ischecked          = false;
  onpage             = commonpage;
  full_name_string   = "Camera Settings (+)";
  author_string      = "SSSiyan";
  description_string = "Change various camera settings.";
  CameraSettings::cheaton = (uintptr_t)&ischecked;

  auto base  = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1 = utility::scan(base, "F3 0F 10 57 30 48 8B D6");
  if (!addr1) {
    return "Unable to find CameraSettings1 pattern.";
  }
  auto addr2 = utility::scan(base, "F3 0F 10 B2 B0 00 00 00 0F 5A F6 0F");
  if (!addr2) {
    return "Unable to find CameraSettings2 pattern.";
  }
  auto addr3 = utility::scan(base, "F3 0F 10 B2 B0 00 00 00 0F 5A F6 E8");
  if (!addr3) {
    return "Unable to find CameraSettings3 pattern.";
  }
  auto addr4 = utility::scan(base, "74 17 89 8B 98 02 00 00");
  if (!addr4) {
    return "Unable to find CameraSettings4 pattern.";
  }
  CameraSettings::jmp_jeKeyboardHorizontalEnable = addr4.value() + 25;

  if (!install_hook_absolute(addr1.value(), m_function_hookFoV, &detourFoV, &jmp_retFoV, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize CameraSettings1";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hookHorizontalSensClockwise, &detourHorizontalSensClockwise, &jmp_retHorizontalSensClockwise, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize CameraSettings2";
  }
  if (!install_hook_absolute(addr3.value(), m_function_hookHorizontalSensAntiClockwise, &detourHorizontalSensAntiClockwise, &jmp_retHorizontalSensAntiClockwise, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize CameraSettings3";
  }
  if (!install_hook_absolute(addr4.value(), m_function_hookKeyboardHorizontalEnable, &detourKeyboardHorizontalEnable, &jmp_retKeyboardHorizontalEnable, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize CameraSettings4";
  }
  return Mod::on_initialize();
}
void CameraSettings::on_config_load(const utility::Config& cfg) {
  keyboardhorizontalenable = cfg.get<bool>("camera_settings_keyboard_horizontal").value_or(true);
  fov = cfg.get<float>("camera_settings_fov").value_or(90.0f);
  horizontalsens = cfg.get<float>("camera_settings_horizontal_sens").value_or(4.00f);
}

void CameraSettings::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("camera_settings_keyboard_horizontal", keyboardhorizontalenable);
  cfg.set<float>("camera_settings_fov", fov);
  cfg.set<float>("camera_settings_horizontal_sens", horizontalsens);
}

void CameraSettings::on_draw_ui() {
  ImGui::Checkbox("Allow Keyboard Camera Movement While Locked On", &keyboardhorizontalenable);
  ImGui::Text("Field of View (65 default)");
  ImGui::SliderFloat("##fovslider", &fov, 0.0f, 120.0f, "%.0f");
  ImGui::Text("Horizontal Sensitivity (3.25 default)");
  ImGui::SliderFloat("##horizontalsensslider", &horizontalsens, 3.25f, 10.0f, "%.2f");
}
