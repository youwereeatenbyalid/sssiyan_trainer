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

bool CameraSettings::cheaton{NULL};

float fov = 65.0;

float horizontalmult = 100.0;
float horizontalsens = 3.25;

bool keyboardhorizontalenable;
bool siyanscamerafixenable;
bool closeautocorrectenable;
bool distantautocorrectenable;
bool movementautocorrectenable;
bool heightautocorrectenable;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detourFoV() {
	__asm {
		cmp byte ptr [CameraSettings::cheaton], 1
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
		cmp byte ptr [CameraSettings::cheaton], 1
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

		cmp byte ptr [CameraSettings::cheaton], 1
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
		cmp byte ptr [CameraSettings::cheaton], 1
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

static naked void detourSiyansCamFix1() {
	__asm {
		cmp byte ptr [CameraSettings::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [siyanscamerafixenable], 1
        jne code
		jmp qword ptr [CameraSettings::jmp_retSiyansCamFix1]

    code:
        cmp qword ptr [rax+18h], 00
        jne jnejmp
        jmp qword ptr [CameraSettings::jmp_retSiyansCamFix1]

    jnejmp:
        jmp qword ptr [CameraSettings::jmp_jneSiyansCamFix1]
	}
}

static naked void detourCloseAutoCorrect() {
	__asm {
		cmp byte ptr [CameraSettings::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [closeautocorrectenable], 1
        jne code
		jmp jnejmp

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
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [distantautocorrectenable], 1
        jne code
		jmp qword ptr [CameraSettings::jmp_retDistantAutoCorrect]

    code:
        mov [rdi+00000280h], eax
        jmp qword ptr [CameraSettings::jmp_retDistantAutoCorrect]
	}
}

static naked void detourHeightAutoCorrect() {
	__asm {
		cmp byte ptr [CameraSettings::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [heightautocorrectenable], 1
        jne code
		jmp qword ptr [CameraSettings::jmp_retHeightAutoCorrect]

    code:
        movss [rdi+00000280h], xmm0
        jmp qword ptr [CameraSettings::jmp_retHeightAutoCorrect]
	}
}

static naked void detourMovementAutoCorrect() {
	__asm {
		cmp byte ptr [CameraSettings::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [movementautocorrectenable], 1
        jne code
		jmp qword ptr [CameraSettings::jmp_retMovementAutoCorrect]

    code:
        movss [rdi+00000280h], xmm0
        jmp qword ptr [CameraSettings::jmp_retMovementAutoCorrect]
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

  m_full_name_string   = "Camera Settings (+)";
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

  return Mod::on_initialize();
}
void CameraSettings::on_config_load(const utility::Config& cfg) {
  siyanscamerafixenable = cfg.get<bool>("camera_settings_siyans_cam_fix_1").value_or(false);
  closeautocorrectenable = cfg.get<bool>("camera_settings_close_auto_correct").value_or(false);
  distantautocorrectenable = cfg.get<bool>("camera_settings_distant_auto_correct").value_or(false);
  heightautocorrectenable = cfg.get<bool>("camera_settings_height_auto_correct").value_or(false);
  movementautocorrectenable = cfg.get<bool>("camera_settings_movement_auto_correct").value_or(false);
  keyboardhorizontalenable = cfg.get<bool>("camera_settings_keyboard_horizontal").value_or(false);
  fov = cfg.get<float>("camera_settings_fov").value_or(65.0f);
  horizontalsens = cfg.get<float>("camera_settings_horizontal_sens").value_or(3.25f);
}

void CameraSettings::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("camera_settings_siyans_cam_fix_1", siyanscamerafixenable);
  cfg.set<bool>("camera_settings_close_auto_correct", closeautocorrectenable);
  cfg.set<bool>("camera_settings_distant_auto_correct", distantautocorrectenable);
  cfg.set<bool>("camera_settings_height_auto_correct", heightautocorrectenable);
  cfg.set<bool>("camera_settings_movement_auto_correct", movementautocorrectenable);
  cfg.set<bool>("camera_settings_keyboard_horizontal", keyboardhorizontalenable);
  cfg.set<float>("camera_settings_fov", fov);
  cfg.set<float>("camera_settings_horizontal_sens", horizontalsens);
}

void CameraSettings::on_draw_ui() {
  ImGui::Checkbox("Siyan's Camera Fix (Old)", &siyanscamerafixenable);
  ImGui::ShowHelpMarker("Old version of the camera settings. Its functionality has been split into the cheats below");
  ImGui::Checkbox("Disable Close Autocorrects", &closeautocorrectenable);
  ImGui::ShowHelpMarker("Autocorrects are when the game automatically spins the camera to keep the enemy in view. They are replaced here with the camera panning out instead.");
  ImGui::Checkbox("Disable Distant Autocorrects", &distantautocorrectenable);
  ImGui::ShowHelpMarker("Autocorrects are when the game automatically spins the camera to keep the enemy in view. They are replaced here with the camera panning out instead.");
  ImGui::Checkbox("Disable Height Based Autocorrects", &heightautocorrectenable);
  ImGui::ShowHelpMarker("Autocorrects are when the game automatically spins the camera to keep the enemy in view. They are replaced here with the camera panning out instead.");
  ImGui::Checkbox("Disable Movement Based Autocorrects", &movementautocorrectenable);
  ImGui::ShowHelpMarker("Autocorrects are when the game automatically spins the camera to keep the enemy in view. They are replaced here with the camera panning out instead.");
  ImGui::Checkbox("Allow Keyboard Camera Movement While Locked On", &keyboardhorizontalenable);
  ImGui::Spacing();
  ImGui::Text("Field of View (65 default)");
  UI::SliderFloat("##fovslider", &fov, 0.0f, 120.0f, "%.0f");
  ImGui::Spacing();
  ImGui::Text("Horizontal Sensitivity (3.25 default)");
  UI::SliderFloat("##horizontalsensslider", &horizontalsens, 3.25f, 10.0f, "%.2f");
}
