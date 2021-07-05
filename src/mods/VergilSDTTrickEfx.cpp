#include "VergilSDTTrickEfx.hpp"

bool VergilSDTTrickEfx::cheaton{false};

uintptr_t VergilSDTTrickEfx::retJmp{NULL};


static void trickefx_detour() {
	__asm {
		cmp byte ptr [VergilSDTTrickEfx::cheaton], 0
		je originalcode

		cheat:
		mov eax, 0x2
		jmp qword ptr [VergilSDTTrickEfx::retJmp]

		originalcode:
		mov eax, [rsi+0x000009B0]
		jmp qword ptr [VergilSDTTrickEfx::retJmp]
	}
}

std::optional<std::string> VergilSDTTrickEfx::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	ischecked = &cheaton;
	onpage = vergilefxsettings;
	full_name_string = "Force SDT Trick Efx";
	author_string = "VPZadov";
	description_string = "Force game to use SDT Trick EFX in all forms.";

	auto sdtEfxAddr = utility::scan(base, "8B 86 B0 09 00 00 4C"); //DevilMayCry5.exe+1DDCB5D
    if (!sdtEfxAddr) {
          return "Unanable to find VergilSDTTrickEfx.sdtEfxAddr pattern.";
    }

	if (!install_hook_absolute(sdtEfxAddr.value(), m_forcesdttrickefx_hook, &trickefx_detour, &retJmp, 0x6)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize VergilSDTTrickEfx.sdtEfx";
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
