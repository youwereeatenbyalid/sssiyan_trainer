#include "BossDanteSetup.hpp"
//clang-format off

static naked void dtregen_detour()
{
	__asm {
		cmp byte ptr [BossDanteSetup::cheaton], 0
		je originalcode
		cmp byte ptr[BossDanteSetup::isDtRegenEnabled], 0
		je originalcode
		jmp qword ptr [BossDanteSetup::dtRegenRet]

		originalcode:
		cmp byte ptr [rsi+0x20], 00
		je ret_je
		jmp qword ptr [BossDanteSetup::dtRegenRet]

		ret_je:
		jmp qword ptr [BossDanteSetup::dtRegenJe]
	}
}

static naked void sdt_transform_detour()
{
	__asm {
		cmp byte ptr [BossDanteSetup::cheaton], 0
		je originalcode
		cmp byte ptr [BossDanteSetup::isSdtTransformSetup], 0
		je originalcode

		cheat:
		mov ecx, dword ptr [BossDanteSetup::sdtTransformMode]
		jmp ret_jmp

		originalcode:
		mov ecx, [rcx+0x10]

		ret_jmp:
		test ecx, ecx
		jmp qword ptr [BossDanteSetup::sdtTransformRet]
	}
}

std::optional<std::string> BossDanteSetup::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	ischecked = &cheaton;
	onpage = balance;
	full_name_string = "Boss Dante settings";
	author_string = "VPZadov";
	description_string = "Vergil's nightmare begins here.";

	auto dtAddr = utility::scan(base, "80 7E 20 00 0F 84 01 02 00 00"); // DevilMayCry5.exe+19C34A9
	if (!dtAddr)
	{
		return "Unanable to find BossDanteSetup.dtAddr pattern.";
	}

	auto sdtTransformAddr = utility::scan(base, "8B 49 10 85 C9 74 35"); // DevilMayCry5.exe+19C637C
	if (!sdtTransformAddr)
	{
		return "Unanable to find BossDanteSetup.sdtTransformAddr pattern.";
	}

	dtRegenJe = dtAddr.value() + 0x20B;

	if (!install_hook_absolute(dtAddr.value(), m_dtregen_hook, &dtregen_detour, &dtRegenRet, 0xA))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize BossDanteSetup.dt";
	}

	if (!install_hook_absolute(sdtTransformAddr.value(), m_sdttransform_hook, &sdt_transform_detour, &sdtTransformRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize BossDanteSetup.sdtTransform";
	}

	return Mod::on_initialize();
}

void BossDanteSetup::on_config_load(const utility::Config& cfg)
{
	isDtRegenEnabled = cfg.get<bool>("BossDanteSetup.isDtRegenEnabled").value_or(true);
	isSdtTransformSetup = cfg.get<bool>("BossDanteSetup.isSdtTransformSetup").value_or(true);
	sdtTransformMode = cfg.get<uint32_t>("BossDanteSetup.sdtTransformMode").value_or(0);
}

void BossDanteSetup::on_config_save(utility::Config& cfg)
{
	cfg.set<bool>("BossDanteSetup.isDtRegenEnabled", isDtRegenEnabled);
	cfg.set<bool>("BossDanteSetup.isSdtTransformSetup", isSdtTransformSetup);
	cfg.set<uint32_t>("BossDanteSetup.sdtTransformMode", sdtTransformMode);
}

void BossDanteSetup::on_frame()
{
}

void BossDanteSetup::on_draw_ui()
{
	ImGui::Checkbox("Enable DT HP regen", &isDtRegenEnabled);
	ImGui::Checkbox("Custom SDT transform setup", &isSdtTransformSetup);
	if (isSdtTransformSetup)
	{
		ImGui::Spacing();
		ImGui::TextWrapped("Change transform mode to: 0 - always normal transform, 1 - always fast transform (default), 2 - first normal other fast (idk is this one even working?)");
		ImGui::SliderInt("##TransformMode", (int*)&sdtTransformMode, 0, 2);
	}
}

void BossDanteSetup::on_draw_debug_ui()
{
}

void BossDanteSetup::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
//clamg-format on
