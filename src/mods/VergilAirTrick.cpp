#include "VergilAirTrick.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h"
#include "EnemyData.hpp"

bool VergilAirTrick::cheaton = false;
bool VergilAirTrick::isSpeedUp = false;
bool VergilAirTrick::isCustomOffset = false;
bool VergilAirTrick::isCustomWaitTime = false;

float VergilAirTrick::initSpeed = 0.0f;
float VergilAirTrick::finishOffsetZ = 0.0f;
float VergilAirTrick::waitTime      = 0.0f;

float specTeleportOffset = 0.25f;

uintptr_t VergilAirTrick::airTrickRet{NULL};
uintptr_t VergilAirTrick::finishOffsetRet{NULL};
uintptr_t VergilAirTrick::waitTimeRet{NULL};
uintptr_t VergilAirTrick::maxXZRet{NULL};
uintptr_t VergilAirTrick::maxSpeedZRet{NULL};
uintptr_t VergilAirTrick::speedAccRet{NULL};
uintptr_t VergilAirTrick::initSpeedRet{NULL};
uintptr_t VergilAirTrick::routineStartRet{NULL};

static naked void waittime_detour() {
	__asm {
		cmp byte ptr [VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr [VergilAirTrick::isCustomWaitTime], 0
		je originalcode

		cheat:
		mov eax, dword ptr [VergilAirTrick::waitTime]
		jmp qword ptr [VergilAirTrick::waitTimeRet]

		originalcode:
		mov eax, [rsi+0x000000B8]
		jmp qword ptr [VergilAirTrick::waitTimeRet]
  }
}

static naked void finish_offset_detour() {//don't forget t skip all coords opcodes
	__asm {
		cmp byte ptr [VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr [VergilAirTrick::isCustomOffset], 0
		je originalcode

		cheat:
		movss xmm0,[rsi+0x000000D0]
        movss xmm1, [VergilAirTrick::finishOffsetZ]
        movss xmm2, [VergilAirTrick::modFinishOffsetX]
		jmp qword ptr [VergilAirTrick::finishOffsetRet]

		originalcode:
		movss xmm0, [rsi+0x000000D0]
        movss xmm1, [rsi+0x000000D4]
        movss xmm2, [rsi+0x000000D8]
		jmp qword ptr [VergilAirTrick::finishOffsetRet]
  }
}

static naked void initspeed_detour() {
	__asm {
		cmp byte ptr[VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr[VergilAirTrick::isTeleport], 1
		je originalcode//teleport
		cmp byte ptr [VergilAirTrick::isSpeedUp], 0
		je originalcode

		cheat:
		mov eax, [VergilAirTrick::initSpeed]
		jmp qword ptr [VergilAirTrick::initSpeedRet]

		originalcode:
		mov eax, dword ptr [rsi+0x000000A0]
		jmp qword ptr [VergilAirTrick::initSpeedRet]

		teleport:
		mov eax, 0
		jmp qword ptr [VergilAirTrick::initSpeedRet]
	}
}

void VergilAirTrick::pos_teleport(TeleportType type, gf::Vec3& outVec, GameFunctions::Vec3 pPos, gf::Vec3 trickVec, float trickCorrect, float trickLen)
{
	switch (type)
	{
		case VergilAirTrick::Front:
		{
			outVec.x = pPos.x + (trickLen - trickCorrect) * trickVec.x / (float)trickLen;
			outVec.y = pPos.y + (trickLen - trickCorrect) * trickVec.y / (float)trickLen;
			//outVec.z = pPos.z + (trickLen - trickCorrect) * trickVec.z / (float)trickLen;
			break;
		}
		case VergilAirTrick::Behind:
		{
			outVec.x = pPos.x + (trickLen + trickCorrect) * trickVec.x / (float)trickLen;
			outVec.y = pPos.y + (trickLen + trickCorrect) * trickVec.y / (float)trickLen;
			//outVec.z = pPos.z + (trickLen + trickCorrect) * trickVec.z / (float)trickLen;
			break;
		}
		default:
		{
			outVec.x = pPos.x + (trickLen - trickCorrect) * trickVec.x / (float)trickLen;
			outVec.y = pPos.y + (trickLen - trickCorrect) * trickVec.y / (float)trickLen;
			//outVec.z = pPos.z + (trickLen - trickCorrect) * trickVec.z / (float)trickLen;
			break;
		}
	}
}


void VergilAirTrick::change_pos_asm(uintptr_t trickAction)
{
	if(trickAction == 0)
	{
		_mod->_isDefaultFinishRangeRequested = true;
		return;
	}
	uint16_t trickOffsAddr = 0;
	__asm (
	"movq %1, %%rax;"
	"movq (%%rax), %%rax;"
	"mov %%ax, %0"
	:"=m"(trickOffsAddr)
	:"g"(trickAction)
	:"rax"
	);
	if (trickOffsAddr == 0xF208)//GrimTrick
	{
		_mod->_isDefaultFinishRangeRequested = true;
		return;
	}
	uintptr_t vergil = *(uintptr_t*)(trickAction + 0x60);//Also for doppel when he summoned;
	if (vergil == 0)
	{
		_mod->_isDefaultFinishRangeRequested = true;
		return;
	}
	auto trickTargetCtrl = *(uintptr_t*)(vergil + 0x19B0);
	uintptr_t enemyToTrick = 0, shellToTrick = 0;
	if (trickTargetCtrl != 0)
	{
		if (*(int*)(trickTargetCtrl + 0x10) == 2)//grimTrick
			return;
		auto lockOnObj = *(uintptr_t*)(trickTargetCtrl + 0x18);
		if (lockOnObj != 0)
		{
			auto target = *(uintptr_t*)(lockOnObj + 0x10);
			if (target != 0)
			{
				enemyToTrick = *(uintptr_t*)(target + 0x60);
				shellToTrick = *(uintptr_t*)(target + 0x68);
			}
		}
	}
	auto emId = EnemyData::get_em_id(enemyToTrick);
	if (enemyToTrick != 0 && shellToTrick == 0)
	{
		if (emId == EnemyData::Nidhogg)//fuck nidhogg
		{
			_mod->_isDefaultFinishRangeRequested = true;
			return;
		}
		if (emId == EnemyData::DeathScissors)
		{
			auto terrainChecker = *(uintptr_t*)(enemyToTrick + 0xD80);
			if (terrainChecker == 0 || *(bool*)(terrainChecker + 0x50))//isOutOfArea
			{
				_mod->_isDefaultFinishRangeRequested = true;
				return;
			}
		}
		if (emId == EnemyData::Vergil || emId == 42)//Vergil
		{
			auto airRaidCtrl = *(uintptr_t*)(enemyToTrick + 0x1C90);
			if (airRaidCtrl == 0 || *(bool*)(airRaidCtrl + 0x30))//isOutOfArea
			{
				_mod->_isDefaultFinishRangeRequested = true;
				return;
			}
		}
	}
	
	bool isDoppel = *(bool*)(vergil + 0x17F0);
	gf::Vec3 targetPos = *(gf::Vec3*)(trickAction + 0xF0);
	float xTmp, yTmp;

	void *transform = (void*)(*(uintptr_t*)(vergil + 0x1F0));
	auto pPos = *(gf::Vec3*)((uintptr_t)transform + 0x30);

	gf::Vec3 trickVec(targetPos.x - pPos.x, targetPos.y - pPos.y, targetPos.z - pPos.z);
	float trickVecLen = gf::Vec3::vec_length(pPos, targetPos);
	TeleportType curType = _mod->trickType;

	bool isWallEnemy = false;
	float correctionTmp = trickCorrection;
	if(enemyToTrick != 0 && shellToTrick == 0)
	{
		if (emId == EnemyData::Urizen1 /*|| *(int*)((uintptr_t)enemyToTrick + 0xB18) == 25*/) //fuck nidhogg still go through the invisible wall
		{
			isWallEnemy = true;
			curType = Front;
			correctionTmp = 4.9f;
		}
	}

	if (curType == Dynamic)
	{
		if (_mod->_inputSystem != nullptr)
		{
			try
			{
				const bool isTrickPressed = _mod->_inputSystem->is_action_button_pressed(*(uintptr_t*)(vergil + 0xEF0), InputSystem::PadInputGameAction::Special0);
				if (isDoppel && isDoppelOppositeTeleport)
				{
					if (isTrickPressed)
						curType = Front;
					else curType = Behind;
				}
				else
				{
					if (isTrickPressed)
						curType = Behind;
					else curType = Front;
				}
				
			}
			catch (const std::exception &exc)
			{
				curType = Front;
			}
		}
	}

	if (isDoppel && isDoppelOppositeTeleport)
	{
		if (_mod->trickType == Front)
			curType = Behind;
		else if (_mod->trickType == Behind)
			curType = Front;
	}
	gf::Vec3 tmpPos = targetPos;
	_mod->pos_teleport(curType, tmpPos, pPos, trickVec, correctionTmp, trickVecLen);
	targetPos.z += _mod->colliderZUp;
	
	if (curType == Behind && isAutoRotate)
	{
		gf::Quaternion reverse(0, 0, 1.0f, 0);
		*(gf::Quaternion*)((uintptr_t)transform + 0x40) *= reverse;
	}

	if(!isWallEnemy)
		gf::Transform_SetPosition::set_character_pos(vergil, targetPos, true);
	else
		gf::Transform_SetPosition::set_character_pos(vergil, tmpPos, true);
	targetPos.z -= _mod->colliderZUp;
	targetPos.z += _mod->teleportZOffs;
	targetPos.x = tmpPos.x;
	targetPos.y = tmpPos.y;

	if (forceGroundTrick && shellToTrick == 0)
	{
		if (enemyToTrick != 0)
		{
			if (_mod->groundTrickType == Default)
			{
				auto adjustTerrain = *(uintptr_t*)(enemyToTrick + 0x310);
				if (emId == EnemyData::Urizen1 || (adjustTerrain != 0 && !(*(bool*)(adjustTerrain + 0x82)) && emId != EnemyData::GreenEmpusa && emId != EnemyData::Lusachia && 
					emId != EnemyData::DeathScissors && emId != EnemyData::Hellbat && emId != EnemyData::Pyrobat && emId != EnemyData::Artemis && emId != EnemyData::Griffon && 
					emId != EnemyData::PhantomArtemis && emId != EnemyData::QliphotsTentacle && emId != EnemyData::Malphas && emId != EnemyData::Dante && emId != EnemyData::Vergil && emId != 42 && emId != 56))
					targetPos.z = (*(gf::Vec3*)(enemyToTrick + 0x3E0)).z - 0.6f;
			}
			else
				targetPos.z = (*(gf::Vec3*)(enemyToTrick + 0x3E0)).z - 0.6f;
		}
	}
	if (!isWallEnemy)
	{
		gf::PositionErrorCorrector setPos { (void*)*(uintptr_t*)(vergil + 0x8E8) };
		setPos.set_position(targetPos);
		/*gf::Transform_SetPosition setPlPos{ transform };
		setPlPos(targetPos);*/
	}
	else
	{
		gf::Transform_SetPosition setPos{transform};
		setPos(targetPos);
	}
	//*(bool*)(trickAction + 0x144) = true; //isPushHit;
	*(bool*)(trickAction + 0x146) = true; //isInterrupted;
	_mod->_isDefaultFinishRangeRequested = false;
}

static naked void speed_acc_detour() {
  __asm {
		cmp byte ptr[VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr[VergilAirTrick::isTeleport], 1
		je originalcode//teleport
		cmp byte ptr [VergilAirTrick::isSpeedUp], 0
		je originalcode

		cheat:
		movss xmm1, [VergilAirTrick::modSpeedAcc]
		jmp qword ptr [VergilAirTrick::speedAccRet]

		originalcode:
		movss xmm1, [rsi+0x000000A4]
		jmp qword ptr [VergilAirTrick::speedAccRet]

		teleport:
		movss xmm1, dword ptr [VergilAirTrick::modSpeedAcc]
		jmp qword ptr [VergilAirTrick::speedAccRet]

  }
}

static naked void maxspeedz_detour() {
	__asm {
		cmp byte ptr [VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr [VergilAirTrick::isTeleport], 1
		je originalcode
		cmp byte ptr [VergilAirTrick::isSpeedUp], 0
		je originalcode

		cheat:
		movss xmm0, [VergilAirTrick::modMaxSpeedZ]
		jmp qword ptr [VergilAirTrick::maxSpeedZRet]

		originalcode:
		movss xmm0, [rsi+0x000000A8]
		jmp qword ptr [VergilAirTrick::maxSpeedZRet]
  }
}

static naked void max_xz_detour() {
	__asm {
		cmp byte ptr [VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr[VergilAirTrick::isTeleport], 1
		je originalcode
		cmp byte ptr [VergilAirTrick::isSpeedUp], 0
		je originalcode

		cheat:
		movss xmm2,[VergilAirTrick::modMaxXZ]
		jmp qword ptr [VergilAirTrick::maxXZRet]

		originalcode:
		movss xmm2,[rsi+0x000000B0]
		jmp qword ptr [VergilAirTrick::maxXZRet]
  }
}

static naked void air_trick_routine3_detour()
{
	__asm {
		cmp byte ptr [VergilAirTrick::cheaton], 0
		je originalcode
		cmp byte ptr [VergilAirTrick::isTeleport], 0
		je originalcode

		cheat:
		je originalcode
		push rax
		push rbx
		push rcx
		push rdx
		push rdi
		push rsp
		push r8
		push r9
		push r10
		push r11
		mov rcx, rdi
		sub rsp, 32
		call qword ptr [VergilAirTrick::change_pos_asm]
		add rsp, 32
		pop r11
		pop r10
		pop r9
		pop r8
		pop rsp
		pop rdi
		pop rdx
		pop rcx
		pop rbx
		pop rax

		originalcode:
		mov dword ptr [rdi + 0x68], 0x3
		jmp qword ptr [VergilAirTrick::routineStartRet]
	}
}

float VergilAirTrick::trick_action_get_range_hook(uintptr_t threadCntx, uintptr_t fsm2TrickAction)
{
	if (!cheaton || !isTeleport)
		return _mod->m_finish_range_hook->get_original<decltype(trick_action_get_range_hook)>()(threadCntx, fsm2TrickAction);
	if (isTeleport)
	{
		if (!_mod->_isIgnoringTrickFinishRange || _mod->_isDefaultFinishRangeRequested)
			return trickCorrection;
		else return 0;
	}
}

void VergilAirTrick::on_push_hit_hook(uintptr_t threadCntx, uintptr_t fsm2TrickAction, uintptr_t gameObj)
{
	if (!cheaton || !isTeleport)
		_mod->m_on_push_hit_hook->get_original<decltype(on_push_hit_hook)>()(threadCntx, fsm2TrickAction, gameObj);
	if (isTeleport)
	{
		if (!_mod->_isIgnoringTrickFinishRange || _mod->_isDefaultFinishRangeRequested)
			return _mod->m_on_push_hit_hook->get_original<decltype(on_push_hit_hook)>()(threadCntx, fsm2TrickAction, gameObj);
	}
}

std::optional<std::string> VergilAirTrick::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = Page_VergilTrick;
	m_full_name_string = "Air Trick Settings (+)";
	m_author_string = "V.P.Zadov";
	m_description_string = "Adjust the properties of Vergil's Trick Actions.";

	set_up_hotkey();

	_inputSystem = static_cast<InputSystem*>(g_framework->get_mods()->get_mod("InputSystem"));
	_mod = this;

	auto waitTimeAddr = m_patterns_cache->find_addr(base, "07 00 00 8B 86 B8 00 00 00"); //DevilMayCry5.exe+1DDCB5D
    if (!waitTimeAddr) {
          return "Unanable to find AirTrick.waitTime pattern.";
    }

	auto finishOffsetAddr = m_patterns_cache->find_addr(base, "60 F3 0F 10 86 D0 00 00 00"); //DevilMayCry5.exe+1DDC4FF
    if (!waitTimeAddr) {
          return "Unanable to find AirTrick.finishOffsetAddr pattern.";
    }

	auto initSpeedAddr = m_patterns_cache->find_addr(base, "08 00 00 8B 86 A0 00 00 00"); //DevilMayCry5.exe+1DDCB43
    if (!initSpeedAddr) {
          return "Unanable to find AirTrick.initSpeedAddr pattern.";
    }

	auto maxSpeedZAddr = m_patterns_cache->find_addr(base, "0A 00 00 F3 0F 10 86 A8 00 00 00"); //DevilMayCry5.exe+1DDD585
    if (!maxSpeedZAddr) {
          return "Unanable to find AirTrick.maxSpeedZAddr pattern.";
    }

	auto maxXZAddr = m_patterns_cache->find_addr(base, "06 00 00 F3 0F 10 96 B0 00 00 00"); //DevilMayCry5.exe+1DDCCC5
    if (!maxXZAddr) {
          return "Unanable to find AirTrick.maxXZAddr pattern.";
    }

	auto speedAccAddr = m_patterns_cache->find_addr(base, "F3 0F 10 8E A4 00 00 00 0F 5A C0 0F 5A C9 48 85 C0 0F 84 40"); //DevilMayCry5.exe+1DDD539
    if (!speedAccAddr) {
          return "Unanable to find AirTrick.speedAccAddr pattern.";
    }

	auto routine3Addr = m_patterns_cache->find_addr(base, "03 00 00 C7 47 68 03 00 00 00"); //DevilMayCry5.exe+1DE0427
	if (!routine3Addr)
	{
		return "Unanable to find AirTrick.routine3Addr pattern.";
	}

	//auto finishRangeAddr = m_patterns_cache->find_addr(base, "F3 41 0F 10 86 AC 00 00 00"); //DevilMayCry5.exe+1DDE979
	//if (!routine3Addr)
	//{
	//	return "Unanable to find AirTrick.finishRangeAddr pattern.";
	//}
	auto finishRangeAddr = m_patterns_cache->find_addr(base, "5B C3 40 53 55 41 56");
	//DevilMayCry5.app_fsm2_player_pl0800_TrickAction__getRange312704 (-0x2)

	auto pushHitAddr = m_patterns_cache->find_addr(base, "40 53 55 41 56 48 83 EC 20 49 8B E8 4C 8B F2 48 8B 52");
	//DevilMayCry5.app_fsm2_player_pl0800_TrickAction__onPushHit312697

	if (!install_hook_absolute(initSpeedAddr.value()+0x3, m_initspeed_hook, &initspeed_detour, &initSpeedRet, 0x6)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.initSpeed"; 
    }

	if (!install_hook_absolute(waitTimeAddr.value()+0x3, m_waittime_hook, &waittime_detour, &waitTimeRet, 0x6)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.waitTime"; 
    }

	if (!install_hook_absolute(finishOffsetAddr.value()+0x1, m_finish_offset_hook, &finish_offset_detour, &finishOffsetRet, 0x18)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.finishOffset"; 
    }

	if (!install_hook_absolute(maxSpeedZAddr.value()+0x3, m_maxspeed_z_hook, &maxspeedz_detour, &maxSpeedZRet, 0x8)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.maxSpeedZ"; 
    }

	if (!install_hook_absolute(maxXZAddr.value()+0x3, m_max_xz_hook, &max_xz_detour, &maxXZRet, 0x8)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.maxXZ"; 
    }

	if (!install_hook_absolute(speedAccAddr.value(), m_speed_acc_hook, &speed_acc_detour, &speedAccRet, 0x8)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.speedAcc"; 
    }

	if (!install_hook_absolute(routine3Addr.value()+0x3, m_teleport_hook, &air_trick_routine3_detour, &routineStartRet, 0x7)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.routine3"; 
    }

	/*if (!install_hook_absolute(finishRangeAddr.value(), m_finish_range_hook, &finish_range_detour, &finishRangeRet, 0x9)) {
      spdlog::error("[{}] failed to initialize", get_name());
          return "Failed to initialize VergilAirTrick.finishRange"; 
    }*/

	m_finish_range_hook = std::make_unique<FunctionHook>(finishRangeAddr.value() + 0x2, &trick_action_get_range_hook);
	if (!m_finish_range_hook->create())
		return "Failed to initialize VergilAirTrick.m_finish_range_hook";

	m_on_push_hit_hook = std::make_unique<FunctionHook>(pushHitAddr.value(), &on_push_hit_hook);
	if (!m_on_push_hit_hook->create())
		return "Failed to initialize VergilAirTrick.m_on_push_hit_hook";
	return Mod::on_initialize();
}

void VergilAirTrick::on_config_load(const utility::Config& cfg)
{
	initSpeed = cfg.get<float>("VergilAirTrick.initSpeed").value_or(3.0f);
	finishOffsetZ = cfg.get<float>("VergilAirTrick.finishOffsetZ").value_or(1.95f);
	waitTime = cfg.get<float>("VergilAirTrick.waitTime").value_or(0.0f);
	isSpeedUp = cfg.get<bool>("VergilAirTrick.isSpeedUp").value_or(true);
	isCustomWaitTime = cfg.get<bool>("VergilAirTrick.isCustomWaitTime").value_or(true);
	isCustomOffset = cfg.get<bool>("VergilAirTrick.isCustomOffset").value_or(false);
	isTeleport = cfg.get<bool>("VergilAirTrick.isTeleport").value_or(false);
	isDoppelOppositeTeleport = cfg.get<bool>("VergilAirTrick.isDoppelOppositeTeleport").value_or(false);
	forceGroundTrick = cfg.get<bool>("VergilAirTrick.forceGroundTrick").value_or(false);
	isAutoRotate = cfg.get<bool>("VergilAirTrick.isAutoRotate").value_or(true);
	_isIgnoringTrickFinishRange = cfg.get<bool>("VergilAirTrick._isIgnoringTrickFinishRange").value_or(false);
	trickType = (TeleportType)cfg.get<int>("VergilAirTrick.trickType").value_or(Dynamic);
	trickCorrection = cfg.get<float>("VergilAirTrick.trickCorrection").value_or(1.8f);
	teleportZOffs = cfg.get<float>("VergilAirTrick.teleportZOffs").value_or(-1.3f);
	groundTrickType = (GroundTrickType)cfg.get<int>("VergilAirTrick.groundTrickType").value_or(GroundTrickType::Default);
}

void VergilAirTrick::on_config_save(utility::Config& cfg)
{
	cfg.set<float>("VergilAirTrick.initSpeed", initSpeed);
	cfg.set<float>("VergilAirTrick.finishOffsetZ", finishOffsetZ);
	cfg.set<float>("VergilAirTrick.waitTime", waitTime);
	cfg.set<bool>("VergilAirTrick.isCustomWaitTime", isCustomWaitTime);
	cfg.set<bool>("VergilAirTrick.isSpeedUp", isSpeedUp);
	cfg.set<bool>("VergilAirTrick.isCustomOffset", isCustomOffset);
	cfg.set<bool>("VergilAirTrick.isTeleport", isTeleport);
	cfg.set<bool>("VergilAirTrick.forceGroundTrick", forceGroundTrick);
	cfg.set<bool>("VergilAirTrick._isIgnoringTrickFinishRange", _isIgnoringTrickFinishRange);
	cfg.set<bool>("VergilAirTrick.isAutoRotate", isAutoRotate);
	cfg.set<float>("VergilAirTrick.teleportZOffs", teleportZOffs);
	cfg.set<int>("VergilAirTrick.trickType", (int)trickType);
	cfg.set<float>("VergilAirTrick.trickCorrection", trickCorrection);
	cfg.set<float>("VergilAirTrick.teleportZOffs", teleportZOffs);
	cfg.set<int>("VergilAirTrick.groundTrickType", (int)groundTrickType);
}

// void VergilAirTrick::on_frame(){}

void VergilAirTrick::on_draw_ui()
{
	ImGui::Checkbox("Change Air Trick startup.", &isCustomWaitTime);
	if (isCustomWaitTime) {
          ImGui::TextWrapped("Controls the delay before trick movement starts. 6 - default game value.");
          UI::SliderFloat("##WaitTimeSlider", &waitTime, 0.0f, 20.0f, "%.1f");
	}
	ImGui::Separator();

	if (!isTeleport)
	{
		ImGui::TextWrapped("Removes Velocity & Acceleration limits on teleportation movement.");
		ImGui::Checkbox("Unclamp Air Trick Speed", &isSpeedUp);
		if (isSpeedUp)
		{
			ImGui::TextWrapped("Controls the speed of the Air Trick movement. High values can cause glitches. 0.7 - default game value.");
			UI::SliderFloat("Air Trick speed", &initSpeed, 0.7f, 3.0f, "%.1f");
		}
		ImGui::Separator();
	}
	
	ImGui::TextWrapped("Vergil teleports directly to the target instead of moving through the air invisibly. Can send you out of bounds." //  Doesn't work with Vergil or Death Scrissors when they are out of bounds. " I think this goes without saying? maybe not.
	"Doesn't work with Nidhogg.");
	ImGui::Checkbox("Instant Transmission", &isTeleport);
	if (isTeleport)
	{
		ImGui::Spacing();
		ImGui::TextWrapped("Teleport type:");
		ImGui::RadioButton("Front trick", (int*)&trickType, 0);
		ImGui::ShowHelpMarker("Vergil appears in front of the enemy relative to the trick start position.");
		ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
		ImGui::RadioButton("Reverse trick", (int*)&trickType, 1);
		ImGui::ShowHelpMarker("Vergil appears behind the enemy relative to the trick start position.");
		ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
		ImGui::RadioButton("Dynamic trick", (int*)&trickType, 2);
		ImGui::ShowHelpMarker("Hold the trick button when Vergil starts teleporting to perform a reverse trick, otherwise perform a front trick. Requires some startup delay to use.");
		ImGui::Checkbox("Teleport doppelganger to opposite side", &isDoppelOppositeTeleport);
		ImGui::Checkbox("Rotate after backstep trick", &isAutoRotate);
		ImGui::ShowHelpMarker("Vergil automatically rotates to face the enemy target before appearing");

		ImGui::Checkbox("Ignore close-range trick interruption", &_isIgnoringTrickFinishRange);
		ImGui::ShowHelpMarker("By default trick action gets interrupted when you are too close to enemy and you can't do reverse trick in that case. Enable this option to remove range restriction while"
			"\"Instant Transmission\" is on.");

		ImGui::TextWrapped("Distance from enemy:"); //redo this text at some point
		UI::SliderFloat("##CorrectionFinishOffset", &trickCorrection, 0.25f, 2.25f, "%.2f", 1.0F, ImGuiSliderFlags_None);
		ImGui::TextWrapped("Height above/below enemy."); //redo this text at some point
		UI::SliderFloat("##CorrectionFinishZOffset", &teleportZOffs, -1.5f, 3.25f, "%.2f", 1.0F, ImGuiSliderFlags_None);
		ImGui::Checkbox("Force ground trick", &forceGroundTrick);
		ImGui::ShowHelpMarker("Vergil always appears on the ground after teleporting to enemies like Empusa Queen, Goliath, Baphomet when they are not in the air.");
		if (forceGroundTrick)
		{
			ImGui::TextWrapped("Ground trick type:");
			ImGui::RadioButton("Default", (int*)&groundTrickType, Default);
			ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
			ImGui::RadioButton("Always ground trick", (int*)&groundTrickType, AlwaysGround);
			ImGui::ShowHelpMarker("Vergil always appears on the ground. Try using this with the \"boss trick up\" mod.");
		}		
	}

	if (!isTeleport)
	{
		ImGui::Checkbox("Custom height offset when finishing Air Trick", &isCustomOffset);
		if (isCustomOffset)
			UI::SliderFloat("Height offset", &finishOffsetZ, 0.0f, 3.0f, "%.1f");
	}
}

// void VergilAirTrick::on_draw_debug_ui(){}

void VergilAirTrick::init_check_box_info()
{
	m_check_box_name = m_prefix_check_box_name + std::string(get_name());
	m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}

