#include "AirMoves.hpp"

bool AirMoves::is_movecheat_enabled_asm(Moves move)
{
	MoveInfo *p = nullptr;
	switch (PlayerTracker::playerid)
	{
		//case 0://Nero
		//{
		//	break;
		//}
		case 1://Dante
		{
			p = (*danteMoves)[move];
			break;
		}
		case 4://plVergil
		{
			p = (*vergilMoves)[move];
			break;
		}
		default:
			break;
	}
	if (p != nullptr)
	{
		if (p->cheatOn)
		{
			if (strcmp(p->get_ingame_name(), "") == 0)
			{
				curMoveHook = nullptr;
				return true;
			}
			curMoveHook = p;
			return true;
		}
	}
	curMoveHook = nullptr;
	return false;
}

void AirMoves::str_cur_action_asm(uintptr_t dotNetString)
{
	if (curMoveHook != nullptr)
	{
		auto str = GameFunctions::PtrController::get_str(dotNetString);
		if (curMoveHook->cheatOn && strcmp(str, curMoveHook->get_ingame_name()) == 0)
		{
			isAirProcess = true;
		}
		else if(strcmp(str, "None") != 0 && strcmp(str, "PutOut") != 0)//PutOut - Dante's stingers 2nd phase
		{
			isAirProcess = false;
			curMoveHook = nullptr;
		}
	}
}

static naked void rapidslash_aircheck_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cmp al, 1
		je setnull
		push rax
		push rcx
		mov ecx, AirMoves::Moves::RapidSlash
		sub rsp, 32
		call qword ptr[AirMoves::is_movecheat_enabled_asm]
		add rsp, 32
		cmp al, 0
		pop rcx
		pop rax
		je originalcode

		cheat:
		mov al, 1

		originalcode:
		movzx ecx,al
		mov rax, [rbx + 0x50]
		jmp qword ptr [AirMoves::rapidSlashIsAirRet]

		setnull:
		mov AirMoves::curMoveHook, 0
		jmp originalcode
	}
}

static naked void fedrive_aircheck_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cmp al, 1
		je setnull
		push rax
		push rcx
		mov ecx, AirMoves::Moves::FEDrive
		sub rsp, 32
		call qword ptr[AirMoves::is_movecheat_enabled_asm]
		add rsp, 32
		cmp al, 0
		pop rcx
		pop rax
		je originalcode

		cheat:
		mov al, 1

		originalcode:
		movzx ecx,al
		mov rax, [rbx + 0x50]
		jmp qword ptr [AirMoves::feDriveIsAirRet]

		setnull:
		mov AirMoves::curMoveHook, 0
		jmp originalcode
	}
}

static naked void void_slash_air_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cmp al, 1
		je setnull
		push rax
		push rcx
		mov ecx, AirMoves::Moves::VoidSlash
		sub rsp, 32
		call qword ptr[AirMoves::is_movecheat_enabled_asm]
		add rsp, 32
		cmp al, 0
		pop rcx
		pop rax
		je originalcode

		cheat:
		mov al, 1

		originalcode:
		movzx ecx,al
		mov rax, [rbx + 0x50]
		jmp qword ptr [AirMoves::voidSlashIsAirRet]

		setnull:
		mov AirMoves::curMoveHook, 0
		jmp originalcode
	}
}

static naked void rb_stinger_air_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cmp al, 1
		je setnull
		push rax
		push rcx
		mov ecx, AirMoves::Moves::RBStinger
		sub rsp, 32
		call qword ptr[AirMoves::is_movecheat_enabled_asm]
		add rsp, 32
		cmp al, 0
		pop rcx
		pop rax
		je originalcode

		cheat:
		mov al, 1

		originalcode:
		movzx ecx,al
		mov rax, [rbx + 0x50]
		jmp qword ptr [AirMoves::rbStingerAirRet]

		setnull:
		mov AirMoves::curMoveHook, 0
		jmp originalcode
	}
}

static naked void sp_stinger_air_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cmp al, 1
		je setnull
		push rax
		push rcx
		mov ecx, AirMoves::Moves::SPStinger
		sub rsp, 32
		call qword ptr[AirMoves::is_movecheat_enabled_asm]
		add rsp, 32
		cmp al, 0
		pop rcx
		pop rax
		je originalcode

		cheat:
		mov al, 1

		originalcode:
		movzx ecx,al
		mov rax, [rbx + 0x50]
		jmp qword ptr [AirMoves::spStingerAirRet]

		setnull:
		mov AirMoves::curMoveHook, 0
		jmp originalcode
	}
}

static naked void ds_stinger_air_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cmp al, 1
		je setnull
		push rax
		push rcx
		mov ecx, AirMoves::Moves::DSStinger
		sub rsp, 32
		call qword ptr[AirMoves::is_movecheat_enabled_asm]
		add rsp, 32
		cmp al, 0
		pop rcx
		pop rax
		je originalcode

		cheat:
		mov al, 1

		originalcode:
		movzx ecx,al
		mov rax, [rbx + 0x50]
		jmp qword ptr [AirMoves::dsStingerAirRet]

		setnull:
		mov AirMoves::curMoveHook, 0
		jmp originalcode
	}
}

static naked void balrog_updraft_air_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cmp al, 1
		je setnull
		push rax
		push rcx
		mov ecx, AirMoves::Moves::BRKUpdraft
		sub rsp, 32
		call qword ptr[AirMoves::is_movecheat_enabled_asm]
		add rsp, 32
		cmp al, 0
		pop rcx
		pop rax
		je originalcode

		cheat:
		mov al, 1

		originalcode:
		movzx ecx,al
		mov rax, [rbx + 0x50]
		jmp qword ptr [AirMoves::balrogUpdraftAirRet]

		setnull:
		mov AirMoves::curMoveHook, 0
		jmp originalcode
	}
}

static naked void beo_kick13_air_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cmp al, 1
		je setnull
		push rax
		push rcx
		mov ecx, AirMoves::Moves::BWKick13
		sub rsp, 32
		call qword ptr[AirMoves::is_movecheat_enabled_asm]
		add rsp, 32
		cmp al, 0
		pop rcx
		pop rax
		je originalcode

		cheat:
		mov al, 1

		originalcode:
		movzx ecx,al
		mov rax, [rbx + 0x50]
		jmp qword ptr [AirMoves::kick13AirRet]

		setnull:
		mov AirMoves::curMoveHook, 0
		jmp originalcode
	}
}

static naked void pl_set_action_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0
		je originalcode

		push rax
		push rcx
		push rdx
		push r8
		push r9
		push r10
		push r11
		mov rcx, r8
		sub rsp, 32
		call qword ptr [AirMoves::str_cur_action_asm]
		add rsp, 32
		pop r11
		pop r10
		pop r9
		pop r8
		pop rdx
		pop rcx
		pop rax

		originalcode:
		mov [rsp + 0x08], rbx
		jmp qword ptr [AirMoves::CurMoveStrRet]
	}
}

static naked void check_ground_hit_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cheat:
		cmp qword ptr [PlayerTracker::playerentity], rdx
		jne originalcode
		cmp byte ptr [AirMoves::isAirProcess], 0
		je originalcode
		jmp qword ptr [AirMoves::checkGroundHitCallRet]

		originalcode:
		call qword ptr [r8+0x000005A0]
		jmp qword ptr [AirMoves::checkGroundHitCallRet]
	}
}

static naked void trickdodge_air_crash_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 1 
		je cheat

		originalcode:
		cmp qword ptr [rax+0x18],00
		jmp qword ptr [AirMoves::airTrickDodgeCrashRet]

		cheat:
		cmp rax, 0
		jne originalcode

		ret_jne:
		jmp qword ptr [AirMoves::airTrickDodgeCrashJne]
	}
}


std::optional<std::string> AirMoves::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	auto p64Base = g_framework->get_module().as<uintptr_t>();
	m_is_enabled = &cheaton;
	m_on_page = mechanics;
	m_full_name_string = "Air Moves";
	m_author_string = "VPZadov";
	m_description_string = "Allow perform some ground moves in the air.";

	set_up_hotkey();

	auto rapidSlashIsAirAddr = patterns->find_addr(base, "0F B6 C8 48 8B 43 50 48 83 78 18 00 75 CD 85 C9 74 34");//DevilMayCry5.exe+1C0930F
	if (!rapidSlashIsAirAddr)
	{
		return "Unable to find AirMoves.rapidSlashIsAirAddr pattern.";
	}

	auto plSetActionAddr = patterns->find_addr(base, "CC CC CC CC 48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 60 80 BC");//DevilMayCry5.app_Player__setAction171195 (-0x4)
	if (!plSetActionAddr)
	{
		return "Unable to find AirMoves.plSetActionAddr pattern.";
	}

	auto checkGroundHitAddr = patterns->find_addr(base, "41 FF 90 A0 05 00 00");//DevilMayCry5.app_character_Character__checkGroundHit167863+60B
	if (!checkGroundHitAddr)
	{
		return "Unable to find AirMoves.checkGroundHitAddr pattern.";
	}

	auto feDriveAirAddr = patterns->find_addr(base, "0F B6 C8 48 8B 43 50 48 83 78 18 00 75 CD 85 C9 74 7C");//DevilMayCry5.exe+C85970
	if (!feDriveAirAddr)
	{
		return "Unable to find AirMoves.feDriveAirAddr pattern.";
	}

	auto rbStingerAirAddr = patterns->find_addr(base, "07 2E 01 32 C0 48 8B 5C 24 60 48 83 C4 50 5F C3 48 8B 02 48 8B 48 F0");//DevilMayCry5.exe+1261038 (-0x23)
	if (!rbStingerAirAddr)
	{
		return "Unable to find AirMoves.rbStingerAirAddr pattern.";
	}

	//auto spStingerAirAddr = patterns->find_addr(base, "07 2E 01 32 C0 48 8B 5C 24 60 48 83 C4 50 5F C3 48 8B 02 48 8B 48 F0 48");//DevilMayCry5.exe+16E3058 (-0x23)
	//if (!spStingerAirAddr)
	//{
	//	return "Unable to find AirMoves.rbStingerAirAddr pattern.";
	//}

	auto balrogUpdraftAirAddr = patterns->find_addr(base, "4F B9 83 FF 0F B6 C8 48 8B 43 50");//DevilMayCry5.exe+C1F4A1 (-0x4)
	if (!balrogUpdraftAirAddr)
	{
		return "Unable to find AirMoves.balrogUpdraftAirAddr pattern.";
	}

	auto kick13AirAddr = patterns->find_addr(base, "0F B6 C8 48 8B 43 50 48 83 78 18 00 75 98 85 C9 74 33");//DevilMayCry5.exe+11DB125
	if (!balrogUpdraftAirAddr)
	{
		return "Unable to find AirMoves.kick13AirAddr pattern.";
	}

	auto airDodgeCrashAddr = patterns->find_addr(base, "1D 11 01 48 8B 43 50 48 83 78 18 00");//DevilMayCry5.exe+542474 (-0x7)
	if (!airDodgeCrashAddr)
	{
		return "Unable to find AirMoves.airDodgeCrashAddr pattern.";
	}

	uintptr_t voidSlashAirAddr = p64Base + 0x1C0A3D5;
	uintptr_t spStingerAirAddr = p64Base + 0x16E3058;
	uintptr_t dsStingerAirAddr = p64Base + 0x1CCE4E7;
	airTrickDodgeCrashJne = airDodgeCrashAddr.value() + 0x7 + 0xBF;

	if (!install_hook_absolute(rapidSlashIsAirAddr.value(), m_rapidslash_air_hook, &rapidslash_aircheck_detour, &rapidSlashIsAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.rapidSlashIsAir";
	}

	if (!install_hook_absolute(plSetActionAddr.value() + 0x4, cur_action_hook, &pl_set_action_detour, &CurMoveStrRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.plSetAction";
	}

	if (!install_hook_absolute(checkGroundHitAddr.value(), m_check_ground_hit_hook, &check_ground_hit_detour, &checkGroundHitCallRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.checkGroundHit";
	}

	if (!install_hook_absolute(feDriveAirAddr.value(), m_fe_drive_air_hook, &fedrive_aircheck_detour, &feDriveIsAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.feDriveAir";
	}

	if (!install_hook_absolute(voidSlashAirAddr, m_void_slash_air_hook, &void_slash_air_detour, &voidSlashIsAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.voidSlashAir";
	}

	if (!install_hook_absolute(rbStingerAirAddr.value() + 0x23, m_rb_stinger_air_hook, &rb_stinger_air_detour, &rbStingerAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.rbStingerAir";
	}

	if (!install_hook_absolute(spStingerAirAddr, m_sp_stinger_air_hook, &sp_stinger_air_detour, &spStingerAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.spStingerAir";
	}

	if (!install_hook_absolute(dsStingerAirAddr, m_ds_stinger_air_hook, &ds_stinger_air_detour, &dsStingerAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.dsStingerAir";
	}

	if (!install_hook_absolute(balrogUpdraftAirAddr.value() + 0x4, m_brk_updraft_air_hook, &balrog_updraft_air_detour, &balrogUpdraftAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.balrogUpdraftAir";
	}

	if (!install_hook_absolute(kick13AirAddr.value(), m_kick_air_hook, &beo_kick13_air_detour, &kick13AirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.kick13Air";
	}

	if (!install_hook_absolute(airDodgeCrashAddr.value() + 0x7, m_air_dodge_crash_hook, &trickdodge_air_crash_detour, &airTrickDodgeCrashRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.airDodgeCrash";
	}

	return Mod::on_initialize();
}

void AirMoves::on_config_load(const utility::Config& cfg)
{
	for (int i = 0; i < vergilMoves->size(); i++)
	{
		vergilMoves->operator[](i).cheatOn = cfg.get<bool>(std::string("AirMoves." + std::string(vergilMoves->operator[](i).get_name()))).value_or(false);
	}
	for (int i = 0; i < danteMoves->size(); i++)
	{
		danteMoves->operator[](i).cheatOn = cfg.get<bool>(std::string("AirMoves." + std::string(danteMoves->operator[](i).get_name()))).value_or(false);
	}
}

void AirMoves::on_config_save(utility::Config& cfg)
{
	for (int i = 0; i < vergilMoves->size(); i++)
	{
		cfg.set<bool>(std::string( "AirMoves." + std::string(vergilMoves->operator[](i).get_name())), vergilMoves->operator[](i).cheatOn);
	}
	for (int i = 0; i < vergilMoves->size(); i++)
	{
		cfg.set<bool>(std::string("AirMoves." + std::string(danteMoves->operator[](i).get_name())), danteMoves->operator[](i).cheatOn);
	}
}

void AirMoves::on_frame()
{
}

void AirMoves::on_draw_ui()
{
	ImGui::TextWrapped("Vergil");
	ImGui::Spacing();

	for (int i = 0; i < vergilMoves->size(); i++)
	{
		ImGui::Checkbox(vergilMoves->operator[](i).get_name(), &vergilMoves->operator[](i).cheatOn);
	}

	ImGui::Separator();
	ImGui::TextWrapped("Dante");
	ImGui::Spacing();
	for (int i = 0; i < danteMoves->size(); i++)
	{
		ImGui::Checkbox(danteMoves->operator[](i).get_name(), &danteMoves->operator[](i).cheatOn);
	}
}

void AirMoves::on_draw_debug_ui()
{
}

void AirMoves::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}