#pragma once
#include "Mod.hpp"
#include "GameFunctions/GameFunc.hpp"
#include "PlayerTracker.hpp"
#include "MissionManager.hpp"

//clang-format off

namespace gf = GameFunctions;

class EnemyFixes : public Mod
{
private:
	std::unique_ptr<FunctionHook> m_spawn_pos_hook;
	std::unique_ptr<FunctionHook> m_m19check_hook;
	std::unique_ptr<FunctionHook> m_griffon_hook;
	std::unique_ptr<FunctionHook> m_shadow_hook;
	std::unique_ptr<FunctionHook> m_nightmire_starting_hook;
	std::unique_ptr<FunctionHook> m_nightmire_arrival_hook;
	std::unique_ptr<FunctionHook> m_cavfix_hook;
	std::unique_ptr<FunctionHook> m_vergilcenterfloor_hook;
	std::unique_ptr<FunctionHook> m_airraid_controller_hook;
	std::unique_ptr<FunctionHook> m_goliath_suctionjmp_hook;
	std::unique_ptr<FunctionHook> m_goliath_leavejmp_hook;
	std::unique_ptr<FunctionHook> m_artemis_centerfloor_hook;
	std::unique_ptr<FunctionHook> m_urizen3_tp_hook;
	std::unique_ptr<FunctionHook> m_malphas_tp_hook;
	std::unique_ptr<FunctionHook> m_cerberus_pos_hook;
	std::unique_ptr<FunctionHook> m_cerberus_thunderwave_hook;
	std::unique_ptr<FunctionHook> m_cerberus_thunderball_hook;
	std::unique_ptr<FunctionHook> m_shadow_warp_func_hook;
	std::unique_ptr<FunctionHook> m_nightmirepos_ext_far_hook;
	std::unique_ptr<FunctionHook> m_timehorse_center_hook;

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	std::unique_ptr<FunctionHook> m_trickup_action_hook;

    static void btn_set_plpos_to(Vector3f& to, const char* btnContent = "Set center of floor to current player position")
    {
        if (ImGui::Button(btnContent))
        {
            if (GameplayStateTracker::nowFlow == 22)
                to = CheckpointPos::get_player_coords();
        }
    }

    static inline void update_spawn(uintptr_t generateData, bool isFlyingEm)
    {
        if (isSpawnOffsForFlyingEnemiesOnly && !isFlyingEm)
            return;
        if (isAlwaysSpawnOnPlPos)
            *(gf::Vec3*)(generateData + 0x30) = *(gf::Vec3*)(PlayerTracker::playertransform + 0x30);
        gf::Vec3 posOffs{ spawnPosXOffset, spawnPosYOffset, spawnPosZOffset };
        *(gf::Vec3*)(generateData + 0x30) += posOffs;
    }

    static void update_spawn_pos_asm(uintptr_t generateData, uintptr_t posData)
	{
		if(generateData == 0 || posData == 0)
			return;
		gf::Vec3 posOffs { spawnPosXOffset, spawnPosYOffset, spawnPosZOffset };
        auto idInfo = *(uintptr_t*)(generateData + 0x60);
        int emId = *(int*)(idInfo + 0x10);
        bool isFlyingEm = emId == 4 || emId == 10 || emId == 11 || emId == 16 || emId == 17 || emId == 18 || emId == 23 || emId == 37 || emId == 29; //Id for flying enemies
        update_spawn(generateData, isFlyingEm);
		if (isForceVerticalSpawnRot)
		{
			auto appear = emId >= 23 ? AppearNoEnum::BossGeneral0 : selectedAppear->n;
			gf::Quaternion rot{0, 0, 0, 1.0f};
			*(gf::Quaternion*)(generateData + 0x40) = rot;
			*(bool*)(generateData + 0x50) = true;//isPlayerDirection
			*(bool*)(posData + 0x1C) = true;//isPlayerDirection
			*(AppearNoEnum*)(posData + 0x18) = appear;
		}
	}

	static naked void spawn_pos_detour() 
	{
		__asm {
			cmp byte ptr [EnemyFixes::cheaton], 0
			je originalcode
			cmp byte ptr [EnemyFixes::isCustomSpawnPos], 00
			je originalcode

			cheat:
			push rax
			push rbx
			push rcx
			push rdx
			push rdi
			push rsi
			push rsp
			push r8
			push r9
			push r10
			push r11
			push r13
			mov rcx, r13
			mov rdx, rsi
			sub rsp, 32
			call qword ptr [EnemyFixes::update_spawn_pos_asm]
			add rsp, 32
			pop r13
			pop r11
			pop r10
			pop r9
			pop r8
			pop rsp
			pop rsi
			pop rdi
			pop rdx
			pop rcx
			pop rbx
			pop rax

			originalcode:
			test rcx,rcx
			jne shortjne
			jmp qword ptr [EnemyFixes::posSpawnRet]

			shortjne:
			jmp qword ptr [EnemyFixes::posSpawnTestJne]

	  }
	}

	static naked void load_Dante_ai_detour() 
	{
		__asm {
			cmp byte ptr [EnemyFixes::cheaton], 00
			je originalcode
			cmp byte ptr [EnemyFixes::isBossDanteAiEnabled], 00
			je originalcode

			cheat:
			cmp dword ptr [rax+0x7C], 0x16 //22 - Training mode. No pause menu allowed if change
			je originalcode
			cmp byte ptr [EnemyFixes::isDanteM20], 01
			je dante_m20
			mov dword ptr [rax+0x7C], 0x13

			originalcode:
			cmp dword ptr [rax+0x7C], 0x13
			je ret_je
			jmp qword ptr [EnemyFixes::bossDanteAiRet]

			dante_m20:
			mov dword ptr [rax+0x7C], 0x14
			jmp originalcode

			ret_je:
			jmp qword ptr [EnemyFixes::bossDanteAiJne]
	  }
	}

	static naked void kill_griffon_detour() 
	{
    __asm 
		{
        cmp byte ptr [EnemyFixes::cheaton], 00
        je originalcode
        cmp dword ptr [MissionManager::missionNumber], 0x12
        je originalcode
        jmp qword ptr [EnemyFixes::killGriffonRet]

        originalcode:
        cmp byte ptr [rdi+0x00000F48],00
        jmp qword ptr [EnemyFixes::killGriffonRet]
		}
	}

	static naked void kill_shadow_detour()
	{
	  __asm {
			cmp byte ptr [EnemyFixes::cheaton], 00
			je originalcode
			cmp dword ptr [MissionManager::missionNumber], 0x12
			je originalcode
			jmp qword ptr [EnemyFixes::killShadowRet]

			originalcode:
			cmp byte ptr [rdi+0x00000F60], 00
			jmp qword ptr [EnemyFixes::killShadowRet]
	  }
	}

	static naked void nightmire_starting_detour() 
    {
    __asm {
        cmp byte ptr [EnemyFixes::cheaton], 00
        je originalcode
        cmp byte ptr [EnemyFixes::isNightmareFix], 00
        je originalcode

        cheat:
        push r10
        /*mov r10, [EnemyFixes::plPosBase]
        mov r10, [r10]
        mov r10, [r10+0x70]
        mov r10, [r10+0xC70]
        mov r10, [r10+0x40]
        mov r10, [r10+0xE40]
        mov r10, [r10+0x80]*/
		mov r10, [PlayerTracker::playertransform]
        movss xmm0, [r10+0x30]//plCoords
        movss xmm1, [r10+0x34]
        movss xmm2, [r10+0x38]
        pop r10
        addss xmm0, [EnemyFixes::nightmareStartPosOffs.x]
        addss xmm1, [EnemyFixes::nightmareStartPosOffs.z]
        addss xmm2, [EnemyFixes::nightmareStartPosOffs.y]

        originalcode:
        mov rdx, rdi
        movss [rbp-0x69], xmm0
        jmp qword ptr [EnemyFixes::nightmareStartingPosRet]
  }
}

	static naked void nightmire_arrival_detour() 
    {
    __asm {
        cmp byte ptr [EnemyFixes::cheaton], 00
        je originalcode
		cmp byte ptr [EnemyFixes::isNightmareFix], 00
		je originalcode

        cheat:
        push r10
		mov r10, [PlayerTracker::playertransform]
        movss xmm10, [r10+0x30] // plCoords
        movss xmm11, [r10+0x34]
        movss xmm12, [r10+0x38]
        pop r10

        originalcode:
        movss [rax+0x30], xmm10
        jmp qword ptr [EnemyFixes::nightmareArrivalPosRet]
      }
    }

	static naked void cavtele_detour()//need to find where it calc distance
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode
		    cmp byte ptr [EnemyFixes::isCavFixEnabled], 00
		    je originalcode

            cheat:
            //--------PlayerPos--------//
            push r10
            mov r10, [PlayerTracker::playertransform]
            //--------PlayerPos--------//
            movss xmm0, [r10+0x30]
            addss xmm0, [EnemyFixes::cavOffset.x]
            movss xmm1, [r10+0x34]
            addss xmm1, [EnemyFixes::cavOffset.z]
            movss xmm2, [r10+0x38]
            addss xmm2, [EnemyFixes::cavOffset.y]
            pop r10
            movss [rdi+0x10], xmm0
            movss [rdi+0x14], xmm1
            movss [rdi+0x18], xmm2

            originalcode:
            //movss [r13+0x00000E20], xmm0
            movss xmm0, [rdi+0x10]
            jmp qword ptr [EnemyFixes::cavFixRet]
        }
    }

	static naked void vergil_centerfloor_detour()//for dive bomb
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode
            cmp byte ptr [EnemyFixes::isVergilFixEnabled], 00
            je originalcode

            cheat:
            movss xmm0, [rbx+0x3E0]
            movss xmm1, [rbx+0x3E4]
            movss xmm2, [rbx+0x3E8]
            push rbx
            mov rbx, rax
            mov rbx, [rbx+0x108]//AirRaid
            //push r8
            movss xmm3, [EnemyFixes::divebombHeightOfArenaSide]
            movss xmm4, [EnemyFixes::divebombHeightOfOutside]
            movss xmm5, [EnemyFixes::divebombDistanceGroundCheck]
            addss xmm3, xmm1
            addss xmm4, xmm1
            addss xmm5, xmm1

            /*mov r8, [EnemyFixes::divebombHeightOfOutside]
            mov [rbx+0x7C], r8
            mov r8, [EnemyFixes::divebombHeightOfArenaSide]
            mov [rbx+0x78], r8
            mov r8, [EnemyFixes::divebombDistanceGroundCheck]
            mov [rbx+0x80], r8*/

            movss dword ptr [rbx+0x78], xmm3
            movss dword ptr[rbx+0x7C], xmm4
            movss dword ptr[rbx+0x80], xmm5

           // pop r8
            pop rbx
            movss [rax+0x000000A0], xmm0
            movss [rax+0x000000A4], xmm1
            movss [rax+0x000000A8], xmm2
            jmp qword ptr [EnemyFixes::vergilFixJs]

            originalcode:
            movss xmm0, [rax+0x000000A0]
            jmp qword ptr [EnemyFixes::vergilFixRet]
        }
    }

	static naked void airraid_detour()
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode
            cmp byte ptr [EnemyFixes::isVergilFixEnabled], 00
            je originalcode

            cheat:
            cmp byte ptr [EnemyFixes::isFastDiveBombAttack], 0
            je originalcode
            cmp byte ptr [rsi+0x32], 1
            je originalcode
            cmp byte ptr [rsi+0x31], 0
            je originalcode
            mov byte ptr [rsi+0x32], 1

            originalcode:
            movss xmm7, [rax+0x000000A0]
            jmp qword ptr [EnemyFixes::airRaidControllerRet]
        }
    }

	static naked void goliath_jmpsuck_detour()
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode
            option_check:
            cmp byte ptr [EnemyFixes::isGoliathFixEnabled], 00
            je originalcode

            cheat:
            //--------PlayerPos--------//
            push rax
            mov rax, [PlayerTracker::playertransform]
            //--------PlayerPos--------//
            movss xmm0, [rax+0x30]
            movss xmm1, [rax+0x34]
            movss xmm2, [rax+0x38]
            pop rax

            originalcode:
            movss [rdx+0x00000D40], xmm0
            jmp qword ptr [EnemyFixes::goliathSuckJmpRet]
        }
    }

	static naked void goliath_jmpleave_detour()
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode
            option_check:
            cmp byte ptr [EnemyFixes::isGoliathFixEnabled], 00
            je originalcode

            cheat:
            //--------PlayerPos--------//
            push rax
            mov rax, [PlayerTracker::playertransform]
            //--------PlayerPos--------//
            movss xmm0, [rax+0x30]
            movss xmm1, [rax+0x34]
            movss xmm2, [rax+0x38]
            pop rax

            originalcode:
            movss [r14+0x00000D30], xmm0
            jmp qword ptr [EnemyFixes::goliathLeaveJmpRet]
        }
    }

    static naked void artemis_centerfloor_fix()
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode
            cmp byte ptr [EnemyFixes::isArtemisFixEnabled], 00
            je originalcode

            cheat:
            cmp byte ptr [EnemyFixes::_isArtemisInitPos], 1
            je initpos
            mov eax, dword ptr [EnemyFixes::artemisCenterOfFloor.z]
            mov [rcx+0x14], eax
            cmp byte ptr [EnemyFixes::isArtemisPlayersXY], 1
            je plposload
            mov eax, dword ptr [EnemyFixes::artemisCenterOfFloor.x]
            mov [rcx+0x10], eax
            mov eax, dword ptr [EnemyFixes::artemisCenterOfFloor.y]
            mov [rcx+0x18], eax
            jmp originalcode

            initpos:
            movss xmm3, dword ptr [r8+0x140]
            movss xmm4, dword ptr[r8+0x144]
            addss xmm4, dword ptr [EnemyFixes::_artemisZInitOffs]
            movss xmm5, dword ptr [r8+0x148]
            movss dword ptr [rcx + 0x10], xmm3
            movss dword ptr [rcx + 0x14], xmm4
            movss dword ptr [rcx + 0x18], xmm5
            jmp originalcode

            plposload:
             //--------PlayerPos--------//
            mov rax, [PlayerTracker::playertransform]
            //--------PlayerPos--------//
            push rbx
            mov ebx, dword ptr [rax+0x30]
            mov [rcx+0x10], ebx
            mov ebx, dword ptr [rax+0x38]
            mov [rcx+0x18], ebx
            pop rbx

            originalcode:
            movss xmm3, [rcx + 0x10]
            jmp qword ptr [EnemyFixes::artemisFixRet]
        }
    }

    static naked void urizen3tp_detour()
{
    __asm {
        cmp byte ptr [EnemyFixes::cheaton], 00
        je originalcode
        cmp byte ptr [EnemyFixes::isUrizen3FixEnabled], 00
        je originalcode

        cheat:
        jmp qword ptr [EnemyFixes::urizen3TpJne]

        originalcode:
        cmp qword ptr [rax+0x18], 00
        jne cheat
        jmp qword ptr [EnemyFixes::urizen3TpRet]
    }
}

    static naked void malphas_tp_detour()
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode
            cmp byte ptr [EnemyFixes::isMalphasFixEnabled], 00
            je originalcode

            cheat:
            cmp byte ptr [EnemyFixes::_isMalphasFixInitPos], 1
            je initpos
            cmp byte ptr [EnemyFixes::malphasFixPlPos], 00
            je load_pos

            //--------PlayerPos--------//
            push rax
            mov rax, [PlayerTracker::playertransform]
            //--------PlayerPos--------//
            movss xmm0, [rax+0x30]
            movss xmm1, [rax+0x34]
            movss xmm2, [rax+0x38]
            pop rax
            jmp xmm_to_addr

            initpos:
            push rax
            mov rax, [rdi + 0x28]
            movss xmm0, dword ptr [rax + 0x140]
            movss xmm1, dword ptr [rax + 0x144]
            movss xmm2, dword ptr [rax + 0x148]
            pop rax
            jmp xmm_to_addr

            load_pos:
            movss xmm0, dword ptr [EnemyFixes::malphasCenterOfFloor.x]
            movss xmm1, dword ptr [EnemyFixes::malphasCenterOfFloor.z]
            movss xmm2, dword ptr [EnemyFixes::malphasCenterOfFloor.y]
            
            xmm_to_addr:
            movss [rdi+0x00000090], xmm0
            movss [rdi+0x00000094], xmm1
            movss [rdi+0x00000098], xmm2

            originalcode:
            movss xmm0, [rdi + 0x00000090]
            jmp qword ptr [EnemyFixes::malphasRet]
        }
    }

    static naked void cerberus_pos_detour()
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode
            cmp byte ptr [EnemyFixes::isCerberusFixEnabled], 00
            je originalcode

            cheat:
            cmp byte ptr [EnemyFixes::_isCerberusFixInitPos], 01
            je initpos
            cmp byte ptr [EnemyFixes::cerberusFixPlPos], 00
            je load_pos
            //--------PlayerPos--------//
            push rax
            mov rax, [PlayerTracker::playertransform]
            //--------PlayerPos--------//
            movss xmm8, [rax+0x30]
            movss xmm9, [rax+0x34]
            movss xmm10, [rax+0x38]
            pop rax
            jmp xmm_to_addr

            initpos:
            movss xmm8, dword ptr [r8+0x140]
            movss xmm9, dword ptr [r8+0x144]
            movss xmm10, dword ptr [r8+0x148]
            jmp xmm_to_addr

            load_pos:
            movss xmm8, dword ptr [EnemyFixes::cerberusCenterOfFloor.x]
            movss xmm9, dword ptr [EnemyFixes::cerberusCenterOfFloor.z]
            movss xmm10, dword ptr [EnemyFixes::cerberusCenterOfFloor.y]

            xmm_to_addr:
            movss [rdx+0x10], xmm8
            movss [rdx+0x14], xmm9
            movss [rdx+0x18], xmm10

            originalcode:
            movss xmm10, [rdx+0x18]
            jmp qword ptr [EnemyFixes::cerberusFixRet]
        }
    }

    static naked void cerberus_thunderwave_detour()
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode
            cmp byte ptr [EnemyFixes::isCerberusFixEnabled], 00
            je originalcode

            cheat:
            movss xmm0, dword ptr [rsi + 0x3E4]

            //cmp byte ptr [EnemyFixes::cerberusThunderWavePlPos], 0
            //je thunderstatic
            ////--------PlayerPos--------//
            //push rax
            //mov rax, [PlayerTracker::playertransform]
            ////--------PlayerPos--------//
            //movss xmm0, [rax+0x34]
            //pop rax
            //jmp change

            //thunderstatic:
            //movss xmm0, [EnemyFixes::cerberusThunderWaveZ]

            change:
            movss [rdi+0x24], xmm0
            movss [rdi+0x28], xmm0
            jmp qword ptr [EnemyFixes::cerberusThunderWaveRet]

            originalcode:
            maxss xmm0, [rdi+0x24]
            jmp qword ptr [EnemyFixes::cerberusThunderWaveRet]
        }
    }

    static naked void cerberus_thunderball_detour()//fu, capcom
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode
            cmp byte ptr [EnemyFixes::isCerberusFixEnabled], 00
            je originalcode

            jmp qword ptr [EnemyFixes::cerberusThunderBallJmp]

            originalcode:
            cmp [rax+0x00000FA9], r14d//r14l
            jmp qword ptr [EnemyFixes::cerberusThunderBallRet]
        }
    }

    static naked void fsm_shadow_warp_to_center_detour()
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode

            cheat:
            push r13
            mov r13, [r12+0x60]//Shadow
            test r13, r13
            je badptr
            mov r13, [r13 + 0xD80]//CharParams
            test r13, r13
            je badptr
            mov r13, [r13 + 0x128]//BattleAreaCenterParam_E
            test r13, r13
            je badptr
            push rax
            mov rax, [PlayerTracker::playertransform]
            movss xmm0, [rax + 0x30]
            movss xmm1, [rax + 0x34]
            movss xmm2, [rax + 0x38]
            addss xmm1, [EnemyFixes::shadow_warp_offs_z]
            movss [r13 + 0x10], xmm0
            movss [r13 + 0x14], xmm1
            movss [r13 + 0x18], xmm2
            movss [r13 + 0x20], xmm0
            movss [r13 + 0x24], xmm1
            movss [r13 + 0x28], xmm2
            movss [r13 + 0x30], xmm0
            movss [r13 + 0x34], xmm1
            movss [r13 + 0x38], xmm2
            pop rax

            badptr:
            pop r13

            originalcode:
            mov rax, [rbx+0x50]
            cmp qword ptr[rax + 0x18], 00
            jmp qword ptr [EnemyFixes::shadowWarpFuncRet]
        }
    }

    static naked void nightmire_teleport_ext_far_detour()
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode

            cheat:
            push rax
            mov rax, [PlayerTracker::playertransform]
            test rax, rax
            je badptr
            movss xmm0, [rax + 0x30]
            movss xmm1, [rax + 0x34]
            movss xmm2, [rax + 0x38]
            badptr:
            pop rax

            originalcode:
            movss [rcx + 0x00000EF0], xmm0
            jmp qword ptr [EnemyFixes::nightmireExtFarPosRet]
        }
    }

    static naked void time_horse_detour()
    {
        __asm {
            cmp byte ptr [EnemyFixes::cheaton], 00
            je originalcode
            cmp byte ptr [EnemyFixes::isTimeHorseFixEnabled], 00
            je originalcode

            cheat:
            cmp byte ptr [_isTimeHorseFixInitPos], 01
            je initpos
            movss xmm8, [EnemyFixes::geryonCenter.x]
            movss [rdi + 0xDC0], xmm8
            movss xmm8, [EnemyFixes::geryonCenter.z]
            movss [rdi + 0xDC4], xmm8
            movss xmm8, [EnemyFixes::geryonCenter.y]
            movss [rdi + 0xDC8], xmm8

            initpos:
            movss xmm8, dword ptr [rdi+0x140]
            movss[rdi + 0xDC0], xmm8
            movss xmm8, dword ptr [rdi+0x144]
            movss[rdi + 0xDC4], xmm8
            movss xmm8, dword ptr [rdi+0x148]
            movss[rdi + 0xDC8], xmm8

            originalcode:
            movss xmm8, [rdi + 0x00000DC0]
            jmp qword ptr [EnemyFixes::timeHorseRet]
        }
    }

public:
	static inline uintptr_t posSpawnRet = 0;
	static inline uintptr_t posSpawnTestJne = 0;
	static inline uintptr_t bossDanteAiRet = 0;
	static inline uintptr_t bossDanteAiJne = 0;
	static inline uintptr_t killShadowRet = 0;
	static inline uintptr_t killGriffonRet = 0;
	static inline uintptr_t nightmareStartingPosRet = 0;
	static inline uintptr_t nightmareArrivalPosRet = 0;
	static inline uintptr_t plPosBase = 0;
	static inline uintptr_t cavFixRet = 0;
	static inline uintptr_t vergilFixRet = 0;
	static inline uintptr_t vergilFixJs = 0;
	static inline uintptr_t airRaidControllerRet = 0;
	static inline uintptr_t goliathSuckJmpRet = 0;
	static inline uintptr_t goliathLeaveJmpRet = 0;
	static inline uintptr_t artemisFixRet = 0;
	static inline uintptr_t urizen3TpRet = 0;
	static inline uintptr_t urizen3TpJne = 0;
	static inline uintptr_t malphasRet = 0;
	static inline uintptr_t cerberusFixRet = 0;
	static inline uintptr_t cerberusThunderWaveRet = 0;
	static inline uintptr_t cerberusThunderBallRet = 0;
	static inline uintptr_t cerberusThunderBallJmp = 0;
	static inline uintptr_t shadowWarpFuncRet = 0;
	static inline uintptr_t nightmireExtFarPosRet = 0;
	static inline uintptr_t timeHorseRet = 0;

    static inline bool isCustomSpawnPos = false;
	static inline bool isBossDanteAiEnabled = false;
	static inline bool isDanteM20 = false;
	static inline bool canKillShadow = false;
	static inline bool canKillGriffon = false;
	static inline bool isNightmareFix = false;
	static inline bool isCavFixEnabled = false;
	static inline bool isVergilFixEnabled = false;
	static inline bool isFastDiveBombAttack = false;
	static inline bool isGoliathFixEnabled = false;
	static inline bool isArtemisFixEnabled = false;
	static inline bool isArtemisPlayersXY = false;
    static inline bool _isArtemisInitPos = false;
	static inline bool isUrizen3FixEnabled = false;
	static inline bool isMalphasFixEnabled = false;
	static inline bool malphasFixPlPos = false;
    static inline bool _isMalphasFixInitPos = false;
	static inline bool isCerberusFixEnabled = false;
	static inline bool cerberusFixPlPos = false;
    static inline bool _isCerberusFixInitPos = false;
	static inline bool cerberusThunderWavePlPos = false;
	static inline bool isTimeHorseFixEnabled = false;
    static inline bool _isTimeHorseFixInitPos = false;
	static inline bool isSpawnOffsForFlyingEnemiesOnly = false;
	static inline bool isForceVerticalSpawnRot = false;
	static inline bool isAlwaysSpawnOnPlPos = false;
    static inline bool isAlwaysSpawnFlyingEmOnPlPos = false;
	static inline bool isFriendlyVergilAI = false;
    static inline bool isDoppelCameraFix = false;

	static inline constexpr float shadow_warp_offs_z = 0.85f;

	static inline float spawnPosZOffset = 0;
	static inline float spawnPosXOffset = 0;
	static inline float spawnPosYOffset = 0;
	static inline float curSpawnPosZ = 0;
	static inline float curSpawnPosX = 0;
	static inline float curSpawnPosY = 0;
	static inline float waitTimeMin = 0;
	static inline float waitTimeMax = 0;
	static inline float odds = 0;
    static inline float _artemisZInitOffs = 5.0f;
	static inline const float divebombDistanceGroundCheck = 5.0f; // default = 5
	static inline const float divebombHeightOfArenaSide = -1.5f; // default = 1.5
	static inline const float divebombHeightOfOutside = 8.0f;// default = 8
	//static float inline radiusOfArea = 15.0f;//43.0
	//static float inline radiusOfRevolution = 35.0f;//80
	static inline float cerberusThunderWaveZ = 0;

    static inline Vector3f nightmareStartPosOffs{135.83f, -112.45f, 82.784f};
	static inline Vector3f cavOffset{ 1.5f, 2.32f, -0.8f };
	static inline Vector3f artemisCenterOfFloor{ -368.0f, -308.5f, -10.35f };
	static inline Vector3f malphasCenterOfFloor{ 0.0f,0.0f,0.0f };
	static inline Vector3f cerberusCenterOfFloor{ 0.0f, -0.1f, 0.0f };
	static inline Vector3f geryonCenter{ 0.0f, -0.0f, 0.0f };

	

	enum AppearNoEnum : int32_t
	{
		None = -1,
		General0 = 0,
		General1 = 1,
		General2 = 2,
		General3 = 3,
		General4 = 4,
		General5 = 5,
		General6 = 6,
		Private0 = 100,
		Private1 = 101,
		Private2 = 102,
		Private3 = 103,
		Private4 = 104,
		Private5 = 105,
		Private6 = 106,
		Private7 = 107,
		Private8 = 108,
		Private9 = 109,
		Private10 = 110,
		Private11 = 111,
		Private12 = 112,
		Private13 = 113,
		Private14 = 114,
		Private15 = 115,
		Private16 = 116,
		Private17 = 117,
		Private18 = 118,
		Private19 = 119,
		Private20 = 120,
		BossGeneral0 = 200
	};

	struct AppearNoName
	{
		const char* name;
		const AppearNoEnum n;

		AppearNoName(const char* curName, AppearNoEnum num) : name(curName), n(num)
		{
		}

		static AppearNoName const* find_by_enum(AppearNoEnum n)
		{
			for (const auto& i : appearNoEnumNames)
			{
				if (n == i.n)
					return &i;
			}
		}
	};

	static inline const std::array<AppearNoName, 30> appearNoEnumNames = {
	  AppearNoName("None", None),
	  AppearNoName("General0", General0),
	  AppearNoName("General1", General1),
	  AppearNoName("General2", General2),
	  AppearNoName("General3", General3),
	  AppearNoName("General4", General4),
	  AppearNoName("General5", General5),
	  AppearNoName("General6", General6),
	  AppearNoName("Private0", Private0),
	  AppearNoName("Private1", Private1),
	  AppearNoName("Private2", Private2),
	  AppearNoName("Private3", Private3),
	  AppearNoName("Private4", Private4),
	  AppearNoName("Private5", Private5),
	  AppearNoName("Private6", Private6),
	  AppearNoName("Private7", Private7),
	  AppearNoName("Private8", Private8),
	  AppearNoName("Private9", Private9),
	  AppearNoName("Private10", Private10),
	  AppearNoName("Private11", Private11),
	  AppearNoName("Private12", Private12),
	  AppearNoName("Private13", Private13),
	  AppearNoName("Private14", Private14),
	  AppearNoName("Private15", Private15),
	  AppearNoName("Private16", Private16),
	  AppearNoName("Private17", Private17),
	  AppearNoName("Private18", Private18),
	  AppearNoName("Private19", Private19),
	  AppearNoName("Private20", Private20),
	  AppearNoName("BossGeneral0", BossGeneral0)
	};

	static inline AppearNoName const* selectedAppear = &(appearNoEnumNames[0]);

	EnemyFixes() = default;

	static inline bool cheaton = true;

	static inline uintptr_t ret = 0;

	std::string_view get_name() const override
	{
		return "EnemyFixes";
	}
	std::string get_checkbox_name() override
	{
		return m_check_box_name;
	};
	std::string get_hotkey_name() override
	{
		return m_hot_key_name;
	};

    static void draw_em_fixes()
    {
        ImGui::TextWrapped("Use this option if you want to swap enemies with boss Dante. Doesn't work on BP stages greater than 19. Can't be changed during gameplay.");
        ImGui::Checkbox("Fix boss Dante AI/Enable boss Vergil mission AI", &isBossDanteAiEnabled);
        if (isBossDanteAiEnabled) {
            ImGui::TextWrapped("Select this to specify Dante/Vergil AI type.\n"
                "Killing boss Vergil with m19 AI on missions/BP will cause a soft lock.");
            //ImGui::Checkbox("Use boss Dante/Vergil M20 AI", &isDanteM20);
            if (ImGui::RadioButton("Use M19 AI", !isDanteM20))
                isDanteM20 = false;
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
            if (ImGui::RadioButton("Use M20 AI", isDanteM20))
                isDanteM20 = true;
        }
        ImGui::Checkbox("Boss Vergil credit AI", &isFriendlyVergilAI);
        ImGui::Separator();
        ImGui::TextWrapped("Fix enemy familiars. Fixes for able killing Shadow, Griphon, fix for Shadow teleport and fix for Nightmare healing teleport pos will be automatically enabled "
            "if Enemy Swapper or Wave Editor are enabled (player position will be using).");
        /*ImGui::Checkbox("Enable kill shadow", &canKillShadow);
        ImGui::Checkbox("Enable kill griffon", &canKillGriffon);*/
        ImGui::Checkbox("Fix Nightmare meteor position", &isNightmareFix);
        if (isNightmareFix)
        {
            ImGui::TextWrapped("Offset from player position to start the meteor at.");
            ImGui::InputFloat("X offset", &nightmareStartPosOffs.x, 1.0f, 2.0f, "%.2f");
            ImGui::InputFloat("Y offset", &nightmareStartPosOffs.y, 1.0f, 2.0f, "%.2f");
            ImGui::InputFloat("Z offset", &nightmareStartPosOffs.z, 1.0f, 2.0f, "%.2f");
        }
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Checkbox("Fix Cavaliere teleports", &isCavFixEnabled);
        ImGui::ShowHelpMarker("Replace pre-coded teleport positions with offset from player position. "
            "Can result in Cavaliere becoming stuck in a teleporting state.");
        if (isCavFixEnabled)
        {
            ImGui::TextWrapped("X offset");
            ImGui::InputFloat("##cavXOffst", &cavOffset.x, 1.0f, 2.0f, "%.2f");
            ImGui::TextWrapped("Y offset");
            ImGui::InputFloat("##cavYOffst", &cavOffset.y, 1.0f, 2.0f, "%.2f");
            ImGui::TextWrapped("Z offset");
            ImGui::InputFloat("##cavZOffst", &cavOffset.z, 1.0f, 2.0f, "%.2f");
        }
        if (isVergilFixEnabled)
            ImGui::Separator();
        ImGui::Checkbox("Fix camera on doppel disappear", &isDoppelCameraFix);
        ImGui::ShowHelpMarker("It's not perfect, camera still can do funny things, but it will end faster)");
        ImGui::Checkbox("Fix Vergil Dive Bomb", &isVergilFixEnabled);
        ImGui::ShowHelpMarker("Replace pre-coded divebomb position with boss's current coordinates.");
        if (isVergilFixEnabled)
        {
           /* ImGui::TextWrapped("Height of arena sides  (default = 1.5)");
            ImGui::InputFloat("##divebombHeightOfArenaSide", &divebombHeightOfArenaSide, 0, 0, "%.1f");
            ImGui::TextWrapped("Height outside arena (default = 8)");
            ImGui::InputFloat("##divebombHeightOfOutside", &divebombHeightOfOutside, 0, 0, "%.1f");
            ImGui::TextWrapped("Distance from ground (default = 5)");
            ImGui::InputFloat("##divebombDistanceGroundCheck", &divebombDistanceGroundCheck, 0, 0, "%.1f");*/
            ImGui::Checkbox("Fast divebomb attack", &isFastDiveBombAttack);
            ImGui::ShowHelpMarker("Force Vergil to attack you earlier in the dive bomb. Vergil will also lock-on to you while flying making him harder to dodge. This can help if he starts infinitely flying in a circle.");
            ImGui::Separator();
            ImGui::Spacing();
        }
        ImGui::Checkbox("Fix Goliath jump", &isGoliathFixEnabled);
        ImGui::ShowHelpMarker("Replace pre-coded leap position with player coordinates.");

        ImGui::Checkbox("Fix Artemis flying position", &isArtemisFixEnabled);
        if (isArtemisFixEnabled)
        {
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Checkbox("Use current player position as arena center ##artemis", &isArtemisPlayersXY);
            ImGui::ShowHelpMarker("Artemis will always use the current player coordinates as the center of the arena wich will force her to fly away from the player when they get too close.");
            if (!isArtemisPlayersXY && !_isArtemisInitPos)
            {
                ImGui::Spacing();
                btn_set_plpos_to(artemisCenterOfFloor, "Set arena center to current player position ##Art");
                ImGui::Spacing();
                ImGui::InputFloat("X coordinate ##Art", &artemisCenterOfFloor.x, 1.0f, 2.0f, "%.2f");
                ImGui::InputFloat("Y coordinate ##Art", &artemisCenterOfFloor.y, 1.0f, 2.0f, "%.2f");
            }
            ImGui::Checkbox("Use init position as center of arena with custom height offset##Art", &_isArtemisInitPos);
            if(_isArtemisInitPos)
                ImGui::InputFloat("Height offset of arena center##Art", &_artemisZInitOffs, 1.0f, 2.0f, "%.1f");
            else
                ImGui::InputFloat("Height of arena center ##Art", &artemisCenterOfFloor.z, 1.0f, 2.0f, "%.2f");
            ImGui::Separator();
            ImGui::Spacing();
        }
        ImGui::Checkbox("Fix Urizen 3 teleports", &isUrizen3FixEnabled);
        ImGui::ShowHelpMarker("Prevent Urizen 3 becoming stuck in his teleport phase when changing battle state.");
        ImGui::Checkbox("Fix Malphas portals", &isMalphasFixEnabled);
        ImGui::ShowHelpMarker("Prevent Malphas from going out of bounds and dying.");
        if (isMalphasFixEnabled)
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Checkbox("Use current player position as arena center##malphas", &malphasFixPlPos);
            ImGui::ShowHelpMarker("Teleports are offset from the arena center, so if this is enabled and the player stands on the edge of the arena, Malphas can still teleport out of bounds. It's recommended to set custom coordinates for different areas manually.");
            if (!malphasFixPlPos && !_isMalphasFixInitPos)
            {
                btn_set_plpos_to(malphasCenterOfFloor, "Set arena center to current player position ##Malph");
                ImGui::InputFloat("X coord of arena center ##Malph", &malphasCenterOfFloor.x, 1.0f, 2.0f, "%.2f");
                ImGui::InputFloat("Y coord of arena center ##Malph", &malphasCenterOfFloor.y, 1.0f, 2.0f, "%.2f");
                ImGui::InputFloat("Z coord of arena center ##Malph", &malphasCenterOfFloor.z, 1.0f, 2.0f, "%.2f");
            }
            ImGui::Checkbox("Use spawn position as center of arena##Malph", &_isMalphasFixInitPos);
            ImGui::Separator();
            ImGui::Spacing();
        }
        ImGui::Checkbox("Fix Cerberus Moves", &isCerberusFixEnabled);
        ImGui::ShowHelpMarker("Fixes jumping in Cerberus's fire state and projectiles in his thunder state. There is currently no fix for the ice wall.");
        if (isCerberusFixEnabled)
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Checkbox("Use current player position as arena center##cerberus", &cerberusFixPlPos);
            if (!cerberusFixPlPos && !_isCerberusFixInitPos)
            {
                btn_set_plpos_to(cerberusCenterOfFloor, "Set arena center to current player position##Cerberus");
                ImGui::InputFloat("X coord of center of arena ##Cerberus", &cerberusCenterOfFloor.x, 0.1f, 0, "%.2f");
                ImGui::InputFloat("Y coord of center of arena ##Cerberus", &cerberusCenterOfFloor.y, 0.1f, 0, "%.2f");
                ImGui::InputFloat("Z coord of center of arena ##Cerberus", &cerberusCenterOfFloor.z, 0.1f, 0, "%.2f");
               /* cerberusThunderWaveZ = cerberusCenterOfFloor.z;
                cerberusThunderWavePlPos = false;*/
            }
            ImGui::Checkbox("Use spawn position as center of arena##Ceberus", &_isCerberusFixInitPos);
            //else cerberusThunderWavePlPos = true;
            ImGui::Separator();
            ImGui::Spacing();
            /*ImGui::Checkbox("Use current player z coord to spawn thunder wave attack", &cerberusThunderWavePlPos);
            if (!cerberusThunderWavePlPos)
                ImGui::InputFloat("Z for thunder wave", &cerberusThunderWaveZ, 0.1f, 0.0f, "%.2f");*/
        }
        ImGui::Checkbox("Fix some Geryon teleportation moves", &isTimeHorseFixEnabled);
        if (isTimeHorseFixEnabled && !_isTimeHorseFixInitPos)
        {
            ImGui::Spacing();
            ImGui::Separator();
            btn_set_plpos_to(geryonCenter, "Set arena center to current player position##horse");
            ImGui::InputFloat("X coord of center of arena ##horse", &geryonCenter.x, 0.1f, 0, "%.1f");
            ImGui::InputFloat("Y coord of center of arena ##horse", &geryonCenter.y, 0.1f, 0, "%.1f");
            ImGui::InputFloat("Z coord of center of arena ##horse", &geryonCenter.z, 0.1f, 0, "%.1f");
            ImGui::Separator();
        }
        ImGui::Checkbox("Use spawn position as center of arena##horse", &_isTimeHorseFixInitPos);
        ImGui::Spacing();
        ImGui::Checkbox("Change XYZ spawn coordinates", &isCustomSpawnPos);
        ImGui::ShowHelpMarker("Fixes some enemies spawning under the floor. Note that this will affect all spawns and can change spawn animations.");
        if (isCustomSpawnPos)
        {
            ImGui::Checkbox("Spawn on player's posiiton", &isAlwaysSpawnOnPlPos);
            ImGui::TextWrapped("Height offset");
            UI::SliderFloat("##spawnPosZOffsetSlider", &spawnPosZOffset, 0.0f, 12.0f, "%.1f");
            ImGui::TextWrapped("Changing XY coordinates can fix enemies spawning behind invisible walls (i.e. swapping Urizen 1, Nidhogg or Qliphot roots with other enemies).");
            ImGui::TextWrapped("X offset");
            UI::SliderFloat("##spawnPosXOffsetSlider", &spawnPosXOffset, -12.0f, 12.0f, "%.1f");
            ImGui::TextWrapped("Y offset");
            UI::SliderFloat("##spawnPosYOffsetSlider", &spawnPosYOffset, -12.0f, 12.0f, "%.1f");
            ImGui::Spacing();
            ImGui::Checkbox("Apply only for flying enemies", &isSpawnOffsForFlyingEnemiesOnly);
        }
        ImGui::Separator();
        ImGui::Checkbox("Force vertical spawn rotation & change appears animation", &isForceVerticalSpawnRot);
        ImGui::ShowHelpMarker("Changing spawn offsets can break some waves and make enemies \"swim in the air\". This should fix it.");
        if (isForceVerticalSpawnRot)
        {
            bool isSelected = false;
            if (ImGui::BeginCombo("##appearSelectionCombo", selectedAppear->name))
            {
                for (const auto& i : appearNoEnumNames)//(int i = 0; i < appearNoEnumNames.size(); i++)
                {
                    isSelected = selectedAppear->n == i.n;
                    if (ImGui::Selectable(i.name, isSelected))
                        selectedAppear = &i;
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::ShowHelpMarker("Change appear animation. \"None\" definitely will save you from wrong enemy rotation, but will remove spawn animation obviosly. You can experiment with other options, idk mb some of them will "
                "also work correct. Bosses will always use \"BossGeneral0\", otherwise softlock can be happened.");
        }
    }

	std::optional<std::string> on_initialize() override
	{
		init_check_box_info();
		auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
		m_is_enabled = &cheaton;
		m_on_page = Page_Enemies;
		m_full_name_string = "Enemy Fixes (+)";
		m_author_string = "V.P. Zadov";
		m_description_string = "Fixes for enemies spawn positions and bosses behaviors. Use it with \"Enemy Swapper\", \"Enemy Wave Editor\", \"Enemy Spawner\" or maybe some file mods that "
            "changes default enemy waves to fix out of bounds spawn or junk bosses behavior.";

        auto customSpawnAddr = m_patterns_cache->find_addr(base, "F3 41 0F11 4D 34 48 8B 4B 18 48 85 C9 75 10"); // DevilMayCry5.exe+11C58BC
        if (!customSpawnAddr) {
            return "Unanable to find customSpawnAddr pattern.";
        }

        auto m19CheckAddr = m_patterns_cache->find_addr(base, "83 78 7C 13 75 11"); //DevilMayCry5.exe+1D47B50
        if (!m19CheckAddr) {
            return "Unanable to find EnemyFixes.m19CheckAddr pattern.";
        }

        auto griffonKillAddr = m_patterns_cache->find_addr(base, "80 BF 48 0F 00 00 00"); //DevilMayCry5.exe+F9E17B
        if (!griffonKillAddr) {
            return "Unanable to find EnemyFixes.griffonKillAddr pattern.";
        }

        auto shadowKillAddr = m_patterns_cache->find_addr(base, "80 BF 60 0F 00 00 00"); //DevilMayCry5.exe+F5B07B
        if (!shadowKillAddr) {
            return "Unanable to find EnemyFixes.shadowKillAddr pattern.";
        }

        auto nightmireStartingAddr = m_patterns_cache->find_addr(base, "48 8B D7 F3 0F 11 45 97 F3 0F 11"); //DevilMayCry5.exe+20DEFFD
        if (!nightmireStartingAddr) {
            return "Unanable to find EnemyFixes.nightmareStartingMeteorPosAddr pattern.";
        }

        auto nightmireArrivalAddr = m_patterns_cache->find_addr(base, "12 F3 44 0F 11 50 30"); //DevilMayCry5.exe+16A13EB
        if (!nightmireArrivalAddr) {
            return "Unanable to find EnemyFixes.nightmareArrivalAddr pattern.";
        }

        //auto cavTeleportPosAddr = patterns->find_addr(base, "F3 41 0F 11 85 20 0E 00 00 F3 41"); //DevilMayCry5.exe+1290351
        //if (!cavTeleportPosAddr)
        //{
        //    return "Unanable to find EnemyFixes.cavTeleportPosAddr pattern.";
        //}

        auto cavTeleportPosAddr = m_patterns_cache->find_addr(base, "73 FD FF FF F3 0F 10 47 10"); //DevilMayCry5.exe+1290342 (-0x4)
        if (!cavTeleportPosAddr)
        {
            return "Unanable to find EnemyFixes.cavTeleportPosAddr pattern.";
        }

        auto vergilCenterFloorAddr = m_patterns_cache->find_addr(base, "04 00 00 F3 0F 10 80 A0 00 00 00 F3"); //DevilMayCry5.exe+9AE0B7 (-0x3);
        if (!vergilCenterFloorAddr)
        {
            return "Unanable to find EnemyFixes.vergilCenterFloorAddr pattern.";
        }

        auto vergilAirRaidControllerAddr = m_patterns_cache->find_addr(base, "F3 0F 10 B8 A0 00 00 00 F3"); //DevilMayCry5.exe+1CBD779
        if (!vergilAirRaidControllerAddr)
        {
            return "Unanable to find EnemyFixes.vergilAirRaidControllerAddr pattern.";
        }

        auto goliathSuckJmpAddr = m_patterns_cache->find_addr(base, "00 F3 0F 11 82 40 0D 00 00"); //DevilMayCry5.exe+12F2545 (-0x1)
        if (!goliathSuckJmpAddr)
        {
            return "Unanable to find EnemyFixes.goliathSuckJmpAddr pattern.";
        }

        auto goliathLeaveJmpAddr = m_patterns_cache->find_addr(base, "F3 41 0F 11 86 30 0D 00 00"); //DevilMayCry5.exe+12F2455
        if (!goliathLeaveJmpAddr)
        {
            return "Unanable to find EnemyFixes.goliathLeaveJmpAddr pattern.";
        }

        auto artemisCenterOfFloorAddr = m_patterns_cache->find_addr(base, "B7 F3 0F 10 59 10"); //DevilMayCry5.exe+1E9CC1D (-0x1)
        if (!artemisCenterOfFloorAddr)
        {
            return "Unanable to find EnemyFixes.artemisCenterOfFloorAddr pattern.";
        }

        auto urizen3TpAddr = m_patterns_cache->find_addr(base, "48 83 78 18 00 75 93 F3 0F 10 87 E0"); //DevilMayCry5.exe+1189760
        if (!urizen3TpAddr)
        {
            return "Unanable to find EnemyFixes.urizen3TpAddr pattern.";
        }

        auto malphasAddr = m_patterns_cache->find_addr(base, "F3 0F 10 87 90 00 00 00 F3 0F 11 4D B4"); //DevilMayCry5.exe+105B4C3
        if (!urizen3TpAddr)
        {
            return "Unanable to find EnemyFixes.malphasAddr pattern.";
        }

        auto cerberusAddr = m_patterns_cache->find_addr(base, "F3 44 0F 10 52 18 48 8B D3"); //DevilMayCry5.exe+111D60E
        if (!cerberusAddr)
        {
            return "Unanable to find EnemyFixes.cerberusAddr pattern.";
        }

        auto cerberusThunderWaveAddr = m_patterns_cache->find_addr(base, "F3 0F 5F 47 24"); //DevilMayCry5.exe+1121E1B
        if (!cerberusThunderWaveAddr)
        {
            return "Unanable to find EnemyFixes.cerberusThunderWaveAddr pattern.";
        }

        auto cerberusThunderBallAddr = m_patterns_cache->find_addr(base, "44 38 B0 A9 0F 00 00"); //DevilMayCry5.exe+1F39983
        if (!cerberusThunderBallAddr)
        {
            return "Unanable to find EnemyFixes.cerberusThunderBallAddr pattern.";
        }

        //auto shadowOnlyWarpAddr = patterns->find_addr(base, "48 8B 43 50 49 8B 4C 24 60 F3 0F 11 45 B0"); // DevilMayCry5.exe+21B0233
        //if (!nowFlowAddr)
        //{
        //    return "Unanable to find EnemyFixes.shadowWarpAddr pattern.";
        //}

        auto shadowFsmToCenterAddr = m_patterns_cache->find_addr(base, "4A F1 FE 48 8B 43 50 48 83 78 18 00"); // DevilMayCry5.exe+21B0114 (-0x3)
        if (!shadowFsmToCenterAddr)
        {
            return "Unanable to find EnemyFixes.shadowFsmToCenterAddr pattern.";
        }

        auto nightmireTeleExtFarAddr = m_patterns_cache->find_addr(base, "F3 0F 11 81 F0 0E 00 00"); // DevilMayCry5.exe+2068EBB
        if (!nightmireTeleExtFarAddr)
        {
            return "Unanable to find EnemyFixes.nightmireTeleExtFarAddr pattern.";
        }

        auto timeHorseCenterAddr = m_patterns_cache->find_addr(base, "F3 44 0F 10 87 C0 0D 00 00"); // DevilMayCry5.exe+10CC7E5
        if (!nightmireTeleExtFarAddr)
        {
            return "Unanable to find EnemyFixes.timeHorseCenterAddr pattern.";
        }

        const ptrdiff_t customSpawnOffs = 0xA;

        posSpawnTestJne = customSpawnAddr.value() + customSpawnOffs + 0x15;
        bossDanteAiJne = m19CheckAddr.value() + 0x17;
        vergilFixJs = vergilCenterFloorAddr.value() + 0x3 + 0x18;
        urizen3TpJne = urizen3TpAddr.value() - 0x66;
        cerberusThunderBallJmp = cerberusThunderBallAddr.value() + 0x1C6;

        //--------------------------------------------------------------------//

        if (!install_hook_absolute(customSpawnAddr.value() + customSpawnOffs, m_spawn_pos_hook, &spawn_pos_detour, &posSpawnRet, 0x5))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.customSpawnAddr";
        }

        if (!install_hook_absolute(m19CheckAddr.value(), m_m19check_hook, &load_Dante_ai_detour, &bossDanteAiRet, 0x6)) {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.m19CheckAddr";
        }

        if (!install_hook_absolute(griffonKillAddr.value(), m_griffon_hook, &kill_griffon_detour, &killGriffonRet, 0x7)) {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.griffonKill";
        }

        if (!install_hook_absolute(shadowKillAddr.value(), m_shadow_hook, &kill_shadow_detour, &killShadowRet, 0x7)) {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.griffonKill";
        }

        if (!install_hook_absolute(nightmireStartingAddr.value(), m_nightmire_starting_hook, &nightmire_starting_detour, &nightmareStartingPosRet, 0x8)) {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.nightmireStartingMeteorPos";
        }

        if (!install_hook_absolute(nightmireArrivalAddr.value() + 0x1, m_nightmire_arrival_hook, &nightmire_arrival_detour, &nightmareArrivalPosRet, 0x6)) {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.nightmireArrivalMeteorPos";
        }

        /*if (!install_hook_absolute(cavTeleportPosAddr.value(), m_cavfix_hook, &cavtele_detour, &cavFixRet, 0x9))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.cavTeleportPos";
        }*/

        if (!install_hook_absolute(cavTeleportPosAddr.value() + 0x4, m_cavfix_hook, &cavtele_detour, &cavFixRet, 0x5))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.cavTeleportPos";
        }

        if (!install_hook_absolute(vergilCenterFloorAddr.value() + 0x3, m_vergilcenterfloor_hook, &vergil_centerfloor_detour, &vergilFixRet, 0x8))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.vergilCenterFloor";
        }

        if (!install_hook_absolute(vergilAirRaidControllerAddr.value(), m_airraid_controller_hook, &airraid_detour, &airRaidControllerRet, 0x8))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.vergilAirRaidController";
        }

        if (!install_hook_absolute(goliathSuckJmpAddr.value() + 0x1, m_goliath_suctionjmp_hook, &goliath_jmpsuck_detour, &goliathSuckJmpRet, 0x8))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.goliathSuckJmp";
        }

        if (!install_hook_absolute(goliathLeaveJmpAddr.value(), m_goliath_leavejmp_hook, &goliath_jmpleave_detour, &goliathLeaveJmpRet, 0x9))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.goliathLeaveJmp";
        }

        if (!install_hook_absolute(artemisCenterOfFloorAddr.value() + 0x1, m_artemis_centerfloor_hook, &artemis_centerfloor_fix, &artemisFixRet, 0x5))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.artemisCenterOfFloor";
        }

        if (!install_hook_absolute(urizen3TpAddr.value(), m_urizen3_tp_hook, &urizen3tp_detour, &urizen3TpRet, 0x7))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.urizen3Tp";
        }

        if (!install_hook_absolute(malphasAddr.value(), m_malphas_tp_hook, &malphas_tp_detour, &malphasRet, 0x8))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.malphas";
        }

        if (!install_hook_absolute(cerberusAddr.value(), m_cerberus_pos_hook, &cerberus_pos_detour, &cerberusFixRet, 0x6))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.cerberus";
        }

        if (!install_hook_absolute(cerberusThunderWaveAddr.value(), m_cerberus_thunderwave_hook, &cerberus_thunderwave_detour, &cerberusThunderWaveRet, 0x5))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.cerberusThunderWave";
        }

        if (!install_hook_absolute(cerberusThunderBallAddr.value(), m_cerberus_thunderball_hook, &cerberus_thunderball_detour, &cerberusThunderBallRet, 0x7))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.cerberusThunderWave";
        }

        if (!install_hook_absolute(shadowFsmToCenterAddr.value() + 0x3, m_shadow_warp_func_hook, &fsm_shadow_warp_to_center_detour, &shadowWarpFuncRet, 0x9))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.shadowFsmToCenter";
        }

        if (!install_hook_absolute(nightmireTeleExtFarAddr.value(), m_nightmirepos_ext_far_hook, &nightmire_teleport_ext_far_detour, &nightmireExtFarPosRet, 0x8))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.shadowFsmToCenter";
        }

        if (!install_hook_absolute(timeHorseCenterAddr.value(), m_timehorse_center_hook, &time_horse_detour, &timeHorseRet, 0x9))
        {
            spdlog::error("[{}] failed to initialize", get_name());
            return "Failed to initialize EnemyFixes.timeHorseCenter";
        }

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
    void on_config_load(const utility::Config& cfg) override
    {
        isCustomSpawnPos = cfg.get<bool>("EnemyFixes.isCustomSpawnPos").value_or(false);
        spawnPosZOffset = cfg.get<float>("EnemyFixes.spawnPosZOffset").value_or(0.6f);
        spawnPosXOffset = cfg.get<float>("EnemyFixes.spawnPosXOffset").value_or(0.0f);
        spawnPosYOffset = cfg.get<float>("EnemyFixes.spawnPosYOffset").value_or(0.0f);
        isBossDanteAiEnabled = cfg.get<bool>("EnemyFixes.isBossDanteAiEnabled").value_or(false);
        isDanteM20 = cfg.get<bool>("EnemyFixes.isDanteM20").value_or(true);
        /*canKillGriffon = cfg.get<bool>("EnemyFixes.canKillGriffon").value_or(false);
        canKillShadow = cfg.get<bool>("EnemyFixes.canKillShadow").value_or(false);*/
        isNightmareFix = cfg.get<bool>("EnemyFixes.isNightmareFix").value_or(true);
        isCavFixEnabled = cfg.get<bool>("EnemyFixes.isCavFixEnabled").value_or(false);
        isVergilFixEnabled = cfg.get<bool>("EnemyFixes.isVergilFixEnabled").value_or(false);
        isFastDiveBombAttack = cfg.get<bool>("EnemyFixes.isFastDiveBombAttack").value_or(false);
        isGoliathFixEnabled = cfg.get<bool>("EnemyFixes.isGoliathFixEnabled").value_or(false);
        isArtemisFixEnabled = cfg.get<bool>("EnemyFixes.isArtemisFixEnabled").value_or(false);
        isArtemisPlayersXY = cfg.get<bool>("EnemyFixes.isArtemisPlayersXY").value_or(false);
        isUrizen3FixEnabled = cfg.get<bool>("EnemyFixes.isUrizen3FixEnabled").value_or(false);
        isMalphasFixEnabled = cfg.get<bool>("EnemyFixes.isMalphasFixEnabled").value_or(false);
        isCerberusFixEnabled = cfg.get<bool>("EnemyFixes.isCerberusFixEnabled").value_or(false);
        cerberusFixPlPos = cfg.get<bool>("EnemyFixes.cerberusFixPlPos").value_or(false);
        //cerberusThunderWavePlPos = cfg.get<bool>("EnemyFixes.cerberusThunderWavePlPos").value_or(true);
        isSpawnOffsForFlyingEnemiesOnly = cfg.get<bool>("EnemyFixes.isSpawnOffsForFlyingEnemiesOnly").value_or(false);
        isForceVerticalSpawnRot = cfg.get<bool>("EnemyFixes.isForceVerticalSpawnRot").value_or(false);
        isAlwaysSpawnOnPlPos = cfg.get<bool>("EnemyFixes.isAlwaysSpawnOnPlPos").value_or(false);
        isDoppelCameraFix = cfg.get<bool>("EnemyFixes.isDoppelCameraFix").value_or(true);
        _isArtemisInitPos = cfg.get<bool>("EnemyFixes._isArtemisInitPos").value_or(true);
        _isMalphasFixInitPos = cfg.get<bool>("EnemyFixes._isMalphasFixInitPos").value_or(true);
        _isCerberusFixInitPos = cfg.get<bool>("EnemyFixes._isCerberusFixInitPos").value_or(true);
        _isTimeHorseFixInitPos = cfg.get<bool>("EnemyFixes._isTimeHorseFixInitPos").value_or(true);
        nightmareStartPosOffs.x = cfg.get<float>("EnemyFixes.nightmareStartPosOffsX").value_or(135.83f);
        nightmareStartPosOffs.y = cfg.get<float>("EnemyFixes.nightmareStartPosOffsY").value_or(-112.45f);
        nightmareStartPosOffs.z = cfg.get<float>("EnemyFixes.nightmareStartPosOffsZ").value_or(82.784f);
       /* divebombHeightOfArenaSide = cfg.get<float>("EnemyFixes.divebombHeightOfArenaSide").value_or(-95.5f);
        divebombHeightOfOutside = cfg.get<float>("EnemyFixes.divebombHeightOfOutside").value_or(-95.0f);
        divebombDistanceGroundCheck = cfg.get<float>("EnemyFixes.divebombDistanceGroundCheck").value_or(100.0f);*/
        artemisCenterOfFloor.x = cfg.get<float>("EnemyFixes.artemisCenterOfFloorX").value_or(-368.0f);
        artemisCenterOfFloor.y = cfg.get<float>("EnemyFixes.artemisCenterOfFloorY").value_or(-308.5f);
        artemisCenterOfFloor.z = cfg.get<float>("EnemyFixes.artemisCenterOfFloorZ").value_or(1.15f);
        _artemisZInitOffs = cfg.get<float>("EnemyFixes._artemisZInitOffs").value_or(5.0f);
        malphasCenterOfFloor.x = cfg.get<float>("EnemyFixes.malphasCenterOfFloorX").value_or(0.0f);
        malphasCenterOfFloor.y = cfg.get<float>("EnemyFixes.malphasCenterOfFloorY").value_or(0.0f);
        malphasCenterOfFloor.z = cfg.get<float>("EnemyFixes.malphasCenterOfFloorZ").value_or(0.0f);
        cerberusCenterOfFloor.x = cfg.get<float>("EnemyFixes.cerberusCenterOfFloorX").value_or(0.0f);
        cerberusCenterOfFloor.y = cfg.get<float>("EnemyFixes.cerberusCenterOfFloorY").value_or(0.0f);
        cerberusCenterOfFloor.z = cfg.get<float>("EnemyFixes.cerberusCenterOfFloorZ").value_or(-0.1f);
        cerberusThunderWaveZ = cfg.get<float>("EnemyFixes.cerberusThunderWaveZ").value_or(-0.1f);
        geryonCenter.x = cfg.get<float>("EnemyFixes.geryonCenterX").value_or(-0.0f);
        geryonCenter.y = cfg.get<float>("EnemyFixes.geryonCenterY").value_or(-0.0f);
        geryonCenter.z = cfg.get<float>("EnemyFixes.geryonCenterZ").value_or(-0.0f);
        selectedAppear = AppearNoName::find_by_enum((AppearNoEnum)cfg.get<int>("EnemyFixes.selectedAppear").value_or(None));
        isFriendlyVergilAI = cfg.get<bool>("EnemyFixes.isFriendlyVergilAI").value_or(false);
    }

    void on_config_save(utility::Config& cfg) override
    {
        cfg.set<bool>("EnemyFixes.isCustomSpawnPos", isCustomSpawnPos);
        cfg.set<float>("EnemyFixes.spawnPosZOffset", spawnPosZOffset);
        cfg.set<float>("EnemyFixes.spawnPosXOffset", spawnPosXOffset);
        cfg.set<float>("EnemyFixes.spawnPosYOffset", spawnPosYOffset);
        cfg.set<bool>("EnemyFixes.isBossDanteAiEnabled", isBossDanteAiEnabled);
        cfg.set<bool>("EnemyFixes.canKillGriffon", canKillGriffon);
        cfg.set<bool>("EnemyFixes.canKillShadow", canKillShadow);
        cfg.set<bool>("EnemyFixes.isNightmareFix", isNightmareFix);
        cfg.set<bool>("EnemyFixes.isCavFixEnabled", isCavFixEnabled);
        cfg.set<bool>("EnemyFixes.isVergilFixEnabled", isVergilFixEnabled);
        cfg.set<bool>("EnemyFixes.isFastDiveBombAttack", isFastDiveBombAttack);
        cfg.set<bool>("EnemyFixes.isGoliathFixEnabled", isGoliathFixEnabled);
        cfg.set<bool>("EnemyFixes.isArtemisFixEnabled", isArtemisFixEnabled);
        cfg.set<bool>("EnemyFixes.isArtemisPlayersXY", isArtemisPlayersXY);
        cfg.set<bool>("EnemyFixes.isUrizen3FixEnabled", isUrizen3FixEnabled);
        cfg.set<bool>("EnemyFixes.isMalphasFixEnabled", isMalphasFixEnabled);
        cfg.set<bool>("EnemyFixes.isCerberusFixEnabled", isCerberusFixEnabled);
        cfg.set<bool>("EnemyFixes.cerberusFixPlPos", cerberusFixPlPos);
        cfg.set<bool>("EnemyFixes.cerberusThunderWavePlPos", cerberusThunderWavePlPos);
        cfg.set<bool>("EnemyFixes.isSpawnOffsForFlyingEnemiesOnly", isSpawnOffsForFlyingEnemiesOnly);
        cfg.set<bool>("EnemyFixes.isForceVerticalSpawnRot", isForceVerticalSpawnRot);
        cfg.set<bool>("EnemyFixes.isAlwaysSpawnOnPlPos", isAlwaysSpawnOnPlPos);
        cfg.set<bool>("EnemyFixes.isFriendlyVergilAI", isFriendlyVergilAI);
        cfg.set<bool>("EnemyFixes.isDoppelCameraFix", isDoppelCameraFix);
        cfg.set<bool>("EnemyFixes._isArtemisInitPos", _isArtemisInitPos);
        cfg.set<bool>("EnemyFixes._isMalphasFixInitPos", _isMalphasFixInitPos);
        cfg.set<bool>("EnemyFixes._isCerberusFixInitPos", _isCerberusFixInitPos);
        cfg.set<bool>("EnemyFixes._isTimeHorseFixInitPos", _isTimeHorseFixInitPos);
        cfg.set<float>("EnemyFixes.nightmareStartPosOffsX", nightmareStartPosOffs.x);
        cfg.set<float>("EnemyFixes.nightmareStartPosOffsY", nightmareStartPosOffs.y);
        cfg.set<float>("EnemyFixes.nightmareStartPosOffsZ", nightmareStartPosOffs.z);
        /*cfg.set<float>("EnemyFixes.divebombHeightOfArenaSide", divebombHeightOfArenaSide);
        cfg.set<float>("EnemyFixes.divebombHeightOfOutside", divebombHeightOfOutside);
        cfg.set<float>("EnemyFixes.divebombDistanceGroundCheck", divebombDistanceGroundCheck);*/
        cfg.set<float>("EnemyFixes.artemisCenterOfFloorX", artemisCenterOfFloor.x);
        cfg.set<float>("EnemyFixes.artemisCenterOfFloorY", artemisCenterOfFloor.y);
        cfg.set<float>("EnemyFixes.artemisCenterOfFloorZ", artemisCenterOfFloor.z);
        cfg.set<float>("EnemyFixes.malphasCenterOfFloorX", malphasCenterOfFloor.x);
        cfg.set<float>("EnemyFixes.malphasCenterOfFloorY", malphasCenterOfFloor.y);
        cfg.set<float>("EnemyFixes.malphasCenterOfFloorZ", malphasCenterOfFloor.z);
        cfg.set<float>("EnemyFixes.cerberusCenterOfFloorX", cerberusCenterOfFloor.x);
        cfg.set<float>("EnemyFixes.cerberusCenterOfFloorY", cerberusCenterOfFloor.y);
        cfg.set<float>("EnemyFixes.cerberusCenterOfFloorZ", cerberusCenterOfFloor.z);
        cfg.set<float>("EnemyFixes.cerberusThunderWaveZ", cerberusThunderWaveZ);
        cfg.set<float>("EnemyFixes.geryonCenterX", geryonCenter.x);
        cfg.set<float>("EnemyFixes.geryonCenterY", geryonCenter.y);
        cfg.set<float>("EnemyFixes.geryonCenterZ", geryonCenter.z);
        cfg.set<float>("EnemyFixes._artemisZInitOffs", _artemisZInitOffs);
        cfg.set<int32_t>("EnemyFixes.selectedAppear", selectedAppear->n);
    }

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
    void on_draw_ui() override
    {
        draw_em_fixes();
    }
};
//clang-format on