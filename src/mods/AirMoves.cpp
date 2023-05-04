#include "AirMoves.hpp"
#include "PlSetActionData.hpp"

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

void AirMoves::str_cur_action_asm(uintptr_t curPl, uintptr_t str)
{
	if (AirMoves::curMoveHook != nullptr)
	{
		if (*(int*)(curPl + 0xE64) == 4 && *(bool*)(curPl + 0x17F0))
		{
			return;
		}
		else
		{
			if (AirMoves::curMoveHook->cheatOn && gf::StringController::str_cmp(str, AirMoves::curMoveHook->get_ingame_name()))
				AirMoves::isAirProcess = true;
			else if ( !gf::StringController::str_cmp(str, L"None") && !gf::StringController::str_cmp(str, L"PutOut"))
			{
				AirMoves::isAirProcess = false;
				AirMoves::curMoveHook = nullptr;
			}
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

static naked void cerberus_ice_age_air_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cmp al, 1
		je setnull
		push rax
		push rcx
		mov ecx, AirMoves::Moves::IceAge
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
		jmp qword ptr [AirMoves::cerberusIceAgeAirRet]

		setnull:
		mov AirMoves::curMoveHook, 0
		jmp originalcode
	}
}

static naked void cerberus_blitz_air_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cmp al, 1
		je setnull
		push rax
		push rcx
		mov ecx, AirMoves::Moves::CerbBlitz
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
		jmp qword ptr [AirMoves::cerberusBlitzAirRet]

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

static naked void check_ground_hit_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cheat:
		cmp qword ptr [PlayerTracker::playerentity], rdx
		jne doppelcheck
		aircheck:
		cmp byte ptr [AirMoves::isAirProcess], 0
		je originalcode
		jmp qword ptr [AirMoves::checkGroundHitCallRet]

		originalcode:
		call qword ptr [r8+0x000005A0]
		jmp qword ptr [AirMoves::checkGroundHitCallRet]

		doppelcheck:
		cmp qword ptr [PlayerTracker::doppelentity], rdx
		jne originalcode
		jmp aircheck
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

static naked void sparda_rt_air_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cmp al, 1
		je setnull
		push rax
		push rcx
		mov ecx, AirMoves::Moves::SpardaRT
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
		jmp qword ptr [AirMoves::_spardaRTAirRet]

		setnull:
		mov AirMoves::curMoveHook, 0
		jmp originalcode
	}
}

static naked void rebellion_rt_air_detour()
{
	__asm {
		cmp byte ptr [AirMoves::cheaton], 0 
		je originalcode

		cmp al, 1
		je setnull
		push rax
		push rcx
		mov ecx, AirMoves::Moves::RebellionRT
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
		jmp qword ptr [AirMoves::_rbRTAirRet]

		setnull:
		mov AirMoves::curMoveHook, 0
		jmp originalcode
	}
}

//static naked void jce_air_detour()
//{
//	__asm {
//		cmp byte ptr [AirMoves::cheaton], 0 
//		je originalcode
//
//		cmp al, 1
//		je setnull
//		push rax
//		push rcx
//		mov ecx, AirMoves::Moves::JCE
//		sub rsp, 32
//		call qword ptr[AirMoves::is_movecheat_enabled_asm]
//		add rsp, 32
//		cmp al, 0
//		pop rcx
//		pop rax
//		je originalcode
//
//		cheat:
//		mov al, 1
//
//		originalcode:
//		movzx ecx,al
//		mov rax, [rbx + 0x50]
//		jmp qword ptr [AirMoves::_jceAirRet]
//
//		setnull:
//		mov AirMoves::curMoveHook, 0
//		jmp originalcode
//	}
//}


std::optional<std::string> AirMoves::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	auto p64Base = g_framework->get_module().as<uintptr_t>();
	m_is_enabled = &cheaton;
	m_on_page = Page_Mechanics;
	m_depends_on = { "PlSetActionData","PlayerTracker"};
	m_full_name_string = "Air Moves (+)";
	m_author_string = "V.P.Zadov";
	m_description_string = "Allow characters to perform certain ground moves in the air.";

	set_up_hotkey();

	auto rapidSlashIsAirAddr = m_patterns_cache->find_addr(base, "0F B6 C8 48 8B 43 50 48 83 78 18 00 75 CD 85 C9 74 34");//DevilMayCry5.exe+1C0930F
	if (!rapidSlashIsAirAddr)
	{
		return "Unable to find AirMoves.rapidSlashIsAirAddr pattern.";
	}

	auto checkGroundHitAddr = m_patterns_cache->find_addr(base, "41 FF 90 A0 05 00 00");//DevilMayCry5.app_character_Character__checkGroundHit167863+60B
	if (!checkGroundHitAddr)
	{
		return "Unable to find AirMoves.checkGroundHitAddr pattern.";
	}

	auto feDriveAirAddr = m_patterns_cache->find_addr(base, "0F B6 C8 48 8B 43 50 48 83 78 18 00 75 CD 85 C9 74 7C");//DevilMayCry5.exe+C85970
	if (!feDriveAirAddr)
	{
		return "Unable to find AirMoves.feDriveAirAddr pattern.";
	}

	auto rbStingerAirAddr = m_patterns_cache->find_addr(base, "07 2E 01 32 C0 48 8B 5C 24 60 48 83 C4 50 5F C3 48 8B 02 48 8B 48 F0");//DevilMayCry5.exe+1261038 (-0x23)
	if (!rbStingerAirAddr)
	{
		return "Unable to find AirMoves.rbStingerAirAddr pattern.";
	}

	//auto spStingerAirAddr = patterns->find_addr(base, "07 2E 01 32 C0 48 8B 5C 24 60 48 83 C4 50 5F C3 48 8B 02 48 8B 48 F0 48");//DevilMayCry5.exe+16E3058 (-0x23)
	//if (!spStingerAirAddr)
	//{
	//	return "Unable to find AirMoves.rbStingerAirAddr pattern.";
	//}

	auto balrogUpdraftAirAddr = m_patterns_cache->find_addr(base, "4F B9 83 FF 0F B6 C8 48 8B 43 50");//DevilMayCry5.exe+C1F4A1 (-0x4)
	if (!balrogUpdraftAirAddr)
	{
		return "Unable to find AirMoves.balrogUpdraftAirAddr pattern.";
	}

	auto kick13AirAddr = m_patterns_cache->find_addr(base, "0F B6 C8 48 8B 43 50 48 83 78 18 00 75 98 85 C9 74 33");//DevilMayCry5.exe+11DB125
	if (!balrogUpdraftAirAddr)
	{
		return "Unable to find AirMoves.kick13AirAddr pattern.";
	}

	auto airDodgeCrashAddr = m_patterns_cache->find_addr(base, "1D 11 01 48 8B 43 50 48 83 78 18 00");//DevilMayCry5.exe+542474 (-0x7)
	if (!airDodgeCrashAddr)
	{
		return "Unable to find AirMoves.airDodgeCrashAddr pattern.";
	}

	auto iceAgeAirAddr = m_patterns_cache->find_addr(base, "14 7C FE 0F B6 C8 48 8B 43 50");//DevilMayCry5.exe+1C99917 (-0x3)
	if (!iceAgeAirAddr)
	{
		return "Unable to find AirMoves.iceAgeAirAddr pattern.";
	}

	auto blitzAddr = m_patterns_cache->find_addr(base, "0F B6 C8 48 8B 43 50 48 83 78 18 00 0F 85 C1 FE FF FF 85 C9 0F");//DevilMayCry5.exe+1C9A245
	if (!iceAgeAirAddr)
	{
		return "Unable to find AirMoves.blitzAddr pattern.";
	}

	uintptr_t voidSlashAirAddr = p64Base + 0x1C0A3D5;
	uintptr_t spStingerAirAddr = p64Base + 0x16E3058;
	uintptr_t dsStingerAirAddr = p64Base + 0x1CCE4E7;
	airTrickDodgeCrashJne = airDodgeCrashAddr.value() + 0x7 + 0xBF;
	uintptr_t spRtAirAddr = p64Base + 0x16E3988;
	uintptr_t rbRtAirAddr = p64Base + 0x12617F8;
	//uintptr_t jceAirAddr = p64Base + 0x1C0A4E0;

	if (!install_new_detour(rapidSlashIsAirAddr.value(), m_rapidslash_air_detour, &rapidslash_aircheck_detour, &rapidSlashIsAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.rapidSlashIsAir";
	}

	if (!install_new_detour(checkGroundHitAddr.value(), m_check_ground_hit_detour, &check_ground_hit_detour, &checkGroundHitCallRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.checkGroundHit";
	}

	if (!install_new_detour(feDriveAirAddr.value(), m_fe_drive_air_detour, &fedrive_aircheck_detour, &feDriveIsAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.feDriveAir";
	}

	if (!install_new_detour(voidSlashAirAddr, m_void_slash_air_detour, &void_slash_air_detour, &voidSlashIsAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.voidSlashAir";
	}

	if (!install_new_detour(rbStingerAirAddr.value() + 0x23, m_rb_stinger_air_detour, &rb_stinger_air_detour, &rbStingerAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.rbStingerAir";
	}

	if (!install_new_detour(spStingerAirAddr, m_sp_stinger_air_detour, &sp_stinger_air_detour, &spStingerAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.spStingerAir";
	}

	if (!install_new_detour(dsStingerAirAddr, m_ds_stinger_air_detour, &ds_stinger_air_detour, &dsStingerAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.dsStingerAir";
	}

	if (!install_new_detour(balrogUpdraftAirAddr.value() + 0x4, m_brk_updraft_air_detour, &balrog_updraft_air_detour, &balrogUpdraftAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.balrogUpdraftAir";
	}

	if (!install_new_detour(kick13AirAddr.value(), m_kick_air_detour, &beo_kick13_air_detour, &kick13AirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.kick13Air";
	}

	if (!install_new_detour(airDodgeCrashAddr.value() + 0x7, m_air_dodge_crash_detour, &trickdodge_air_crash_detour, &airTrickDodgeCrashRet, 0x5))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.airDodgeCrash";
	}

	if (!install_new_detour(iceAgeAirAddr.value() + 0x3, m_cerberus_ice_edge_air_detour, &cerberus_ice_age_air_detour, &cerberusIceAgeAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.iceAgeAir";
	}

	if (!install_new_detour(spRtAirAddr, m_spRtAir_detour, &sparda_rt_air_detour, &_spardaRTAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.spRtAir";
	}

	if (!install_new_detour(rbRtAirAddr, m_rbRtAir_detour, &rebellion_rt_air_detour, &_rbRTAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.rbRtAir";
	}

	if (!install_new_detour(blitzAddr.value(), m_cerberus_blitz_air_detour, &cerberus_blitz_air_detour, &cerberusBlitzAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.blitz";
	}

	/*if (!install_new_detour(jceAirAddr, m_jceAir_detour, &jce_air_detour, &_jceAirRet, 0x7))
	{
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AirMoves.jceAir";
	}*/

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
	for (int i = 0; i < danteMoves->size(); i++)
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