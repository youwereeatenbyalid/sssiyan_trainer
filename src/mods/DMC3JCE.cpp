#include "DMC3JCE.hpp"

static naked void can_exe_jce_detour()
{
	__asm {
		setbe al
		cmp byte ptr [DMC3JCE::cheaton], 1
		je cheat

		originalcode:
		//setbe al
		test al, al
		jmp qword ptr [DMC3JCE::canExeJceRet]

		cheat:
		cmp dword ptr [rbx + 0x1978], 0 //isYamato
		jne originalcode
		mov al, 01
		test al, al
		jmp qword ptr [DMC3JCE::canExeJceRet]
	}
}

static naked void can_exe_jce_detour1()
{
	__asm {
		setbe al
		cmp byte ptr [DMC3JCE::cheaton], 1
		je cheat

		originalcode:
		//setbe al
		test al, al
		jmp qword ptr[DMC3JCE::canExeJceRet1]

		cheat:
		movss xmm0, [rdx+0x1B20]//CurSdtVal
		comiss xmm0, [DMC3JCE::minSdt]
		jb disable_jce
		mov al, 01
		test al, al
		jmp qword ptr[DMC3JCE::canExeJceRet1]

		disable_jce:
		mov al, 00
		test al, al
		jmp qword ptr[DMC3JCE::canExeJceRet1]
	}
}

static naked void jcehuman_sub_sdt_detour()
{
	__asm {
		cmp byte ptr [DMC3JCE::cheaton], 1
		je cheat

		originalcode:
		movss [r8+0x00001B20], xmm1
		jmp qword ptr [DMC3JCE::subSdtRet]

		cheat:
		movss xmm1, [r8+0x00001B20]
		movss xmm0, [DMC3JCE::humanJCECost]
		subss xmm1, xmm0
		movss [r8 + 0x00001B20], xmm1
		jmp qword ptr [DMC3JCE::subSdtRet]
	}
}

std::optional<std::string> DMC3JCE::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

	ischecked = &cheaton;
	onpage = vergiltrick;
	full_name_string = "DMC3 JCE (+)";
	author_string = "VPZadov";
	description_string = "Swap virgin DMC5 human JCE to chad DMC3 boss Vergil JCE.";
	//auto pauseBase = base + 0x07E55910;
	//jceController = std::make_unique<JCEController>();

	auto canExeJceAddr = utility::scan(base, "0F 96 C0 84 C0 74 90");//DevilMayCry5.exe+54F481
	if (!canExeJceAddr)
	{
		return "Unable to find DMC3JCE.canExeJceAddr pattern.";
	}

	auto canExeJceAddr1 = utility::scan(base, "0F 96 C0 84 C0 0F 84 CF 00 00 00 48 8B CB 48 85 D2 0F 84 9D 00 00 00 E8 D8");//DevilMayCry5.exe+1C0A54C
	if (!canExeJceAddr1)
	{
		return "Unable to find DMC3JCE.canExeJceAddr1 pattern.";
	}

	auto subHumanJceAddr = utility::scan(base, "F3 41 0F 11 88 20 1B 00 00");//DevilMayCry5.exe+25009B2
	if (!subHumanJceAddr)
	{
		return "Unable to find DMC3JCE.subHumanJceAddr pattern.";
	}

	if (!install_hook_absolute(canExeJceAddr.value(), m_can_exe_jce_hook, &can_exe_jce_detour, &canExeJceRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DMC3JCE.canExeJce";
	}

	if (!install_hook_absolute(canExeJceAddr1.value(), m_can_exe_jce1_hook, &can_exe_jce_detour1, &canExeJceRet1, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DMC3JCE.canExeJce1";
	}

	if (!install_hook_absolute(subHumanJceAddr.value(), m_sub_human_jce_hook, &jcehuman_sub_sdt_detour, &subSdtRet, 0x9))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize DMC3JCE.canExeJce1";
	}

	return Mod::on_initialize();
}

void DMC3JCE::on_config_load(const utility::Config& cfg)
{
}

void DMC3JCE::on_config_save(utility::Config& cfg)
{
}

void DMC3JCE::on_frame()
{
}

void DMC3JCE::on_draw_ui()
{
	ImGui::TextWrapped("Debug shit;");
	ImGui::TextWrapped("Delay between jc spawn (ms). Small value can crash the game:");
	ImGui::InputInt("##Delay", &jceController.rndTime, 50, 120);
	if (ImGui::Button("Start 3 jce"))
	{
		jceController.jceType = JCEController::Random;
		jceController.start_jce();
		//std::thread([&]
		//{
		//	std::this_thread::sleep_for(std::chrono::seconds(2));
		//	//jceController.executing.store(false);
		//	jceController.executing = false;
		//}).detach();
	}

	if (ImGui::Button("Stop 3 jce"))
	{
		jceController.stop_jce();
	}
}

void DMC3JCE::on_draw_debug_ui()
{
}

void DMC3JCE::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
