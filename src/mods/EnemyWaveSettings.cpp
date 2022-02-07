#include "EnemyWaveSettings.hpp"
 
uint32_t EnemyWaveSettings::generateType = 0;
//uint32_t EnemyWaveSettings::permitNum    = 0;

uintptr_t EnemyWaveSettings::enemyWaveRet{NULL};

bool EnemyWaveSettings::cheaton = false;
//bool EnemyWaveSettings::isCustomPermitSetting = false;
//bool EnemyWaveSettings::isCustomGenerateType  = false;

//float EnemyWaveSettings::startWaitTime = 0.0f;

static naked void wave_data_detour() {
	__asm {
		//cmp byte ptr [LDK::cheaton], 1
		//je wavespawncheat
		cmp byte ptr [EnemyWaveSettings::cheaton], 0
		je originalcode
		//cmp byte ptr [EnemyWaveSettings::isCustomGenerateType], 1
		//je generatecheat
		//settingscheck:
		/*cmp byte ptr [EnemyWaveSettings::isCustomPermitSetting], 1
		je permitcheat*/
		jmp originalcode

		/*wavespawncheat://Actually hardlimit
		mov esi, [LDK::hardlimit]
		mov [r8+0x1C], esi
		cmp byte ptr [EnemyWaveSettings::cheaton], 0
		je originalcode*/

		generatecheat:
		mov esi, [EnemyWaveSettings::generateType]
		mov [r8+0x40], esi
		//jmp settingscheck

		/*permitcheat:
		mov r13, [r8+0x30]
		mov esi, [EnemyWaveSettings::permitNum]
		mov [r13+0x10], esi
		//mov byte ptr [r13+0x1C], 0 //isIgnorePermit
		//mov dword ptr [r13+0x14], 1 //PlPermitNum
		//mov dword ptr [r13+0x18], 3 //AddDisableTime
		//mov dword ptr [r8+0x28], 6 //StartWaitTime
		//mov byte ptr [r8+0x2C], 0 //IsParentPermitData*/

		originalcode:
		mov rsi, r9
		mov r13, r8
		jmp qword ptr [EnemyWaveSettings::enemyWaveRet]
	}
}

std::optional<std::string> EnemyWaveSettings::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	ischecked = &cheaton;
	onpage = balance;
	full_name_string = "Enemy waves settings (+)";
	author_string = "VPZadov";
	description_string = "Change some enemy waves settings.";

	auto initAddr1 = patterns.find_addr(base, "49 8B F1 4D 8B E8 4C 8B FA"); // "DevilMayCry5.exe"+FE54FB 
	if (!initAddr1) {
		return "Unanable to find EnemyWaveSettings pattern.";
	}

	if (!install_hook_absolute(initAddr1.value(), m_wave_settings_hook, &wave_data_detour, &enemyWaveRet, 0x6)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize EnemyWaveSettings.initAddr1";
	}

	return Mod::on_initialize();
}

void EnemyWaveSettings::on_config_load(const utility::Config& cfg)
{
	//isCustomGenerateType = cfg.get<bool>("EnemyWaveSettings.isCustomGenerateType").value_or(false);
	generateType = cfg.get<uint32_t>("EnemyWaveSettings.generateType").value_or(0);
	//isCustomPermitSetting = cfg.get<bool>("EnemyWaveSettings.isCustomPermitSetting").value_or(false);
	//permitNum = cfg.get<uint32_t>("EnemyWaveSettings.permitNum").value_or(0);
}

void EnemyWaveSettings::on_config_save(utility::Config& cfg)
{
	//cfg.set<bool>("EnemyWaveSettings.isCustomGenerateType", isCustomGenerateType);
	cfg.set<uint32_t>("EnemyWaveSettings.generateType", generateType);
	//cfg.set<bool>("EnemyWaveSettings.isCustomPermitSetting", isCustomPermitSetting);
	//cfg.set<uint32_t>("EnemyWaveSettings.permitNum", permitNum);

}

void EnemyWaveSettings::on_frame()
{
}

void EnemyWaveSettings::on_draw_ui()
{
	//ImGui::Checkbox("Use custom generate type setting", &isCustomGenerateType);
	ImGui::TextWrapped("Change the order of spawns in subwaves.");
	ImGui::TextWrapped("Change \"generateType\" to: 0 - order, 1 - random, 2 - parallel. Parallel is glitchy and can cause enemies not to spawn.");
	UI::SliderInt("##generateTypeSlider", (int*)&generateType, 0, 2);
	ImGui::Separator();

	/*ImGui::TextWrapped("(?) Controls, how many enemies can attack at the same time in a wave (?).");
	ImGui::Checkbox("Use custom attack permit settings. ", &isCustomPermitSetting);
	ImGui::TextWrapped("(?)Enemies num, that can start attak at the same time:(?)");//no
	UI::SliderInt("##permitNumSlider", (int*)&permitNum, 0, 25);*/
}

void EnemyWaveSettings::on_draw_debug_ui()
{
}

void EnemyWaveSettings::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
