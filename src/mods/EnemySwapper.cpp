#include "EnemySwapper.hpp"
#include <string>
#include "sdk/ReClass.hpp"
#include "EnemyDataSettings.hpp"

bool EnemySwapper::cheaton{false};
bool EnemySwapper::isSwapAll{false};
bool EnemySwapper::isCustomRandomSettings{false};
bool EnemySwapper::isCustomSeed{false};
bool EnemySwapper::isCustomSpawnPos{false};
bool EnemySwapper::isBossDanteAiEnabled{false};
bool EnemySwapper::isDanteM20{false};
bool EnemySwapper::canKillGriffon{false};
bool EnemySwapper::canKillShadow{false};
bool EnemySwapper::isNightmareFix{false};
bool EnemySwapper::isInMission{false};
bool EnemySwapper::isCavFixEnabled{false};
bool EnemySwapper::isVergilFixEnabled{false};
bool EnemySwapper::isFastDiveBombAttack{false};
bool EnemySwapper::isGoliathFixEnabled{false};
bool EnemySwapper::isArtemisFixEnabled{false};
bool EnemySwapper::isArtemisPlayersXY{false};
bool EnemySwapper::isUrizen3FixEnabled{false};
bool EnemySwapper::isMalphasFixEnabled{false};
bool EnemySwapper::malphasFixPlPos{false};
bool EnemySwapper::isCerberusFixEnabled{false};
bool EnemySwapper::cerberusFixPlPos{false};
bool EnemySwapper::cerberusThunderWavePlPos {false};

bool isReswap = false;
bool isSkipNum = false;
bool checkForReswap = false;
bool defaultEmSetting = true;
int index = 0;

uintptr_t EnemySwapper::setEnemyDataRet1{NULL};
uintptr_t EnemySwapper::setEnemyDataRet2{NULL};
uintptr_t EnemySwapper::posSpawnRet{NULL};
uintptr_t EnemySwapper::posSpawnTestJne{NULL};
uintptr_t EnemySwapper::setEnemyDataRet3{NULL};
//uintptr_t EnemySwapper::setEnemyDataRet4{NULL};
//uintptr_t EnemySwapper::setEnemyData4Jmp{NULL};
uintptr_t EnemySwapper::setEnemyDataRet5{NULL};
uintptr_t EnemySwapper::setEnemyDataRet6{NULL};
//uintptr_t EnemySwapper::setEnemyDataRet7{NULL};
//uintptr_t EnemySwapper::swapIdRet{NULL};
uintptr_t EnemySwapper::nowFlowRet{NULL};
uintptr_t EnemySwapper::gameModeRet{NULL};
uintptr_t EnemySwapper::bossDanteAiRet{NULL};
uintptr_t EnemySwapper::bossDanteAiJne{NULL};
uintptr_t EnemySwapper::killShadowRet{NULL};
uintptr_t EnemySwapper::killGriffonRet{NULL};
uintptr_t EnemySwapper::nightmareStartingPosRet{NULL};
uintptr_t EnemySwapper::nightmareArrivalPosRet{NULL};
uintptr_t EnemySwapper::plPosBase{NULL};
uintptr_t EnemySwapper::cavFixRet{NULL};
uintptr_t EnemySwapper::vergilFixRet{NULL};
uintptr_t EnemySwapper::vergilFixJs{NULL};
uintptr_t EnemySwapper::airRaidControllerRet{NULL};
uintptr_t EnemySwapper::goliathLeaveJmpRet{NULL};
uintptr_t EnemySwapper::goliathSuckJmpRet{NULL};
uintptr_t EnemySwapper::artemisFixRet{NULL};
uintptr_t EnemySwapper::urizen3TpRet{NULL};
uintptr_t EnemySwapper::urizen3TpJne{NULL};
uintptr_t EnemySwapper::malphasRet{NULL};
uintptr_t EnemySwapper::cerberusFixRet{NULL};
uintptr_t EnemySwapper::cerberusThunderWaveRet{NULL};
uintptr_t EnemySwapper::cerberusThunderBallJmp{NULL};
uintptr_t EnemySwapper::cerberusThunderBallRet{NULL};

uint32_t EnemySwapper::selectedToSwap[enemyListCount];
uint32_t EnemySwapper::selectedSwapAll{NULL};
uint32_t selectedForAllSwap = 0;//selected indx for all swap
uint32_t EnemySwapper::currentEnemyId = 0;//Enemy Id from asm
uint32_t EnemySwapper::newEnemyId     = 0;//Enemy Id to swap in current state

uint32_t EnemySwapper::newEnemyId1 = 0;
uint32_t EnemySwapper::newEnemyId2 = 0;
uint32_t EnemySwapper::newEnemyId3 = 0;
uint32_t EnemySwapper::newEnemyId5 = 0;
uint32_t EnemySwapper::newEnemyId6 = 0;

uint32_t EnemySwapper::nowFlow        = 0;
uint32_t EnemySwapper::prevFlow       = 0;
uint32_t flowTmp = 0;
uint32_t EnemySwapper::gameMode       = 0;

float EnemySwapper::spawnPosZOffset = 0.0f;
float EnemySwapper::spawnPosXOffset = 0.0f;
float EnemySwapper::spawnPosYOffset = 0.0f;
float EnemySwapper::curSpawnPosZ = 0.0f;
float EnemySwapper::curSpawnPosX      = 0.0f;
float EnemySwapper::curSpawnPosY      = 0.0f;
int reswapCount                     = 0;
float EnemySwapper::waitTimeMin       = 0.0f;
float EnemySwapper::waitTimeMax       = 0.0f;
float EnemySwapper::odds              = 100.0f;
int EnemySwapper::enemyNum            = 1;
float EnemySwapper::divebombDistanceGroundCheck = 100.0f; // default = 5
float EnemySwapper::divebombHeightOfArenaSide = -100.5f; // default = 1.5
float EnemySwapper::divebombHeightOfOutside = -100.0f; // default = 8
float EnemySwapper::cerberusThunderWaveZ = 0.0f; 
static std::string uniqStr = "";
std::string uniqComboStr = "";//For comboboxes

std::array<EnemySwapper::EnemyId, EnemySwapper::enemyListCount> EnemySwapper::swapSettings;//cur id - id to swap

EnemySwapper::EnemyId EnemySwapper::swapForAll;

std::vector<uintptr_t> EnemySwapper::setDataAddrs;// = new std::vector<uintptr_t>();

Vector3f EnemySwapper::nightmareStartPosOffs;

uintptr_t curSetDataAddr = 0;

 bool skip_reswap() {
  if (EnemySwapper::setDataAddrs.size() != 0) {
     for (int i = 0; i < EnemySwapper::setDataAddrs.size(); i++) {
      if (curSetDataAddr == EnemySwapper::setDataAddrs[i]) {
        return true;
      }
    }
  }
  return false;
}
 
  static uint32_t custom_swap_setup() {
  //EnemySwapper::newEnemyId = EnemySwapper::currentEnemyId;
    uint32_t res = EnemySwapper::currentEnemyId;

  //if (checkForReswap) {
    if (skip_reswap()) {
      return res;
    }
  //}
    //mtx.lock();
 for (int i = 0; i < EnemySwapper::swapSettings.size(); i++) {
      if (EnemySwapper::currentEnemyId == EnemySwapper::swapSettings[i].get_current_id()) {
       if (EnemySwapper::currentEnemyId != EnemySwapper::swapSettings[i].get_swap_id()) {
         // EnemySwapper::newEnemyId = EnemySwapper::swapSettings[i].get_swap_id();
          //if (checkForReswap) {
         res = EnemySwapper::swapSettings[i].get_swap_id();
          EnemySwapper::setDataAddrs.push_back(curSetDataAddr);
          //}
       }
       //mtx.unlock();
       return res;
      }
  }
 //mtx.unlock();
 return res;
 //EnemySwapper::newEnemyId = 0;
 }

 static void enemydata_settings_setup() {
   defaultEmSetting = true;
   for (auto const &em : EnemySwapper::enemySettings) {
     if (em.emId.get_current_id() == EnemySwapper::currentEnemyId) {
       if (em.useDefault) {
         return;
       }
       defaultEmSetting = false;
       EnemySwapper::waitTimeMin = em.waitTimeMin;
       EnemySwapper::waitTimeMax = em.waitTimeMax;
       EnemySwapper::enemyNum = em.enemyNum;
       EnemySwapper::odds = em.odds;
       return;
     }
   }
 }

static naked void enemy_swap_detour1() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall

        /*checkdataoption:
        cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata
        jmp originalcode*/

        swapsettings:
        cmp dword ptr [EnemySwapper::gameMode], 3
        jne originalcode
        //mov byte ptr [r15+0x5C], 00  // IsEndSaveData
        mov [curSetDataAddr], rcx
        //mov byte ptr [checkForReswap], 1
        mov esi,[rcx+0x10]
        mov [EnemySwapper::currentEnemyId], esi

        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32
        call [custom_swap_setup]
        add rsp, 32
        //mov dword ptr [rcx+0x10], eax
        mov [EnemySwapper::newEnemyId1], eax
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax

        mov esi, dword ptr [EnemySwapper::newEnemyId1]
        mov [rcx+0x10], esi
        jmp originalcode//setup

        originalcode:
        mov esi,[rcx+0x10]
        test rax,rax
        jmp qword ptr [EnemySwapper::setEnemyDataRet1]

        /*setup:
        //mov byte ptr [r15+0x52], 01 // IsInit
        // mov byte ptr [r15+0xB2], 00 // isReadSealMode
       // mov byte ptr [r15+0x55], 01 // IsManualPlayerInTargetArea
       // mov byte ptr [r15+0x5E], 01 // IsCheckLoadRequest
        //mov byte ptr [r15+0xCC], 01  // IsPause
        // mov byte ptr [r15+0x2A], 00 // IsNearThePlayer
        //mov byte ptr [r15+0xCD], 01 // IsREquestEndGenerate
        movss xmm1, [EnemySwapper::odds]
        movss [rcx+0x24], xmm1
        movss xmm1, [EnemySwapper::waitTimeMin]
        movss [rcx+0x2C], xmm1
        movss xmm1, [EnemySwapper::waitTimeMax]
        movss [rcx+0x30], xmm1
        mov esi, [EnemySwapper::enemyNum]

        mov dword ptr [rcx+0x20], esi  // EnemyNum
        jmp originalcode  */      

        swapall:
        mov esi, [EnemySwapper::swapForAll.swapId]
        mov [rcx+0x10], esi
        //mov dword ptr [rcx+0x20], 4//EnemyNum
        jmp originalcode//setup

  }
}

static naked void enemy_swap_detour2() {
  __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode//checkdataoption
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall
        jmp swapsettings

        swapsettings:
        //cmp byte ptr [EnemySwapper::isBp], 1
        //je originalcode
       // mov byte ptr [checkForReswap], 1
        cmp dword ptr [EnemySwapper::gameMode], 3
        jne originalcode
        //mov byte ptr [r15+0x5C], 00 // IsEndSaveData

        mov r8d, [rcx+0x10]
        mov [EnemySwapper::currentEnemyId], r8d
        //mov [EnemySwapper::newEnemyId], r8d
        mov [curSetDataAddr], rcx

        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32

        call qword ptr [custom_swap_setup]

        add rsp, 32
        //mov dword ptr [rcx+0x10], eax
        mov [EnemySwapper::newEnemyId2], eax
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax

        mov r8d, dword ptr [EnemySwapper::newEnemyId2]
        mov [rcx+0x10], r8d
        //cmp byte ptr [EnemyDataSettings::cheaton], 1
        //je setupemdata
        jmp originalcode//setup
        //mov byte ptr [r15+0xF8], 1//isResourceLoad
        //mov dword ptr [rcx+0x54], 0x4

        originalcode:
        mov r8d,[rcx+0x10]
        mov rdx,rdi
        jmp qword ptr [EnemySwapper::setEnemyDataRet2]

        //setupemdata:
        //mov byte ptr [r15+0x52], 01//IsInit
        //mov byte ptr [r15+0xB2], 00 // isReadSealMode
        //mov byte ptr [r15+0x55], 01 // IsManualPlayerInTargetArea
        //mov byte ptr [r15+0x5E], 01 // IsCheckLoadRequest
        //mov byte ptr [r15+0xCC], 00 // IsPause
        //mov byte ptr [r15+0x2A], 00 // IsNearThePlayer
        //mov byte ptr [r15+0xCD], 01 // IsREquestEndGenerate
        

        swapall:
        mov r8d, [EnemySwapper::swapForAll.swapId]
        mov [rcx+0x10], r8d
        //cmp byte ptr [EnemyDataSettings::cheaton], 1
       // je setupemdata
        jmp originalcode//setup

  }
}

static naked void enemy_swap_detour3() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je checkdataoption
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall
        /*cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata*/
        jmp swapsettings
        //jmp setup

        checkdataoption:
        cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata
        jmp originalcode


        swapsettings:
        cmp dword ptr [EnemySwapper::gameMode], 3 //for bp skip
        je checkdataoption
        //mov byte ptr [checkForReswap], 1
        mov ecx, [r14+0x10]
        mov [EnemySwapper::currentEnemyId], ecx
        //mov [EnemySwapper::newEnemyId], ecx
        mov qword ptr [curSetDataAddr], r14

        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32

        call qword ptr [custom_swap_setup]

        add rsp, 32
        mov [EnemySwapper::newEnemyId3], eax
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax

        mov ecx, [EnemySwapper::newEnemyId3]
        mov [r14+0x10], ecx
        cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata
        //mov [r8+0x10], ecx//Can't load BP
        jmp originalcode // setup

        setupemdata:
        cmp byte ptr [EnemyDataSettings::shareSettings], 1
        je setdatavalues
        mov ecx, dword ptr [r14+0x10]
        mov  [EnemySwapper::currentEnemyId], ecx
        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32

        call [enemydata_settings_setup]

		add rsp, 32
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax
        cmp byte ptr [defaultEmSetting], 1
        je originalcode

        setdatavalues:
        mov rcx, 0
        mov ecx, [EnemySwapper::odds]
        mov [r14+0x24], ecx
        mov ecx, [EnemySwapper::waitTimeMin]
        mov [r14+0x2C], ecx
        mov ecx, [EnemySwapper::waitTimeMax]
        mov [r14+0x30], ecx
        mov ecx, [EnemySwapper::enemyNum]
        mov dword ptr [r14+0x20], ecx // EnemyNum
        jmp originalcode

        originalcode:
        mov ecx,[r14+0x10]
        test rax, rax
        jmp qword ptr [EnemySwapper::setEnemyDataRet3]

        swapall:
        mov ecx, [EnemySwapper::swapForAll.swapId]
        mov [r14+0x10], ecx
        cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata
        jmp originalcode // setup
  }
}

/*static naked void enemy_swap_detour4() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode

        checkaddr:
        test rbx, rbx
        jne originalcode
        test r8, r8
        je originalcode
        push rbx
        mov rbx, [r8]//crash, can be not-a-pointer actually
        cmp bx, 0x6CC0 //SetEnemyData or PlayerManager
        pop rbx
        je checkstate
        jmp originalcode

        checkstate:
        pop rbx
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall

        swapsetup:
        jmp originalcode

        swapall:
        mov eax, [EnemySwapper::swapForAll.swapId]

        originalcode:
        xor rbx, rbx //Non-original line
        mov [r14], eax
        jmp qword ptr [EnemySwapper::setEnemyData4Jmp]
        jmp qword ptr [EnemySwapper::setEnemyDataRet4]

  }
}*/

static naked void enemy_swap_detour5() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall

        cheat:
        mov eax, dword ptr [r15+0x10]
        mov [EnemySwapper::currentEnemyId], eax
        mov [curSetDataAddr], r15
        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32

        call qword ptr [custom_swap_setup]

        add rsp, 32
    // mov dword ptr [r14+0x10], eax
        mov [r15+0x10], eax
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax
        mov eax, [r15+0x10]

        originalcode:
        mov [rsi+0x10],eax
        mov rax,[rdi+0x50]
        jmp qword ptr [EnemySwapper::setEnemyDataRet5]

        /*setup:
        movss xmm1, [EnemySwapper::odds]
        movss [r14+0x24], xmm1
        movss xmm1, [EnemySwapper::waitTimeMin]
        movss [r14+0x2C], xmm1
        movss xmm1, [EnemySwapper::waitTimeMax]
        movss [r14+0x30], xmm1
        mov eax, [EnemySwapper::enemyNum]
        mov dword ptr [r14+0x20], eax // EnemyNum
        mov eax, [r15+0x10]
        jmp originalcode*/

        swapall:
        mov eax, [EnemySwapper::swapForAll.swapId]
        mov [r15+0x10], eax
        jmp originalcode
  }
}


static naked void enemy_swap_detour6() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode//checkdataoption
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall
        jmp swapsettings

        swapsettings:
        //cmp byte ptr [EnemySwapper::isBp], 1
        //je originalcode
        //mov byte ptr [checkForReswap], 1
        mov ebx, [r14+0x10]
        mov [EnemySwapper::currentEnemyId], ebx
        //mov [EnemySwapper::newEnemyId], ebx
        mov [curSetDataAddr], r14

        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32

        call qword ptr [custom_swap_setup]

        add rsp, 32
        //mov dword ptr [r14+0x10], eax
        mov [EnemySwapper::newEnemyId6], eax
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax
        mov ebx, [EnemySwapper::newEnemyId6]
        mov [r14+0x10], ebx
        mov rbx, 0
        /*cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata*/
        jmp originalcode // setup

        originalcode:
        mov r14d, [r14+0x10]
        mov [rsp+0x40], r15
        jmp [EnemySwapper::setEnemyDataRet6]

        swapall:
        mov ebx, dword ptr [EnemySwapper::swapForAll.swapId]
        mov [r14+0x10], ebx
        mov ebx, 0
        /*cmp byte ptr [EnemyDataSettings::cheaton], 1
        je setupemdata*/
        jmp originalcode//setup
  }
}

static void clear_setData_addrs() {
  if (EnemySwapper::setDataAddrs.size() != 0)
    EnemySwapper::setDataAddrs.clear();
  //reswapCount = 0;
}

static naked void spawn_pos_detour() {
    __asm {
        cmp byte ptr[WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je ewe_check
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        ewe_check:
        cmp byte ptr [EnemySwapper::isCustomSpawnPos], 1
        je cheat
        jmp originalcode

        cheat:
        movss [EnemySwapper::curSpawnPosZ], xmm1
        addss xmm1, [EnemySwapper::spawnPosZOffset]
        movss [r13+0x34], xmm1
        movss xmm1, [r13+0x30]//X
        movss [EnemySwapper::curSpawnPosX], xmm1
        addss xmm1, [EnemySwapper::spawnPosXOffset]
        movss [r13+0x30], xmm1
        movss xmm0, [r13+0x38] // Y
        movss [EnemySwapper::curSpawnPosY], xmm0
        addss xmm0, [EnemySwapper::spawnPosYOffset]
        movss [r13+0x38], xmm0

        originalcode:
        test rcx,rcx
        jne shortjne
        jmp qword ptr [EnemySwapper::posSpawnRet]

        shortjne:
        jmp qword ptr [EnemySwapper::posSpawnTestJne]

  }
}

void restore_em_list()
{
    //if (EnemySwapper::nowFlow == 23)//MissionRes
    //{
        WaveEditorMod::EnemyWaveEditor::mimObjManager.restore_all_data();
        WaveEditorMod::EnemyWaveEditor::mimObjManager.dealloc_all();
        WaveEditorMod::EnemyWaveEditor::emSetterCounter = 0;
        WaveEditorMod::EnemyWaveEditor::isPfbLoadRequested = false;
        //mimObjManager.dealloc_all();
    //}
}

static naked void now_flow_detour() {
    __asm {
        mov dword ptr [EnemySwapper::nowFlow], eax
        mov eax, [rdx+0x88]
        mov [EnemySwapper::isInMission], ah
        mov eax, dword ptr [rdx + 0x54]//nowFlow
        cmp [flowTmp], eax
        jne changeprev
        //cmp byte ptr [EnemySwapper::cheaton], 0
        //je em_list_check//originalcode
        jmp check

        changeprev:
        mov eax, [flowTmp]
        mov [EnemySwapper::prevFlow], eax
        mov eax, dword ptr [EnemySwapper::nowFlow]
        mov [flowTmp], eax
        //cmp byte ptr [EnemySwapper::cheaton], 0
        //je em_list_check//originalcode

        check:
        cmp eax, 0xE //14-MainMenu
        je clear
        /*cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 1
        je reset_em_list*/
        //jmp em_list_check//originalcode
        jmp originalcode


        clear:
        cmp byte ptr [EnemySwapper::prevFlow], 0x16 // 22-Game
        jne originalcode
        push rax
		push rbx
		push rcx
        push rsi
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32
        call [clear_setData_addrs]
        add rsp, 32
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rsi
		pop rcx
		pop rbx
		pop rax

        /*em_list_check:
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 1
        je reset_em_list*/

        originalcode:
        mov rbx,rdx
        mov rdi,rcx
        jmp qword ptr [EnemySwapper::nowFlowRet]

        /*reset_em_list:
        cmp dword ptr [EnemySwapper::nowFlow], 0x17
        jne originalcode
        push rax
        push rcx
        push rdx
		push r8
		push r9
		push r10
		push r11
        sub rsp, 32
        call [restore_em_list]
        add rsp, 32
        pop r11
		pop r10
	    pop r9
		pop r8
        pop rdx
		pop rcx
		pop rax
        jmp originalcode*/
  }
}

static naked void gamemode_detour() {
    __asm {
        push rcx
        mov ecx, dword ptr [rax+0x90]
        mov dword ptr [EnemySwapper::gameMode], ecx
        pop rcx
        cmp [rax+0x00000090],edi
        jmp qword ptr [EnemySwapper::gameModeRet]
  }
}

static naked void load_Dante_ai_detour() {
    __asm {
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isBossDanteAiEnabled], 00
        je originalcode

        cheat:
        cmp byte ptr [EnemySwapper::isDanteM20], 01
        je dante_m20
        mov dword ptr [rax+0x7C], 0x13

        originalcode:
        cmp dword ptr [rax+0x7C], 0x13
        je ret_je
        jmp qword ptr [EnemySwapper::bossDanteAiRet]

        dante_m20:
        mov dword ptr [rax+0x7C], 0x14
        jmp originalcode

        ret_je:
        jmp qword ptr [EnemySwapper::bossDanteAiJne]
  }
}

static naked void kill_griffon_detour() {
    __asm {
        cmp byte ptr[WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        /*cmp byte ptr [Mission::canKillGriffon], 00
        je originalcode*/
        cmp dword ptr [MissionManager::missionNumber], 0x12
        je originalcode
        jmp qword ptr [EnemySwapper::killGriffonRet]

        originalcode:
        cmp byte ptr [rdi+0x00000F48],00
        jmp qword ptr [EnemySwapper::killGriffonRet]
  }
}

static naked void kill_shadow_detour() {
  __asm {
        cmp byte ptr[WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        /*cmp byte ptr [EnemySwapper::canKillShadow], 00
        je originalcode*/
        cmp dword ptr [MissionManager::missionNumber], 0x12
        je originalcode
        jmp qword ptr [EnemySwapper::killShadowRet]

        originalcode:
        cmp byte ptr [rdi+0x00000F60], 00
        jmp qword ptr [EnemySwapper::killShadowRet]
  }
}

static naked void nightmire_starting_detour() {
    __asm {
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isNightmareFix], 00
        je originalcode

        cheat:
        push r10
        mov r10, [EnemySwapper::plPosBase]
        mov r10, [r10]
        mov r10, [r10+0x70]
        mov r10, [r10+0xC70]
        mov r10, [r10+0x40]
        mov r10, [r10+0xE40]
        mov r10, [r10+0x80]
        movss xmm0, [r10+0x30]//plCoords
        movss xmm1, [r10+0x34]
        movss xmm2, [r10+0x38]
        pop r10
        addss xmm0, [EnemySwapper::nightmareStartPosOffs.x]
        addss xmm1, [EnemySwapper::nightmareStartPosOffs.z]
        addss xmm2, [EnemySwapper::nightmareStartPosOffs.y]

        originalcode:
        mov rdx, rdi
        movss [rbp-0x69], xmm0
        jmp qword ptr [EnemySwapper::nightmareStartingPosRet]
  }
}

static naked void nightmire_arrival_detour() {
    __asm {
        cmp byte ptr[WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isNightmareFix], 00
        je originalcode

        cheat:
        push r10
        mov r10, [EnemySwapper::plPosBase]
        mov r10, [r10]
        mov r10, [r10+0x70]
        mov r10, [r10+0xC70]
        mov r10, [r10+0x40]
        mov r10, [r10+0xE40]
        mov r10, [r10+0x80]
        movss xmm10, [r10+0x30] // plCoords
        movss xmm11, [r10+0x34]
        movss xmm12, [r10+0x38]
        pop r10

        originalcode:
        movss [rax+0x30], xmm10
        jmp qword ptr [EnemySwapper::nightmareArrivalPosRet]
  }
}

static naked void cavtele_detour()//need to find where it calc distance
{
    __asm {
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isCavFixEnabled], 00
        je originalcode

        cheat:
        //--------PlayerPos--------//
        push r10
        mov r10, [EnemySwapper::plPosBase]
        mov r10, [r10]
        mov r10, [r10+0x70]
        mov r10, [r10+0xC70]
        mov r10, [r10+0x40]
        mov r10, [r10+0xE40]
        mov r10, [r10+0x80]
        //--------PlayerPos--------//
        movss xmm0, [r10+0x30]
        addss xmm0, [EnemySwapper::cavOffset.x]
        movss xmm1, [r10+0x34]
        addss xmm1, [EnemySwapper::cavOffset.z]
        movss xmm2, [r10+0x38]
        addss xmm2, [EnemySwapper::cavOffset.y]
        pop r10
        movss [rdi+0x10], xmm0
        movss [rdi+0x14], xmm1
        movss [rdi+0x18], xmm2

        originalcode:
        //movss [r13+0x00000E20], xmm0
        movss xmm0, [rdi+0x10]
        jmp qword ptr [EnemySwapper::cavFixRet]
    }
}

static naked void vergil_centerfloor_detour()//for dive bomb
{
    __asm {
        cmp byte ptr[WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isVergilFixEnabled], 00
        je originalcode

        cheat:
        //--------PlayerPos--------//
        push rbx
        mov rbx, [EnemySwapper::plPosBase]
        mov rbx, [rbx]
        mov rbx, [rbx+0x70]
        mov rbx, [rbx+0xC70]
        mov rbx, [rbx+0x40]
        mov rbx, [rbx+0xE40]
        mov rbx, [rbx+0x80]
        //--------PlayerPos--------//
        movss xmm0, [rbx+0x30]
        movss xmm1, [rbx+0x34]
        movss xmm2, [rbx+0x38]
        mov rbx, rax
        mov rbx, [rbx+0x108]//AirRaid
        push r8
        mov r8, [EnemySwapper::divebombHeightOfOutside]
        mov [rbx+0x7C], r8
        mov r8, [EnemySwapper::divebombHeightOfArenaSide]
        mov [rbx+0x78], r8
        mov r8, [EnemySwapper::divebombDistanceGroundCheck]
        mov [rbx+0x80], r8
        /*mov r8, [EnemySwapper::radiusOfArea]
        mov [rbx+0x70], r8
        mov r8, [EnemySwapper::radiusOfRevolution]
        mov [rbx+0x74], r8*/
        pop r8
        pop rbx
        movss [rax+0x000000A0], xmm0
        movss [rax+0x000000A4], xmm1
        movss [rax+0x000000A8], xmm2
        jmp qword ptr [EnemySwapper::vergilFixJs]

        originalcode:
        movss xmm0, [rax+0x000000A0]
        jmp qword ptr [EnemySwapper::vergilFixRet]
    }
}

static naked void airraid_detour()
{
    __asm {
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isVergilFixEnabled], 00
        je originalcode

        cheat:
        cmp byte ptr [EnemySwapper::isFastDiveBombAttack], 0
        je originalcode
        cmp byte ptr [rsi+0x32], 1
        je originalcode
        cmp byte ptr [rsi+0x31], 0
        je originalcode
        mov byte ptr [rsi+0x32], 1

        originalcode:
        movss xmm7, [rax+0x000000A0]
        jmp qword ptr [EnemySwapper::airRaidControllerRet]
    }
}

static naked void goliath_jmpsuck_detour()
{
    __asm {
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isGoliathFixEnabled], 00
        je originalcode

        cheat:
        //--------PlayerPos--------//
        push rax
        mov rax, [EnemySwapper::plPosBase]
        mov rax, [rax]
        mov rax, [rax+0x70]
        mov rax, [rax+0xC70]
        mov rax, [rax+0x40]
        mov rax, [rax+0xE40]
        mov rax, [rax+0x80]
        //--------PlayerPos--------//
        movss xmm0, [rax+0x30]
        movss xmm1, [rax+0x34]
        movss xmm2, [rax+0x38]
        pop rax

        originalcode:
        movss [rdx+0x00000D40], xmm0
        jmp qword ptr [EnemySwapper::goliathSuckJmpRet]
    }
}

static naked void goliath_jmpleave_detour()
{
    __asm {
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isGoliathFixEnabled], 00
        je originalcode

        cheat:
        //--------PlayerPos--------//
        push rax
        mov rax, [EnemySwapper::plPosBase]
        mov rax, [rax]
        mov rax, [rax+0x70]
        mov rax, [rax+0xC70]
        mov rax, [rax+0x40]
        mov rax, [rax+0xE40]
        mov rax, [rax+0x80]
        //--------PlayerPos--------//
        movss xmm0, [rax+0x30]
        movss xmm1, [rax+0x34]
        movss xmm2, [rax+0x38]
        pop rax

        originalcode:
        movss [r14+0x00000D30], xmm0
        jmp qword ptr [EnemySwapper::goliathLeaveJmpRet]
    }
}

static naked void artemis_centerfloor_fix()
{
    __asm {
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isArtemisFixEnabled], 00
        je originalcode

        cheat:
        mov eax, dword ptr [EnemySwapper::artemisCenterOfFloor.z]
        mov [rcx+0x14], eax
        cmp byte ptr [EnemySwapper::isArtemisPlayersXY], 1
        je plposload
        mov eax, dword ptr [EnemySwapper::artemisCenterOfFloor.x]
        mov [rcx+0x10], eax
        mov eax, dword ptr [EnemySwapper::artemisCenterOfFloor.y]
        mov [rcx+0x18], eax
        jmp originalcode

        plposload:
         //--------PlayerPos--------//
        mov rax, [EnemySwapper::plPosBase]
        mov rax, [rax]
        mov rax, [rax+0x70]
        mov rax, [rax+0xC70]
        mov rax, [rax+0x40]
        mov rax, [rax+0xE40]
        mov rax, [rax+0x80]
        //--------PlayerPos--------//
        push rbx
        mov ebx, dword ptr [rax+0x30]
        mov [rcx+0x10], ebx
        mov ebx, dword ptr [rax+0x38]
        mov [rcx+0x18], ebx
        pop rbx

        originalcode:
        movss xmm3, [rcx + 0x10]
        jmp qword ptr [EnemySwapper::artemisFixRet]
    }
}

static naked void urizen3tp_detour()
{
    __asm {
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isUrizen3FixEnabled], 00
        je originalcode

        cheat:
        jmp qword ptr [EnemySwapper::urizen3TpJne]

        originalcode:
        cmp qword ptr [rax+0x18], 00
        jne cheat
        jmp qword ptr [EnemySwapper::urizen3TpRet]
    }
}

static naked void malphas_tp_detour()
{
    __asm {
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isMalphasFixEnabled], 00
        je originalcode

        cheat:
        cmp byte ptr [EnemySwapper::malphasFixPlPos], 00
        je load_pos
        //--------PlayerPos--------//
        push rax
        mov rax, [EnemySwapper::plPosBase]
        mov rax, [rax]
        mov rax, [rax+0x70]
        mov rax, [rax+0xC70]
        mov rax, [rax+0x40]
        mov rax, [rax+0xE40]
        mov rax, [rax+0x80]
        //--------PlayerPos--------//
        movss xmm0, [rax+0x30]
        movss xmm1, [rax+0x34]
        movss xmm2, [rax+0x38]
        pop rax
        movss [rdi+0x00000090], xmm0
        movss [rdi+0x00000094], xmm1
        movss [rdi+0x00000098], xmm2
        jmp originalcode

        load_pos:
        movss xmm0, dword ptr [EnemySwapper::malphasCenterOfFloor.x]
        movss xmm1, dword ptr [EnemySwapper::malphasCenterOfFloor.z]
        movss xmm2, dword ptr [EnemySwapper::malphasCenterOfFloor.y]
        movss [rdi+0x00000090], xmm0
        movss [rdi+0x00000094], xmm1
        movss [rdi+0x00000098], xmm2

        originalcode:
        movss xmm0, [rdi + 0x00000090]
        jmp qword ptr [EnemySwapper::malphasRet]
    }
}

static naked void cerberus_pos_detour()
{
    __asm {
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isCerberusFixEnabled], 00
        je originalcode

        cheat:
        cmp byte ptr [EnemySwapper::cerberusFixPlPos], 00
        je load_pos
        //--------PlayerPos--------//
        push rax
        mov rax, [EnemySwapper::plPosBase]
        mov rax, [rax]
        mov rax, [rax+0x70]
        mov rax, [rax+0xC70]
        mov rax, [rax+0x40]
        mov rax, [rax+0xE40]
        mov rax, [rax+0x80]
        //--------PlayerPos--------//
        movss xmm8, [rax+0x30]
        movss xmm9, [rax+0x34]
        movss xmm10, [rax+0x38]
        pop rax
        movss [rdx+0x10], xmm8
        movss [rdx+0x14], xmm9
        movss [rdx+0x18], xmm10
        jmp originalcode

        load_pos:
        movss xmm8, [EnemySwapper::cerberusCenterOfFloor.x]
        movss xmm9, [EnemySwapper::cerberusCenterOfFloor.z]
        movss xmm10, [EnemySwapper::cerberusCenterOfFloor.y]
        movss [rdx+0x10], xmm8
        movss [rdx+0x14], xmm9
        movss [rdx+0x18], xmm10

        originalcode:
        movss xmm10, [rdx+0x18]
        jmp qword ptr [EnemySwapper::cerberusFixRet]
    }
}

static naked void cerberus_thunderwave_detour()
{
    __asm {
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isCerberusFixEnabled], 00
        je originalcode

        cheat:
        cmp byte ptr [EnemySwapper::cerberusThunderWavePlPos], 0
        je thunderstatic
        //--------PlayerPos--------//
        push rax
        mov rax, [EnemySwapper::plPosBase]
        mov rax, [rax]
        mov rax, [rax+0x70]
        mov rax, [rax+0xC70]
        mov rax, [rax+0x40]
        mov rax, [rax+0xE40]
        mov rax, [rax+0x80]
        //--------PlayerPos--------//
        movss xmm0, [rax+0x34]
        pop rax
        jmp change

        thunderstatic:
        movss xmm0, [EnemySwapper::cerberusThunderWaveZ]

        change:
        movss [rdi+0x24], xmm0
        movss [rdi+0x28], xmm0
        jmp qword ptr [EnemySwapper::cerberusThunderWaveRet]

        originalcode:
        maxss xmm0, [rdi+0x24]
        jmp qword ptr [EnemySwapper::cerberusThunderWaveRet]
    }
}

static naked void cerberus_thunderball_detour()//fu, capcom
{
    __asm {
        cmp byte ptr [WaveEditorMod::EnemyWaveEditor::cheaton], 01
        je option_check
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        option_check:
        cmp byte ptr [EnemySwapper::isCerberusFixEnabled], 00
        je originalcode

        jmp qword ptr [EnemySwapper::cerberusThunderBallJmp]

        originalcode:
        cmp [rax+0x00000FA9], r14d//r14l
        jmp qword ptr [EnemySwapper::cerberusThunderBallRet]
    }
}

void EnemySwapper::set_swapper_setting(int emListIndx, int swapToIndx) {
  selectedToSwap[emListIndx] = swapToIndx;
  swapSettings[emListIndx].set_current_id(swapToIndx);
  swapSettings[emListIndx].set_swap_id(selectedToSwap[swapToIndx]);
}

void EnemySwapper::set_swapper_settings(std::array<int ,enemyListCount> &swapToIndx)
{
  for (int i = 0; i < swapToIndx.size(); i++)
    set_swapper_setting(i, swapToIndx[i]);
}


void EnemySwapper::on_config_load(const utility::Config& cfg) {
  isSwapAll          = cfg.get<bool>("EnemySwapper.isSwapAll").value_or(false);
  selectedForAllSwap = cfg.get<uint32_t>("SwapAllEnemiesToID").value_or(0);
  swapForAll.set_swap_id(selectedForAllSwap);
  isCustomSpawnPos = cfg.get<bool>("EnemySwapper.isCustomSpawnPos").value_or(false);
  spawnPosZOffset  = cfg.get<float>("EnemySwapper.spawnPosZOffset").value_or(0.6f);
  spawnPosXOffset  = cfg.get<float>("EnemySwapper.spawnPosXOffset").value_or(0.0f);
  spawnPosYOffset  = cfg.get<float>("EnemySwapper.spawnPosYOffset").value_or(0.0f);
  isBossDanteAiEnabled = cfg.get<bool>("EnemySwapper.isBossDanteAiEnabled").value_or(false);
  isDanteM20 = cfg.get<bool>("EnemySwapper.isDanteM20").value_or(true);
  /*canKillGriffon = cfg.get<bool>("EnemySwapper.canKillGriffon").value_or(false);
  canKillShadow = cfg.get<bool>("EnemySwapper.canKillShadow").value_or(false);*/
  isNightmareFix  = cfg.get<bool>("EnemySwapper.isNightmareFix").value_or(false);
  isCavFixEnabled = cfg.get<bool>("EnemySwapper.isCavFixEnabled").value_or(false);
  isVergilFixEnabled = cfg.get<bool>("EnemySwapper.isVergilFixEnabled").value_or(false);
  isFastDiveBombAttack = cfg.get<bool>("EnemySwapper.isFastDiveBombAttack").value_or(false);
  isGoliathFixEnabled = cfg.get<bool>("EnemySwapper.isGoliathFixEnabled").value_or(false);
  isArtemisFixEnabled = cfg.get<bool>("EnemySwapper.isArtemisFixEnabled").value_or(false);
  isArtemisPlayersXY = cfg.get<bool>("EnemySwapper.isArtemisPlayersXY").value_or(false);
  isUrizen3FixEnabled = cfg.get<bool>("EnemySwapper.isUrizen3FixEnabled").value_or(false);
  isMalphasFixEnabled = cfg.get<bool>("EnemySwapper.isMalphasFixEnabled").value_or(false);
  isCerberusFixEnabled = cfg.get<bool>("EnemySwapper.isCerberusFixEnabled").value_or(false);
  cerberusFixPlPos = cfg.get<bool>("EnemySwapper.cerberusFixPlPos").value_or(false);
  cerberusThunderWavePlPos = cfg.get<bool>("EnemySwapper.cerberusThunderWavePlPos").value_or(true);
  nightmareStartPosOffs.x = cfg.get<float>("EnemySwapper.nightmareStartPosOffsX").value_or(135.83f);
  nightmareStartPosOffs.y = cfg.get<float>("EnemySwapper.nightmareStartPosOffsY").value_or(-112.45f);
  nightmareStartPosOffs.z = cfg.get<float>("EnemySwapper.nightmareStartPosOffsZ").value_or(82.784f);
  divebombHeightOfArenaSide = cfg.get<float>("EnemySwapper.divebombHeightOfArenaSide").value_or(-95.5f);
  divebombHeightOfOutside = cfg.get<float>("EnemySwapper.divebombHeightOfOutside").value_or(-95.0f);
  divebombDistanceGroundCheck = cfg.get<float>("EnemySwapper.divebombDistanceGroundCheck").value_or(100.0f);
  artemisCenterOfFloor.x = cfg.get<float>("EnemySwapper.artemisCenterOfFloorX").value_or(-368.0f);
  artemisCenterOfFloor.y = cfg.get<float>("EnemySwapper.artemisCenterOfFloorY").value_or(-308.5f);
  artemisCenterOfFloor.z = cfg.get<float>("EnemySwapper.artemisCenterOfFloorZ").value_or(1.15f);
  malphasCenterOfFloor.x = cfg.get<float>("EnemySwapper.malphasCenterOfFloorX").value_or(0.0f);
  malphasCenterOfFloor.y = cfg.get<float>("EnemySwapper.malphasCenterOfFloorY").value_or(0.0f);
  malphasCenterOfFloor.z = cfg.get<float>("EnemySwapper.malphasCenterOfFloorZ").value_or(0.0f);
  cerberusCenterOfFloor.x = cfg.get<float>("EnemySwapper.cerberusCenterOfFloorX").value_or(0.0f);
  cerberusCenterOfFloor.y = cfg.get<float>("EnemySwapper.cerberusCenterOfFloorY").value_or(0.0f);
  cerberusCenterOfFloor.z = cfg.get<float>("EnemySwapper.cerberusCenterOfFloorZ").value_or(-0.1f);
  cerberusThunderWaveZ = cfg.get<float>("EnemySwapper.cerberusThunderWaveZ").value_or(-0.1f);

  std::string key;
  uint32_t swapTo = 0;
  for (int i = 0; i < EnemySwapper::emNames.size(); i++) {
    key = std::string(EnemySwapper::emNames[i]) + "_swapTo";
    swapTo = cfg.get<uint32_t>(key).value_or(i);
    set_swapper_setting(i, swapTo);
  }
}

void EnemySwapper::on_config_save(utility::Config& cfg) {
  cfg.set<uint32_t>("SwapAllEnemiesToID", selectedForAllSwap);
  cfg.set<bool>("EnemySwapper.isCustomSpawnPos", isCustomSpawnPos);
  cfg.set<float>("EnemySwapper.spawnPosZOffset", spawnPosZOffset);
  cfg.set<float>("EnemySwapper.spawnPosXOffset", spawnPosXOffset);
  cfg.set<float>("EnemySwapper.spawnPosYOffset", spawnPosYOffset);
  cfg.set<bool>("EnemySwapper.isSwapAll", isSwapAll);
  cfg.set<bool>("EnemySwapper.isBossDanteAiEnabled", isBossDanteAiEnabled);
  cfg.set<bool>("EnemySwapper.canKillGriffon", canKillGriffon);
  cfg.set<bool>("EnemySwapper.canKillShadow", canKillShadow);
  cfg.set<bool>("EnemySwapper.isNightmareFix", isNightmareFix);
  cfg.set<bool>("EnemySwapper.isCavFixEnabled", isCavFixEnabled);
  cfg.set<bool>("EnemySwapper.isVergilFixEnabled", isVergilFixEnabled);
  cfg.set<bool>("EnemySwapper.isFastDiveBombAttack", isFastDiveBombAttack);
  cfg.set<bool>("EnemySwapper.isGoliathFixEnabled", isGoliathFixEnabled);
  cfg.set<bool>("EnemySwapper.isArtemisFixEnabled", isArtemisFixEnabled);
  cfg.set<bool>("EnemySwapper.isArtemisPlayersXY", isArtemisPlayersXY);
  cfg.set<bool>("EnemySwapper.isUrizen3FixEnabled", isUrizen3FixEnabled);
  cfg.set<bool>("EnemySwapper.isMalphasFixEnabled", isMalphasFixEnabled);
  cfg.set<bool>("EnemySwapper.isCerberusFixEnabled", isCerberusFixEnabled);
  cfg.set<bool>("EnemySwapper.cerberusFixPlPos", cerberusFixPlPos);
  cfg.set<bool>("EnemySwapper.cerberusThunderWavePlPos", cerberusThunderWavePlPos);
  cfg.set<float>("EnemySwapper.nightmareStartPosOffsX", nightmareStartPosOffs.x);
  cfg.set<float>("EnemySwapper.nightmareStartPosOffsY", nightmareStartPosOffs.y);
  cfg.set<float>("EnemySwapper.nightmareStartPosOffsZ", nightmareStartPosOffs.z);
  cfg.set<float>("EnemySwapper.divebombHeightOfArenaSide", divebombHeightOfArenaSide);
  cfg.set<float>("EnemySwapper.divebombHeightOfOutside", divebombHeightOfOutside);
  cfg.set<float>("EnemySwapper.divebombDistanceGroundCheck", divebombDistanceGroundCheck);
  cfg.set<float>("EnemySwapper.artemisCenterOfFloorX", artemisCenterOfFloor.x);
  cfg.set<float>("EnemySwapper.artemisCenterOfFloorY", artemisCenterOfFloor.y);
  cfg.set<float>("EnemySwapper.artemisCenterOfFloorZ", artemisCenterOfFloor.z);
  cfg.set<float>("EnemySwapper.malphasCenterOfFloorX", malphasCenterOfFloor.x);
  cfg.set<float>("EnemySwapper.malphasCenterOfFloorY", malphasCenterOfFloor.y);
  cfg.set<float>("EnemySwapper.malphasCenterOfFloorZ", malphasCenterOfFloor.z);
  cfg.set<float>("EnemySwapper.cerberusCenterOfFloorX", cerberusCenterOfFloor.x);
  cfg.set<float>("EnemySwapper.cerberusCenterOfFloorY", cerberusCenterOfFloor.y);
  cfg.set<float>("EnemySwapper.cerberusCenterOfFloorZ", cerberusCenterOfFloor.z);
  cfg.set<float>("EnemySwapper.cerberusThunderWaveZ", cerberusThunderWaveZ);

  for (int i = 0; i < EnemySwapper::emNames.size(); i++) {
    cfg.set<uint32_t>(std::string(EnemySwapper::emNames[i]) + "_swapTo", selectedToSwap[i]);
  }
}

// void EnemySwapper::on_frame() {}

void EnemySwapper::restore_default_settings() {
  for (int i = 0; i < EnemySwapper::enemyListCount; i++)
    set_swapper_setting(i, i);
  EnemySwapper::curMinIndx = minIndx;
  EnemySwapper::curMaxIndx = maxIndx;
  seed       = -1;
}

void set_Dante_ai() {
    ImGui::TextWrapped("Use this option if you want to swap enemies with boss Dante. Doesn't work on BP stages greater than 19. Can't be changed during gameplay.");
    ImGui::Checkbox("Fix boss Dante AI/Enable boss Vergil mission AI", &EnemySwapper::isBossDanteAiEnabled);
    if (EnemySwapper::isBossDanteAiEnabled) {
        ImGui::TextWrapped("Select this to specify Dante/Vergil AI type.\n"
            "Killing boss Vergil with m19 AI on missions/BP will cause a soft lock.");
        //ImGui::Checkbox("Use boss Dante/Vergil M20 AI", &EnemySwapper::isDanteM20);
        if(ImGui::RadioButton("Use M19 AI", !EnemySwapper::isDanteM20))
            EnemySwapper::isDanteM20 = false;
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        if (ImGui::RadioButton("Use M20 AI", EnemySwapper::isDanteM20))
            EnemySwapper::isDanteM20 = true;
    }
}

void print_issues(const char* str) {
    ImGui::TextWrapped(str);
  ImGui::Spacing();
}

void EnemySwapper::on_draw_ui() {
  if (ImGui::CollapsingHeader("Current Issues")){
    print_issues("Killing enemies swapped with a boss in mission can cause BGM issues.");
    print_issues("Only 1 type of Dante's/Vergil's AI can be loaded for mission/BP floor.");
    print_issues("Dante AI fix doesn't work on most of BP stages.");
    print_issues("Bp stages will not end with Dante boss. Same with Griffon and Shadow.");
    print_issues("Some of battle arenas may be softlocked with boss Dante.");
    print_issues("Game may softlock if you skip swapped Dante boss fight with AI fix.");
    print_issues("Enemy Dante doesn't dealloc memory after his death. So swap all enemies to Dante on missions can crash the game. Also Dante's bodies doesn't disappears.");
    print_issues("Nightmare fix can make meteor fast. Too fast.");
    print_issues("Griffon and Shadow bodies with kill fixes doesn't disappears.");
    print_issues("Force killing Griffon and Shadow can broke mission 18.");
    print_issues("Nightmare can teleports to nowhere to heal familiars if they also in a fight.");
    print_issues("Swapping the Qliphoth root boss can softlock mission 1.");
    print_issues("Swapping Dante can softlock mission 20.");
    print_issues("Wrong swap on some BP stages sometimes.");
    print_issues("Some BP stages softlock when swapping enemies to qliphoth tentacles.");
    ImGui::Separator();
  }

  if (ImGui::CollapsingHeader("Enemy fixes")) {
      ImGui::Spacing();
      set_Dante_ai();
      ImGui::Separator();
      ImGui::TextWrapped("Fix enemy familiars.");
      /*ImGui::Checkbox("Enable kill shadow", &canKillShadow);
      ImGui::Checkbox("Enable kill griffon", &canKillGriffon);*/
      ImGui::Checkbox("Fix Nightmare meteor position", &isNightmareFix);
      if (isNightmareFix) {
          ImGui::TextWrapped("Offset from player position to start the meteor at.");
          ImGui::InputFloat("X offset", &nightmareStartPosOffs.x, 0.0f, 0.0f, "%.2f");
          ImGui::InputFloat("Y offset", &nightmareStartPosOffs.y, 0.0f, 0.0f, "%.2f");
          ImGui::InputFloat("Z offset", &nightmareStartPosOffs.z, 0.0f, 0.0f, "%.2f");
      }
      ImGui::Separator();
      ImGui::Spacing();
      ImGui::Checkbox("Fix Cavaliere teleports", &isCavFixEnabled);
      ImGui::ShowHelpMarker("Replace pre-coded teleport positions with offset from player position. "
          "Can result in Cavaliere becoming stuck in a teleporting state.");
      if (isCavFixEnabled)
      {
          ImGui::TextWrapped("X offset");
          ImGui::InputFloat("##cavXOffst", &cavOffset.x, 0, 0, "%.2f");
          ImGui::TextWrapped("Y offset");
          ImGui::InputFloat("##cavYOffst", &cavOffset.y, 0, 0, "%.2f");
          ImGui::TextWrapped("Z offset");
          ImGui::InputFloat("##cavZOffst", &cavOffset.z, 0, 0, "%.2f");
      }
      if (isVergilFixEnabled)
          ImGui::Separator();
      ImGui::Checkbox("Fix Vergil Dive Bomb", &isVergilFixEnabled);
      ImGui::ShowHelpMarker("Replace pre-coded divebomb position with player coordinates.");
      if (isVergilFixEnabled)
      {
          ImGui::TextWrapped("Height of arena sides  (default = 1.5)");
          ImGui::InputFloat("##divebombHeightOfArenaSide", &divebombHeightOfArenaSide, 0, 0, "%.1f");
          ImGui::TextWrapped("Height outside arena (default = 8)");
          ImGui::InputFloat("##divebombHeightOfOutside", &divebombHeightOfOutside, 0, 0, "%.1f");
          ImGui::TextWrapped("Distance from ground (default = 5)");
          ImGui::InputFloat("##divebombDistanceGroundCheck", &divebombDistanceGroundCheck, 0, 0, "%.1f");
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
          if (!isArtemisPlayersXY)
          {
              ImGui::Spacing();
              btn_set_plpos_to(artemisCenterOfFloor, "Set arena center to current player position ##Art");
              ImGui::Spacing();
              ImGui::InputFloat("X coordinate ##Art", &artemisCenterOfFloor.x, 0.1f, 0, "%.2f");
              ImGui::InputFloat("Y coordinate ##Art", &artemisCenterOfFloor.y, 0.1f, 0, "%.2f");
          }
          ImGui::InputFloat("Height of arena center ##Art", &artemisCenterOfFloor.z, 0.1f, 0, "%.2f");
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
          if (!malphasFixPlPos)
          {
              btn_set_plpos_to(malphasCenterOfFloor, "Set arena center to current player position ##Malph");
              ImGui::InputFloat("X coord of arena center ##Malph", &malphasCenterOfFloor.x, 0.1f, 0, "%.2f");
              ImGui::InputFloat("Y coord of arena center ##Malph", &malphasCenterOfFloor.y, 0.1f, 0, "%.2f");
              ImGui::InputFloat("Z coord of arena center ##Malph", &malphasCenterOfFloor.z, 0.1f, 0, "%.2f");
          }
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
          if (!cerberusFixPlPos)
          {
              btn_set_plpos_to(cerberusCenterOfFloor, "Set arena center to current player position##Cerberus");
              ImGui::InputFloat("X coord of center of arena ##Cerberus", &cerberusCenterOfFloor.x, 0.1f, 0, "%.2f");
              ImGui::InputFloat("Y coord of center of arena ##Cerberus", &cerberusCenterOfFloor.y, 0.1f, 0, "%.2f");
              ImGui::InputFloat("Z coord of center of arena ##Cerberus", &cerberusCenterOfFloor.z, 0.1f, 0, "%.2f");
              cerberusThunderWaveZ = cerberusCenterOfFloor.z;
              cerberusThunderWavePlPos = false;
          }
          else cerberusThunderWavePlPos = true;
          ImGui::Separator();
          ImGui::Spacing();
          /*ImGui::Checkbox("Use current player z coord to spawn thunder wave attack", &cerberusThunderWavePlPos);
          if (!cerberusThunderWavePlPos)
              ImGui::InputFloat("Z for thunder wave", &cerberusThunderWaveZ, 0.1f, 0.0f, "%.2f");*/

      }
      ImGui::Checkbox("Change XYZ spawn coordinates", &isCustomSpawnPos);
      ImGui::ShowHelpMarker("Fixes some enemies spawning under the floor. Note that this will affect all spawns and can change spawn animations.");
      if (isCustomSpawnPos) {
          ImGui::TextWrapped("Height offset");
          ImGui::SliderFloat("##spawnPosZOffsetSlider", &spawnPosZOffset, 0.0f, 12.0f, "%.1f");
          ImGui::TextWrapped("Changing XY coordinates can fix enemies spawning behind invisible walls (i.e. swapping Urizen 1, Nidhogg or Qliphot roots with other enemies).");
          ImGui::TextWrapped("X offset");
          ImGui::SliderFloat("##spawnPosXOffsetSlider", &spawnPosXOffset, -12.0f, 12.0f, "%.1f");
          ImGui::TextWrapped("Y offset");
          ImGui::SliderFloat("##spawnPosYOffsetSlider", &spawnPosYOffset, -12.0f, 12.0f, "%.1f");
      }
      ImGui::Separator();
      ImGui::Spacing();
  }
  /*ImGui::TextWrapped("Vector itemSize is: %d", setDataAddrs.itemSize());
  ImGui::Spacing();
  ImGui::TextWrapped("ReswapCount: %d", reswapCount);*/

  //ImGui::Separator();






  ImGui::Checkbox("Swap all enemies", &isSwapAll);
  if (isSwapAll) {
    ImGui::TextWrapped("Swap to:");
    ImGui::SameLine();
    ImGui::Combo("##Swap to", (int*)&selectedForAllSwap, emNames.data(), emNames.size(),20);
    swapForAll.set_swap_id(selectedForAllSwap);
  } 
  else {


    /*if (ImGui::Button("Apply")) {
      for (int i = 0; i < emNames.itemSize(); i++) {
        set_swapper_setting(i, selectedToSwap[i]);
      }
    }*/

    ImGui::Columns(4, NULL, false);
    for (int i = 0; i < EnemySwapper::emNames.size(); i++) {
        if (i == 32 || i == 33)//Urizens
            continue;

        bool state = (selectedToSwap[i] != i);
        ImVec4 backgroundcolor = state ? ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered) : SELECTABLE_STYLE_ACT;
        if (i == index)
            backgroundcolor = SELECTABLE_STYLE_HVR;
        ImGui::PushStyleColor(ImGuiCol_Header, backgroundcolor);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, SELECTABLE_STYLE_ACT_HVR);

        
        if (state) {
            uniqStr = std::string(EnemySwapper::emNames[i] + std::string(" ->") + EnemySwapper::emNames[selectedToSwap[i]]);
        }
        else {
            uniqStr = EnemySwapper::emNames[i];
        }

        if (ImGui::Selectable(uniqStr.c_str(), state, ImGuiSelectableFlags_AllowDoubleClick)) {
            if (ImGui::IsMouseDoubleClicked(0)) {
                selectedToSwap[i] = i;
            }
            else {
                index = i;
            }
        }
        ImGui::PopStyleColor(2);
        ImGui::NextColumn();
    }
    if (ImGui::Button("Restore default settings"))
        restore_default_settings();

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextWrapped(emNames[index]);
    uniqComboStr = "##SwapToCustom";
    ImGui::Combo(uniqComboStr.c_str(), (int*)&selectedToSwap[index], emNames.data(), emNames.size(), 20);
    //set_swapper_setting(i, selectedToSwap[i]);
    swapSettings[index].set_current_id(index);
    swapSettings[index].set_swap_id(selectedToSwap[index]);
    ImGui::Spacing();
    ImGui::Separator();

    /*
    for (int i = 0; i < emNames.size(); i++) {
        if (i % 3 == 0) {
            ImGui::Columns(1);
            ImGui::Separator();
            ImGui::Columns(3, NULL, false);
        }
      ImGui::TextWrapped(emNames[i]);
      uniqComboStr = std::to_string(i) + "##SwapTo";
      ImGui::Combo(uniqComboStr.c_str(), (int*)&selectedToSwap[i], emNames.data(), emNames.size(), 20);
      //set_swapper_setting(i, selectedToSwap[i]);
      swapSettings[i].set_current_id(i);
      swapSettings[i].set_swap_id(selectedToSwap[i]);
      ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();
    */

    
    if (ImGui::Button("Randomize enemies", ImVec2(165, 25))) {
        random_em_swap(curMinIndx, curMaxIndx);
    }

    ImGui::Checkbox("Change randomizer settings", &isCustomRandomSettings);
    if (isCustomRandomSettings) {
      ImGui::Checkbox("Use custom seed", &isCustomSeed);
      if (isCustomSeed) {
        ImGui::Text("Seed:");
        if (ImGui::InputInt("##seedInput", &seed))
          seed_rnd_gen(seed);
      } else
        seed = -1;
      ImGui::TextWrapped("Controls the pool of enemies that can be selected from when randomizing");
      ImGui::Columns(2, NULL, false);
      ImGui::TextWrapped("Min enemy index:");
      ImGui::SliderInt("##minIndxSlider", &curMinIndx, 0, 39);
      ImGui::NextColumn();
      ImGui::TextWrapped("Max enemy index:");
      ImGui::SliderInt("##maxIndxSlider", &curMaxIndx, 1, 40);
      ImGui::Columns(1);
      if (curMinIndx >= curMaxIndx)
        curMinIndx = curMaxIndx - 1;
    } else {
      curMinIndx = minIndx;
      curMaxIndx = maxIndx;
    }

    
  }
  
  /*
  ImGui::Separator();
  ImGui::TextWrapped("Some debug shit");
  ImGui::Spacing();
  if (ImGui::Button("Clear vector")) {
    setDataAddrs.clear();
    reswapCount = 0;
  }
  ImGui::TextWrapped("Clear and reserve space for vector of enemyData addresses");
  ImGui::InputInt("NewSize", (int*)&reservedForReswap);
  if (ImGui::Button("reserve")) {
    reserveReswapVector(reservedForReswap);
  }
*/
}

void EnemySwapper::btn_set_plpos_to(Vector3f& to, const char* btnContent)
{
    if (ImGui::Button(btnContent))
    {
        if (nowFlow == 22)
            to = CheckpointPos::get_player_coords();
    }
}

void EnemySwapper::on_draw_debug_ui() {
  //ImGui::TextWrapped("Vector itemSize is: %d", setDataAddrs.itemSize());
  /*ImGui::TextWrapped("nowFlow: %d", nowFlow);
  ImGui::Spacing();
  ImGui::TextWrapped("prevFlow: %d", prevFlow);
  ImGui::Spacing();*/
}

void EnemySwapper::reserveReswapVector(size_t newSize) {
  reservedForReswap = newSize;
  setDataAddrs.resize(0);
  setDataAddrs.reserve(newSize);
}

void EnemySwapper::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

void EnemySwapper::seed_rnd_gen(int seed) {
  if (seed == -1)
    gen.seed(rd());
  else
    gen.seed(seed);
}

void EnemySwapper::random_em_swap(uint32_t min, uint32_t max) {
  std::uniform_int_distribution<> distrib(min, max);
  for (int i = 0; i < 20; i++) {//Rand only for regular enemies
    set_swapper_setting(i, distrib(gen));
  }
}

std::optional<std::string> EnemySwapper::on_initialize() {
  init_check_box_info();
  auto base      = g_framework->get_module().as<HMODULE>(); // note HMODULE

  m_is_enabled = &cheaton;
  m_on_page         = balance;
  m_full_name_string   = "Enemy Swapper(+)";
  m_author_string      = "VPZadov";
  m_description_string = "Swap enemy spawns. Effects normal spawns & hell judecca summons.";

  plPosBase = g_framework->get_module().as<uintptr_t>() + 0x07E625D0;

  auto initAddr1 = patterns->find_addr(base, "8B 71 10 48 85 C0 0F 84 43");// "DevilMayCry5.exe"+FE568B //For BP custom swap
  if (!initAddr1) {
    return "Unanable to find EnemySwapper pattern.";
  }

  auto initAddr2 = patterns->find_addr(base, "44 8B 41 10 48 8B D7 48 8B CB E8 F8"); // DevilMayCry5.exe+FE57A9 //1st op
  if (!initAddr2) {
    return "Unanable to find EnemySwapper pattern.";
  }

  //auto tempcustomSpawnAddr = g_framework->get_module().as<uintptr_t>() + 0x11C58BC;

  /*auto initAddr4 = g_framework->get_module().as<uintptr_t>() + 0x259BBC9;
  setEnemyData4Jmp = g_framework->get_module().as<uintptr_t>() + 0x259B5D4;*/

   auto initAddr5 = g_framework->get_module().as<uintptr_t>() + 0xCA17A4;
  

  auto customSpawnAddr = patterns->find_addr(base, "F3 41 0F11 4D 34 48 8B 4B 18 48 85 C9 75 10"); // DevilMayCry5.exe+11C58BC
  if (!customSpawnAddr) {
    return "Unanable to find customSpawnAddr pattern.";
  }

  auto initAddr3 = patterns->find_addr(base, "41 8B 4E 10 48 85 C0 74"); // DevilMayCry5.exe+F34255
  if (!customSpawnAddr) {
    return "Unanable to find EnemySwapper.initAddr2 pattern.";
  }

  auto initAddr6 = patterns->find_addr(base, "45 8B 76 10 4C 89 7C 24 40"); // DevilMayCry5.exe+BD4691
  if (!initAddr6) {
    return "Unanable to find EnemySwapper.initAddr6 pattern.";
  }

  /*auto initAddr7 = patterns->find_addr(base, "45 8B 76 10 4C 89 7C 24 40"); // 
  if (!initAddr6) {
    return "Unanable to find EnemySwapper.initAddr7 pattern.";
  }*/

  auto m19CheckAddr = patterns->find_addr(base, "83 78 7C 13 75 11"); //DevilMayCry5.exe+1D47B50
  if (!m19CheckAddr) {
    return "Unanable to find EnemySwapper.m19CheckAddr pattern.";
  }

  auto griffonKillAddr = patterns->find_addr(base, "80 BF 48 0F 00 00 00"); //DevilMayCry5.exe+F9E17B
  if (!griffonKillAddr) {
    return "Unanable to find EnemySwapper.griffonKillAddr pattern.";
  }

  auto shadowKillAddr = patterns->find_addr(base, "80 BF 60 0F 00 00 00"); //DevilMayCry5.exe+F5B07B
  if (!shadowKillAddr) {
    return "Unanable to find EnemySwapper.shadowKillAddr pattern.";
  }

  auto nightmireStartingAddr = patterns->find_addr(base, "48 8B D7 F3 0F 11 45 97 F3 0F 11"); //DevilMayCry5.exe+20DEFFD
  if (!nightmireStartingAddr) {
    return "Unanable to find EnemySwapper.nightmareStartingMeteorPosAddr pattern.";
  }

  auto nightmireArrivalAddr = patterns->find_addr(base, "12 F3 44 0F 11 50 30"); //DevilMayCry5.exe+16A13EB
  if (!nightmireArrivalAddr) {
    return "Unanable to find EnemySwapper.nightmareArrivalAddr pattern.";
  }

  //auto cavTeleportPosAddr = patterns->find_addr(base, "F3 41 0F 11 85 20 0E 00 00 F3 41"); //DevilMayCry5.exe+1290351
  //if (!cavTeleportPosAddr)
  //{
  //    return "Unanable to find EnemySwapper.cavTeleportPosAddr pattern.";
  //}

  auto cavTeleportPosAddr = patterns->find_addr(base, "73 FD FF FF F3 0F 10 47 10"); //DevilMayCry5.exe+1290342 (-0x4)
  if (!cavTeleportPosAddr)
  {
      return "Unanable to find EnemySwapper.cavTeleportPosAddr pattern.";
  }

  auto vergilCenterFloorAddr = patterns->find_addr(base, "04 00 00 F3 0F 10 80 A0 00 00 00 F3"); //DevilMayCry5.exe+9AE0B7 (-0x3);
  if (!vergilCenterFloorAddr)
  {
      return "Unanable to find EnemySwapper.vergilCenterFloorAddr pattern.";
  }
  //auto vergilCenterFloor1Addr = patterns->find_addr(base, "E3 F3 0F 10 80 A0 00 00 00"); // DevilMayCry5.exe+1CBB022 (-0x1);
  //if (!vergilCenterFloor1Addr)
  //{
  //    return "Unanable to find EnemySwapper.vergilCenterFloor1Addr pattern.";
  //}

  auto vergilAirRaidControllerAddr = patterns->find_addr(base, "F3 0F 10 B8 A0 00 00 00 F3"); //DevilMayCry5.exe+1CBD779
  if (!vergilAirRaidControllerAddr)
  {
      return "Unanable to find EnemySwapper.vergilAirRaidControllerAddr pattern.";
  }

  auto goliathSuckJmpAddr = patterns->find_addr(base, "00 F3 0F 11 82 40 0D 00 00"); //DevilMayCry5.exe+12F2545 (-0x1)
  if (!goliathSuckJmpAddr)
  {
      return "Unanable to find EnemySwapper.goliathSuckJmpAddr pattern.";
  }

  auto goliathLeaveJmpAddr = patterns->find_addr(base, "F3 41 0F 11 86 30 0D 00 00"); //DevilMayCry5.exe+12F2455
  if (!goliathLeaveJmpAddr)
  {
      return "Unanable to find EnemySwapper.goliathLeaveJmpAddr pattern.";
  }

  auto artemisCenterOfFloorAddr = patterns->find_addr(base, "B7 F3 0F 10 59 10"); //DevilMayCry5.exe+1E9CC1D (-0x1)
  if (!artemisCenterOfFloorAddr)
  {
      return "Unanable to find EnemySwapper.artemisCenterOfFloorAddr pattern.";
  }

  auto urizen3TpAddr = patterns->find_addr(base, "48 83 78 18 00 75 93 F3 0F 10 87 E0"); //DevilMayCry5.exe+1189760
  if (!urizen3TpAddr)
  {
      return "Unanable to find EnemySwapper.urizen3TpAddr pattern.";
  }

  auto malphasAddr = patterns->find_addr(base, "F3 0F 10 87 90 00 00 00 F3 0F 11 4D B4"); //DevilMayCry5.exe+105B4C3
  if (!urizen3TpAddr)
  {
      return "Unanable to find EnemySwapper.malphasAddr pattern.";
  }

  auto cerberusAddr = patterns->find_addr(base, "F3 44 0F 10 52 18 48 8B D3"); //DevilMayCry5.exe+111D60E
  if (!cerberusAddr)
  {
      return "Unanable to find EnemySwapper.cerberusAddr pattern.";
  }

  auto cerberusThunderWaveAddr = patterns->find_addr(base, "F3 0F 5F 47 24"); //DevilMayCry5.exe+1121E1B
  if (!cerberusThunderWaveAddr)
  {
      return "Unanable to find EnemySwapper.cerberusThunderWaveAddr pattern.";
  }

  auto cerberusThunderBallAddr = patterns->find_addr(base, "44 38 B0 A9 0F 00 00"); //DevilMayCry5.exe+1F39983
  if (!cerberusThunderBallAddr)
  {
      return "Unanable to find EnemySwapper.cerberusThunderBallAddr pattern.";
  }

  auto nowFlowAddr = patterns->find_addr(base, "48 8B DA 48 8B F9 83 F8 1A"); // DevilMayCry5.exe+89429E 
  if (!nowFlowAddr)
  {
      return "Unanable to find EnemySwapper.nowFlowAddr pattern.";
  }

  auto gameModeAddr = patterns->find_addr(base, "39 B8 90 00 00 00 48"); // DevilMayCry5.exe+132B592
  if (!nowFlowAddr)
  {
      return "Unanable to find EnemySwapper.gameMode pattern.";
  }

  //uintptr_t swapIdAddr = g_framework->get_module().as<uintptr_t>() + 0xF34F6A;

  const uintptr_t spawnAddrOffset = 0xA;

  posSpawnTestJne = customSpawnAddr.value() + spawnAddrOffset + 0x15;
  bossDanteAiJne  = m19CheckAddr.value() + 0x17;
  vergilFixJs = vergilCenterFloorAddr.value() + 0x3 + 0x18;
  urizen3TpJne = urizen3TpAddr.value() - 0x66;
  cerberusThunderBallJmp = cerberusThunderBallAddr.value() + 0x1C6;

  if (!install_hook_absolute(initAddr1.value(), m_enemy_swapper_hook1, &enemy_swap_detour1, &setEnemyDataRet1, 0x6)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr1"; 
  }

  if (!install_hook_absolute(initAddr2.value(), m_enemy_swapper_hook2, &enemy_swap_detour2, &setEnemyDataRet2, 0x7)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr2"; 
  }

  if (!install_hook_absolute(customSpawnAddr.value() + spawnAddrOffset, m_spawn_pos_hook, &spawn_pos_detour, &posSpawnRet, 0x5)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.customSpawnAddr"; 
  }

  if (!install_hook_absolute(initAddr3.value(), m_enemy_swapper_hook3, &enemy_swap_detour3, &setEnemyDataRet3, 0x7)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.customSpawnAddr"; 
  }

  /*if (!install_hook_absolute(initAddr4, m_enemy_swapper_hook4, &enemy_swap_detour4, &setEnemyDataRet4, 0x8)) {//BadPointer
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr4"; 
  }*/

  if (!install_hook_absolute(initAddr5, m_enemy_swapper_hook5, &enemy_swap_detour5, &setEnemyDataRet5, 0x7)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr5"; 
  }

  if (!install_hook_absolute(initAddr6.value(), m_enemy_swapper_hook6, &enemy_swap_detour6, &setEnemyDataRet6, 0x9)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.initAddr6"; 
  }

  if (!install_hook_absolute(m19CheckAddr.value(), m_m19check_hook, &load_Dante_ai_detour, &bossDanteAiRet, 0x6)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.gameModeAddr"; 
  }

  if (!install_hook_absolute(griffonKillAddr.value(), m_griffon_hook, &kill_griffon_detour, &killGriffonRet, 0x7)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.griffonKill"; 
  }

  if (!install_hook_absolute(shadowKillAddr.value(), m_shadow_hook, &kill_shadow_detour, &killShadowRet, 0x7)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.griffonKill"; 
  }

  if (!install_hook_absolute(nightmireStartingAddr.value(), m_nightmire_starting_hook, &nightmire_starting_detour, &nightmareStartingPosRet, 0x8)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.nightmireStartingMeteorPos"; 
  }

  if (!install_hook_absolute(nightmireArrivalAddr.value()+0x1, m_nightmire_arrival_hook, &nightmire_arrival_detour, &nightmareArrivalPosRet, 0x6)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.nightmireArrivalMeteorPos"; 
  }

  /*if (!install_hook_absolute(cavTeleportPosAddr.value(), m_cavfix_hook, &cavtele_detour, &cavFixRet, 0x9))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.cavTeleportPos";
  }*/

  if (!install_hook_absolute(cavTeleportPosAddr.value()+0x4, m_cavfix_hook, &cavtele_detour, &cavFixRet, 0x5))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.cavTeleportPos";
  }

  if (!install_hook_absolute(vergilCenterFloorAddr.value() + 0x3, m_vergilcenterfloor_hook, &vergil_centerfloor_detour, &vergilFixRet, 0x8))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.vergilCenterFloor";
  }

  if (!install_hook_absolute(vergilAirRaidControllerAddr.value(), m_airraid_controller_hook, &airraid_detour, &airRaidControllerRet, 0x8))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.vergilAirRaidController";
  }

  if (!install_hook_absolute(goliathSuckJmpAddr.value()+0x1, m_goliath_suctionjmp_hook, &goliath_jmpsuck_detour, &goliathSuckJmpRet, 0x8))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.goliathSuckJmp";
  }

  if (!install_hook_absolute(goliathLeaveJmpAddr.value(), m_goliath_leavejmp_hook, &goliath_jmpleave_detour, &goliathLeaveJmpRet, 0x9))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.goliathLeaveJmp";
  }

  if (!install_hook_absolute(artemisCenterOfFloorAddr.value()+0x1, m_artemis_centerfloor_hook, &artemis_centerfloor_fix, &artemisFixRet, 0x5))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.artemisCenterOfFloor";
  }

  if (!install_hook_absolute(urizen3TpAddr.value(), m_urizen3_tp_hook, &urizen3tp_detour, &urizen3TpRet, 0x7))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.urizen3Tp";
  }

  if (!install_hook_absolute(malphasAddr.value(), m_malphas_tp_hook, &malphas_tp_detour, &malphasRet, 0x8))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.malphas";
  }

  if (!install_hook_absolute(cerberusAddr.value(), m_cerberus_pos_hook, &cerberus_pos_detour, &cerberusFixRet, 0x6))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.cerberus";
  }

  if (!install_hook_absolute(cerberusThunderWaveAddr.value(), m_cerberus_thunderwave_hook, &cerberus_thunderwave_detour, &cerberusThunderWaveRet, 0x5))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.cerberusThunderWave";
  }

  if (!install_hook_absolute(cerberusThunderBallAddr.value(), m_cerberus_thunderball_hook, &cerberus_thunderball_detour, &cerberusThunderBallRet, 0x7))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.cerberusThunderWave";
  }

  if (!install_hook_absolute(nowFlowAddr.value(), m_now_flow_hook, &now_flow_detour, &nowFlowRet, 0x6))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.nowFlowAddr";
  }

  /*if (!install_hook_absolute(swapIdAddr, m_enemy_swapper_hook7, &swap_id_detour, &swapIdRet, 0x7)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.swapIdAddr";
  }*/

  if (!install_hook_absolute(gameModeAddr.value(), m_gamemode_hook, &gamemode_detour, &gameModeRet, 0x6))
  {
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize EnemySwapper.gameModeAddr";
  }

  seed_rnd_gen(-1);
  EnemySwapper::setDataAddrs.reserve(reservedForReswap);
  //setDataAddrs = new std::vector<uintptr_t>();
    return Mod::on_initialize();
}