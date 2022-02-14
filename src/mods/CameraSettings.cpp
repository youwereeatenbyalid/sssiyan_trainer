#include "CameraSettings.hpp"

uintptr_t CameraSettings::jmp_retFoV{NULL};
uintptr_t CameraSettings::jmp_retHorizontalSensClockwise{NULL};
uintptr_t CameraSettings::jmp_retHorizontalSensAntiClockwise{NULL};

uintptr_t CameraSettings::jmp_retKeyboardHorizontalEnable{NULL};
uintptr_t CameraSettings::jmp_jeKeyboardHorizontalEnable{NULL};

uintptr_t CameraSettings::jmp_retSiyansCamFix1;
uintptr_t CameraSettings::jmp_jneSiyansCamFix1;

uintptr_t CameraSettings::jmp_retCloseAutoCorrect;
uintptr_t CameraSettings::jmp_jneCloseAutoCorrect;

uintptr_t CameraSettings::jmp_retDistantAutoCorrect;

uintptr_t CameraSettings::jmp_retMovementAutoCorrect;

uintptr_t CameraSettings::jmp_retHeightAutoCorrect;

uintptr_t CameraSettings::jmp_retNoVignette;

bool CameraSettings::cheaton{NULL};

float fov = 65.0;

float horizontalmult = 100.0;
float horizontalsens = 3.25;

bool keyboardhorizontalenable;
bool nocameraautocorrectsenable;
bool novignetteenable;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detourFoV() {
	__asm {
		cmp byte ptr [CameraSettings::cheaton], 1
        jne code
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
		cmp byte ptr [CameraSettings::cheaton], 1
        jne code
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
		cmp byte ptr [CameraSettings::cheaton], 1
        jne code
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
		cmp byte ptr [CameraSettings::cheaton], 1
        jne code
        cmp byte ptr [keyboardhorizontalenable], 1
        je jejmp
    code:
        test edx,edx
        je jejmp
        mov [rbx+00000298h], ecx
        jmp qword ptr [CameraSettings::jmp_retKeyboardHorizontalEnable]

    jejmp:
        jmp qword ptr [CameraSettings::jmp_jeKeyboardHorizontalEnable]
	}
}

static naked void detourSiyansCamFix1() {
	__asm {
		cmp byte ptr [CameraSettings::cheaton], 1
        jne code
        cmp byte ptr [nocameraautocorrectsenable], 1
        je cheatcode
    code:
        cmp qword ptr [rax+18h], 00
        jne jnejmp
    cheatcode:
        jmp qword ptr [CameraSettings::jmp_retSiyansCamFix1]

    jnejmp:
        jmp qword ptr [CameraSettings::jmp_jneSiyansCamFix1]
	}
}

static naked void detourCloseAutoCorrect() {
	__asm {
		cmp byte ptr [CameraSettings::cheaton], 1
        jne code
        cmp byte ptr [nocameraautocorrectsenable], 1
        je jnejmp
    code:
        cmp byte ptr [rbx+00000494h], 00
        jne jnejmp
        jmp qword ptr [CameraSettings::jmp_retCloseAutoCorrect]

    jnejmp:
        jmp qword ptr [CameraSettings::jmp_jneCloseAutoCorrect]
	}
}

static naked void detourDistantAutoCorrect() {
	__asm {
		cmp byte ptr [CameraSettings::cheaton], 1
        jne code
        cmp byte ptr [nocameraautocorrectsenable], 1
        je cheatcode
    code:
        mov [rdi+00000280h], eax
    cheatcode:
        jmp qword ptr [CameraSettings::jmp_retDistantAutoCorrect]
	}
}

static naked void detourHeightAutoCorrect() {
	__asm {
		cmp byte ptr [CameraSettings::cheaton], 1
        jne code
        cmp byte ptr [nocameraautocorrectsenable], 1
        je cheatcode
    code:
        movss [rdi+00000280h], xmm0
    cheatcode:
        jmp qword ptr [CameraSettings::jmp_retHeightAutoCorrect]
	}
}

static naked void detourMovementAutoCorrect() {
	__asm {
		cmp byte ptr [CameraSettings::cheaton], 1
        jne code
        cmp byte ptr [nocameraautocorrectsenable], 1
        je cheatcode
    code:
        movss [rdi+00000280h], xmm0
    cheatcode:
        jmp qword ptr [CameraSettings::jmp_retMovementAutoCorrect]
	}
}

static naked void detourNoVignette() {
    __asm {
        cmp byte ptr[CameraSettings::cheaton], 1
        jne code
        cmp byte ptr [novignetteenable], 1
        je cheatcode
    code:
        mov [rax+0x00000180], edx
        jmp qword ptr[CameraSettings::jmp_retNoVignette]

    cheatcode:
        mov edx, 2
        mov [rax+0x00000180], edx
        jmp qword ptr[CameraSettings::jmp_retNoVignette]
    }
}

// clang-format on

void CameraSettings::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> CameraSettings::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &CameraSettings::cheaton;
  m_on_page             = camera;

  m_full_name_string   = "Camera Options (+)";
  m_author_string      = "SSSiyan";
  m_description_string = "Change various camera settings.";

  set_up_hotkey();


  auto base  = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1 = patterns->find_addr(base, "F3 0F 10 57 30 48 8B D6");
  if (!addr1) {
    return "Unable to find CameraSettings1 pattern.";
  }

  auto addr2 = patterns->find_addr(base, "F3 0F 10 B2 B0 00 00 00 0F 5A F6 0F");
  if (!addr2) {
    return "Unable to find CameraSettings2 pattern.";
  }

  auto addr3 = patterns->find_addr(base, "F3 0F 10 B2 B0 00 00 00 0F 5A F6 E8");
  if (!addr3) {
    return "Unable to find CameraSettings3 pattern.";
  }

  auto addr4 = patterns->find_addr(base, "74 17 89 8B 98 02 00 00");
  if (!addr4) {
    return "Unable to find CameraSettings4 pattern.";
  }
  CameraSettings::jmp_jeKeyboardHorizontalEnable = addr4.value() + 25;

  auto addr5 = patterns->find_addr(base, "48 83 78 18 00 0F 85 5D 01 00 00 C6");
  if (!addr5) {
    return "Unable to find CameraSettings5 pattern.";
  }
  CameraSettings::jmp_jneSiyansCamFix1 = addr5.value() + 360;

  auto addr6 = patterns->find_addr(base, "80 BB 94 04 00 00 00 0F 85 6F");
  if (!addr6) {
    return "Unable to find CameraSettings6 pattern.";
  }
  CameraSettings::jmp_jneCloseAutoCorrect = addr6.value() + 380;

  auto addr7 = patterns->find_addr(base, "89 87 80 02 00 00 48 8B 43 50 48 83");
  if (!addr7) {
    return "Unable to find CameraSettings7 pattern.";
  }

  auto addr8 = patterns->find_addr(base, "F3 0F 11 87 80 02 00 00 48 8B 43 50 48 8B");
  if (!addr8) {
    return "Unable to find CameraHeightBasedAutocorrects pattern.";
  }

  auto addr9 = patterns->find_addr(base, "F3 0F 11 87 80 02 00 00 48 83");
  if (!addr9) {
    return "Unable to find CameraMovementBasedAutocorrects pattern.";
  }

  auto addr10 = patterns->find_addr(base, "89 90 80 01 00 00 C3");
  if (!addr10) {
      return "Unable to find CameraNoVignette pattern.";
  }

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
  if (!install_hook_absolute(addr5.value(), m_function_hookSiyansCamFix1, &detourSiyansCamFix1, &jmp_retSiyansCamFix1, 11)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize CameraSettings5";
  }
  if (!install_hook_absolute(addr6.value(), m_function_hookCloseAutoCorrect, &detourCloseAutoCorrect, &jmp_retCloseAutoCorrect, 13)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize CameraSettings6";
  }
  if (!install_hook_absolute(addr7.value(), m_function_hookDistantAutoCorrect, &detourDistantAutoCorrect, &jmp_retDistantAutoCorrect, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize CameraSettings7";
  }
  if (!install_hook_absolute(addr8.value(), m_function_hookHeightAutoCorrect, &detourHeightAutoCorrect, &jmp_retHeightAutoCorrect, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize CameraSettings8";
  }
  if (!install_hook_absolute(addr9.value(), m_function_hookMovementAutoCorrect, &detourMovementAutoCorrect, &jmp_retMovementAutoCorrect, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize CameraSettings9";
  }
  if (!install_hook_absolute(addr10.value(), m_function_hookNoVignette, &detourNoVignette, &jmp_retNoVignette, 6)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize CameraSettingsNoVignette";
  }

  return Mod::on_initialize();
}
void CameraSettings::on_config_load(const utility::Config& cfg) {
  nocameraautocorrectsenable = cfg.get<bool>("camera_settings_siyans_cam_fix_1").value_or(false);
  keyboardhorizontalenable = cfg.get<bool>("camera_settings_keyboard_horizontal").value_or(false);
  fov = cfg.get<float>("camera_settings_fov").value_or(65.0f);
  horizontalsens = cfg.get<float>("camera_settings_horizontal_sens").value_or(3.25f);
  novignetteenable = cfg.get<bool>("camera_settings_no_vignette").value_or(false);
}

void CameraSettings::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("camera_settings_siyans_cam_fix_1", nocameraautocorrectsenable);
  cfg.set<bool>("camera_settings_keyboard_horizontal", keyboardhorizontalenable);
  cfg.set<float>("camera_settings_fov", fov);
  cfg.set<float>("camera_settings_horizontal_sens", horizontalsens);
  cfg.set<bool>("camera_settings_no_vignette", novignetteenable);
}

void CameraSettings::on_draw_ui() {
  ImGui::Checkbox("Prefer panning to rotating", &nocameraautocorrectsenable);
  ImGui::ShowHelpMarker("The game automatically spins the camera to keep the enemy in view. This will attempt to avoid that by panning instead.");
  ImGui::Checkbox("Disable Vignette", &novignetteenable);
  ImGui::Checkbox("Allow Camera Movement While Locked On Using Keyboard", &keyboardhorizontalenable);
  ImGui::Text("Field of View (65 default)");
  UI::SliderFloat("##fovslider", &fov, 0.0f, 120.0f, "%.0f");
  ImGui::Text("Horizontal Sensitivity (3.25 default)");
  UI::SliderFloat("##horizontalsensslider", &horizontalsens, 3.25f, 10.0f, "%.2f");
}
