#include "VergilSDTTrickEfx.hpp"
#include "VergilSDTFormTracker.hpp"

bool VergilSDTTrickEfx::cheaton{false};

uintptr_t VergilSDTTrickEfx::retJmp{NULL};
uintptr_t VergilSDTTrickEfx::retJe{NULL};
uintptr_t VergilSDTTrickEfx::yamatoBehaviorRet{NULL};
uintptr_t VergilSDTTrickEfx::yamatoBehaviorJne{NULL};


static void trickefx_detour() {
	__asm {
		cmp byte ptr [VergilSDTTrickEfx::cheaton], 0
		je originalcode

		cheat:
		jmp qword ptr [VergilSDTTrickEfx::retJe]

		originalcode:
		cmp eax, 0x2
		je ret_je
        //mov eax, [rsi+0x000009B0]
		jmp qword ptr [VergilSDTTrickEfx::retJmp]

		ret_je:
		jmp qword ptr [VergilSDTTrickEfx::retJe]
	}
}

static void yamato_behavior_detour() {
	__asm {
		cmp byte ptr [VergilSDTTrickEfx::cheaton], 0x1
		je cheat

		originalcode:
		cmp qword ptr [rax+0x18], 00
		jne ret_je
		jmp qword ptr [VergilSDTTrickEfx::yamatoBehaviorRet]

		ret_je:
		jmp qword ptr [VergilSDTTrickEfx::yamatoBehaviorJne]

		cheat:
		cmp dword ptr [VergilSDTFormTracker::vergilform_state], 0x2
		je originalcode
		jmp qword ptr [VergilSDTTrickEfx::yamatoBehaviorJne]
  }
}

std::optional<std::string> VergilSDTTrickEfx::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = vergilefxsettings;
	m_full_name_string = "SDT Tricks EFX";
	m_author_string = "VPZadov";
	m_description_string = "Always use SDT Tricks EFX and SDT yamato sheath flash EFX.";

  set_up_hotkey();

	auto sdtEfxAddr = utility::scan(base, "83 F8 02 74 06 41 83 CE"); //DevilMayCry5.exe+58991C      //utility::scan(base, "8B 86 B0 09 00 00 4C"); //DevilMayCry5.exe+1DDCB5D
    if (!sdtEfxAddr) {
          return "Unanable to find VergilSDTTrickEfx.sdtEfxAddr pattern.";
    }

	auto yamatoBehaviorAddr = utility::scan(base, "48 83 78 18 00 75 18 8B 47 74"); //DevilMayCry5.exe+15822AD
    if (!yamatoBehaviorAddr) {
          return "Unanable to find VergilSDTTrickEfx.yamatoBehaviorAddr pattern.";
    }

	yamatoBehaviorJne = yamatoBehaviorAddr.value() + 0x1F;
    retJe             = sdtEfxAddr.value() + 0xB;

	if (!install_hook_absolute(sdtEfxAddr.value(), m_forcesdttrickefx_hook, &trickefx_detour, &retJmp, 0x5)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilSDTTrickEfx.sdtEfx";
	}

	if (!install_hook_absolute(yamatoBehaviorAddr.value(), m_yamatobehavior_hook, &yamato_behavior_detour, &yamatoBehaviorRet, 0x7)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilSDTTrickEfx.yamatoBehavior";
	}

    return Mod::on_initialize();
}

void VergilSDTTrickEfx::on_config_load(const utility::Config& cfg)
{
}

void VergilSDTTrickEfx::on_config_save(utility::Config& cfg)
{
}

void VergilSDTTrickEfx::on_frame()
{
}

void VergilSDTTrickEfx::on_draw_ui()
{
}

void VergilSDTTrickEfx::on_draw_debug_ui()
{
}

void VergilSDTTrickEfx::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}
