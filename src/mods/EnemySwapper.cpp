#include "EnemySwapper.hpp"
#include <string>
#include "sdk/ReClass.hpp"
#include "EnemyDataSettings.hpp"

bool EnemySwapper::cheaton{NULL};
bool EnemySwapper::isSwapAll{NULL};
bool EnemySwapper::isCustomRandomSettings{false};
bool EnemySwapper::isCustomSeed{false};
bool EnemySwapper::isCustomSpawnPos{NULL};
bool EnemySwapper::isBossDanteAiEnabled{NULL};
bool EnemySwapper::isDanteM20{NULL};
bool isReswap = false;
bool isSkipNum = false;
bool checkForReswap = false;
bool defaultEmSetting = true;

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


std::string uniqComboStr = "";//For comboboxes

std::array<EnemySwapper::EnemyId, EnemySwapper::enemyListCount> EnemySwapper::swapSettings;//cur id - id to swap

EnemySwapper::EnemyId EnemySwapper::swapForAll;

std::vector<uintptr_t> EnemySwapper::setDataAddrs;// = new std::vector<uintptr_t>();

uintptr_t curSetDataAddr = 0;
//std::mutex mtx;

//std::mutex EnemySwapper::mtx;

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
        mov byte ptr [r15+0x5C], 00  // IsEndSaveData
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
        mov byte ptr [r15+0x5C], 00 // IsEndSaveData

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

/*static naked void swap_id_detour() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        cmp byte ptr [EnemySwapper::isSwapAll], 1
        je swapall

        swapsetup:
        mov byte ptr [checkForReswap], 0
        mov r8d, [rax+0x10] 
        mov [EnemySwapper::currentEnemyId], r8d
        mov [EnemySwapper::swapIdBackup.rax], rax
        mov [EnemySwapper::swapIdBackup.rbx], rbx
        mov [EnemySwapper::swapIdBackup.rcx], rcx
        mov [EnemySwapper::swapIdBackup.rdx], rdx
        mov [EnemySwapper::swapIdBackup.rsi], rsi
        //mov [EnemySwapper::swapIdBackup.r8], r8
        mov [EnemySwapper::swapIdBackup.r9], r9
        mov [EnemySwapper::swapIdBackup.r10], r10
        mov [EnemySwapper::swapIdBackup.r11], r11

        call [custom_swap_setup]

        mov rax, [EnemySwapper::swapIdBackup.rax]
        mov rbx, [EnemySwapper::swapIdBackup.rbx]
        mov rcx, [EnemySwapper::swapIdBackup.rcx]
        mov rdx, [EnemySwapper::swapIdBackup.rdx]
        mov rsi, [EnemySwapper::swapIdBackup.rsi]
        //mov r8, [EnemySwapper::swapIdBackup.r8]
        mov r9, [EnemySwapper::swapIdBackup.r9]
        mov r10, [EnemySwapper::swapIdBackup.r10]
        mov r11, [EnemySwapper::swapIdBackup.r11]
        mov r8d, [EnemySwapper::newEnemyId]
        mov [rax+0x10], r8d

        originalcode:
        mov r8d, [rax+0x10]
        mov rcx, rsi
        jmp qword ptr [EnemySwapper::swapIdRet]

        swapall:
        mov r8d, dword ptr [EnemySwapper::swapForAll.swapId]
        mov [rax+0x10], r8d
        jmp originalcode
  }
}*/

static void clear_setData_addrs() {
  if (EnemySwapper::setDataAddrs.size() != 0)
    EnemySwapper::setDataAddrs.clear();
  //reswapCount = 0;
}

static naked void spawn_pos_detour() {
    __asm {
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
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

static naked void now_flow_detour() {
    __asm {
        mov dword ptr [EnemySwapper::nowFlow], eax
        cmp [flowTmp], eax
        jne changeprev
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode
        jmp check

        changeprev:
        mov eax, [flowTmp]
        mov [EnemySwapper::prevFlow], eax
        mov eax, dword ptr [EnemySwapper::nowFlow]
        mov [flowTmp], eax
        cmp byte ptr [EnemySwapper::cheaton], 0
        je originalcode

        check:
        cmp eax, 0xE //14-MainMenu
        je clear
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

        originalcode:
        mov rbx,rdx
        mov rdi,rcx
        jmp qword ptr [EnemySwapper::nowFlowRet]
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
        cmp byte ptr [EnemySwapper::cheaton], 00
        je originalcode
        cmp byte ptr [EnemySwapper::isBossDanteAiEnabled], 00
        je originalcode

        cheat:
        cmp byte ptr [EnemySwapper::isDanteM20], 01
        je dante_m19
        mov dword ptr [rax+0x7C], 0x13

        originalcode:
        cmp dword ptr [rax+0x7C], 0x13
        je ret_je
        jmp qword ptr [EnemySwapper::bossDanteAiRet]

        dante_m19:
        mov dword ptr [rax+0x7C], 0x14
        jmp originalcode

        ret_je:
        jmp qword ptr [EnemySwapper::bossDanteAiJne]
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
  isDanteM20 = cfg.get<bool>("EnemySwapper.isDanteM20").value_or(false);
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
  cfg.set<bool>("EnemySwapper.isDanteM20", isDanteM20);
  for (int i = 0; i < EnemySwapper::emNames.size(); i++) {
    cfg.set<uint32_t>(std::string(EnemySwapper::emNames[i]) + "_swapTo", selectedToSwap[i]);
  }
}

void EnemySwapper::on_frame() {}

void EnemySwapper::restore_default_settings() {
  for (int i = 0; i < EnemySwapper::enemyListCount; i++)
    set_swapper_setting(i, i);
  EnemySwapper::curMinIndx = minIndx;
  EnemySwapper::curMaxIndx = maxIndx;
  seed       = -1;
}

void set_Dante_ai() {
    ImGui::TextWrapped("Use this option if you want to swap some enemies to boss Dante. Can't be changed during gameplay.");
    ImGui::Checkbox("Enable boss Dante AI fix/Enable boss Vergil mission AI", &EnemySwapper::isBossDanteAiEnabled);
    if (EnemySwapper::isBossDanteAiEnabled) {
        ImGui::TextWrapped("Select this to use boss Dante/Vergil M20 AI type. By default mod using M19 AI type.\n"
            "Killing boss Vergil with m19 AI on missions/BP will cause a soft lock.");
        ImGui::Checkbox("Use boss Dante/Vergil M20 AI", &EnemySwapper::isDanteM20);
    }
}

void EnemySwapper::on_draw_ui() {
  if (ImGui::CollapsingHeader("Current Issues")){
      ImGui::TextWrapped("Killing enemies swapped with a boss in mission can cause BGM issues.\n"
          "Shadow and Griffon can't be killed.\n"
          "Only 1 type of Dante's/Vergil's AI can be loaded for mission/BP floor.\n"
          "Dante AI fix doesn't work on most of BP stages.\n"
          "Bp stages will not end with Dante boss.\n"
          "Some of battle arenas may be softlocked with boss Dante.\n"
          "Game may softlock if you skip swapped Dante boss fight with AI fix.\n"
          "Enemy Dante doesn't dealloc memory after his death. So swap all enemies to Dante on missions can crash the game. Also Dante's bodies doesn't disappears.\n"
          "Nightmare will disappear and spawn as meteor 228 billion meters above your head. You will need to wait a while for him to fall.\n"
          "Swapping the Qliphot root boss can softlock mission 1.\n"
          "Swapping Dante can softlock mission 20.\n"
          "Wrong swap on some BP stages sometimes.\n"
          "Some BP stages softlock when swapping enemies to qliphod tentacles.");
      ImGui::Spacing();
  }

  set_Dante_ai();
  ImGui::Separator();

  /*ImGui::TextWrapped("nowFlow: %d", nowFlow);
  ImGui::Spacing();
  ImGui::TextWrapped("prevFlow: %d", prevFlow);
  ImGui::Spacing();*/

  ImGui::Separator();
  ImGui::Checkbox("Increase spawn XYZ coords", &isCustomSpawnPos);
  ImGui::TextWrapped("Fixes some enemies spawning under the floor. Note that this will affect for all spawns and can change spawn animations.\n"
      "This option can be changed during the mission.");
  if (isCustomSpawnPos) {
    ImGui::TextWrapped("Z offset");
    ImGui::SliderFloat("##spawnPosZOffsetSlider", &spawnPosZOffset, 0.0f, 6.0f, "%.1f");
    ImGui::TextWrapped("Changing x,y coords can fix spawn enemy behind invisible walls (for example swap Urizen 1, Nidhogg or Qliphot roots boss to some another enemies).");
    ImGui::TextWrapped("X offset");
    ImGui::SliderFloat("##spawnPosXOffsetSlider", &spawnPosXOffset, -12.0f, 12.0f, "%.1f");
    ImGui::TextWrapped("Y offset");
    ImGui::SliderFloat("##spawnPosYOffsetSlider", &spawnPosYOffset, -12.0f, 12.0f, "%.1f");
  }

  ImGui::Separator();
  /*ImGui::TextWrapped("Vector size is: %d", setDataAddrs.size());
  ImGui::Spacing();
  ImGui::TextWrapped("ReswapCount: %d", reswapCount);*/

  //ImGui::Separator();

  ImGui::Checkbox("Swap all enemies to:", &isSwapAll);
  if (isSwapAll) {
    ImGui::Combo("", (int*)&selectedForAllSwap, emNames.data(), emNames.size(),20);
    swapForAll.set_swap_id(selectedForAllSwap);
  } 
  else {
    ImGui::Separator();
    if (ImGui::Button("Restore default swapper settings"))
      restore_default_settings();

    ImGui::Separator();
    /*if (ImGui::Button("Apply")) {
      for (int i = 0; i < emNames.size(); i++) {
        set_swapper_setting(i, selectedToSwap[i]);
      }
    }*/
    for (int i = 0; i < emNames.size(); i++) {
      ImGui::TextWrapped(emNames[i]);
      uniqComboStr = std::to_string(i) + "##SwapTo";
      ImGui::Combo(uniqComboStr.c_str(), (int*)&selectedToSwap[i], emNames.data(), emNames.size(), 20);
      //set_swapper_setting(i, selectedToSwap[i]);
      swapSettings[i].set_current_id(i);
      swapSettings[i].set_swap_id(selectedToSwap[i]);
      ImGui::Separator();
    }

    ImGui::Checkbox("Custom random settings", &isCustomRandomSettings);
    if (isCustomRandomSettings) {
      ImGui::Checkbox("Use custom seed", &isCustomSeed);
      if (isCustomSeed) {
        ImGui::Text("Seed:");
        if (ImGui::InputInt("##seedInput", &seed))
          seed_rnd_gen(seed);
      } else
        seed = -1;
      ImGui::TextWrapped("Min enemy index:");
      ImGui::SliderInt("##minIndxSlider", &curMinIndx, 0, 39);
      ImGui::TextWrapped("Max enemy index:");
      ImGui::SliderInt("##maxIndxSlider", &curMaxIndx, 1, 40);
      if (curMinIndx >= curMaxIndx)
        curMinIndx = curMaxIndx - 1;
    } else {
      curMinIndx = minIndx;
      curMaxIndx = maxIndx;
    }
    if (ImGui::Button("Random regular enemies", ImVec2(165, 25))) {
      random_em_swap(curMinIndx, curMaxIndx);
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

void EnemySwapper::on_draw_debug_ui() {
  //ImGui::TextWrapped("Vector size is: %d", setDataAddrs.size());
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
  ischecked = &cheaton;
  onpage         = balance;
  full_name_string   = "Enemy Swapper (Beta) (+)";
  author_string      = "VPZadov";
  description_string = "Swap enemy spawns. Effects normal spawns & hell judecca summons.";

  auto initAddr1 = utility::scan(base, "8B 71 10 48 85 C0 0F 84 43");// "DevilMayCry5.exe"+FE568B //For BP custom swap
  if (!initAddr1) {
    return "Unanable to find EnemySwapper pattern.";
  }

  auto initAddr2 = utility::scan(base, "44 8B 41 10 48 8B D7 48 8B CB E8 F8"); // DevilMayCry5.exe+FE57A9 //1st op
  if (!initAddr2) {
    return "Unanable to find EnemySwapper pattern.";
  }

  //auto tempcustomSpawnAddr = g_framework->get_module().as<uintptr_t>() + 0x11C58BC;

  /*auto initAddr4 = g_framework->get_module().as<uintptr_t>() + 0x259BBC9;
  setEnemyData4Jmp = g_framework->get_module().as<uintptr_t>() + 0x259B5D4;*/

   auto initAddr5 = g_framework->get_module().as<uintptr_t>() + 0xCA17A4;
  

  auto customSpawnAddr = utility::scan(base, "F3 41 0F11 4D 34 48 8B 4B 18 48 85 C9 75 10"); // DevilMayCry5.exe+11C58BC
  if (!customSpawnAddr) {
    return "Unanable to find customSpawnAddr pattern.";
  }

  auto initAddr3 = utility::scan(base, "41 8B 4E 10 48 85 C0 74"); // DevilMayCry5.exe+F34255
  if (!customSpawnAddr) {
    return "Unanable to find EnemySwapper.initAddr2 pattern.";
  }

  auto initAddr6 = utility::scan(base, "45 8B 76 10 4C 89 7C 24 40"); // DevilMayCry5.exe+BD4691
  if (!initAddr6) {
    return "Unanable to find EnemySwapper.initAddr6 pattern.";
  }

  /*auto initAddr7 = utility::scan(base, "45 8B 76 10 4C 89 7C 24 40"); // 
  if (!initAddr6) {
    return "Unanable to find EnemySwapper.initAddr7 pattern.";
  }*/

  auto nowFlowAddr = utility::scan(base, "48 8B DA 48 8B F9 83 F8 1A"); // 
  if (!nowFlowAddr) {
    return "Unanable to find EnemySwapper.nowFlowAddr pattern.";
  }

  auto gameModeAddr = utility::scan(base, "39 B8 90 00 00 00 48"); //
  if (!nowFlowAddr) {
    return "Unanable to find EnemySwapper.gameMode pattern.";
  }

  auto m19CheckAddr = utility::scan(base, "83 78 7C 13 75 11"); //DevilMayCry5.exe+1D47B50
  if (!m19CheckAddr) {
    return "Unanable to find EnemySwapper.m19CheckAddr pattern.";
  }

  //uintptr_t swapIdAddr = g_framework->get_module().as<uintptr_t>() + 0xF34F6A;

  const uintptr_t spawnAddrOffset = 0xA;

  posSpawnTestJne = customSpawnAddr.value() + spawnAddrOffset + 0x15;
  bossDanteAiJne  = m19CheckAddr.value() + 0x17;

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

  if (!install_hook_absolute(nowFlowAddr.value(), m_now_flow_hook, &now_flow_detour, &nowFlowRet, 0x6)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.nowFlowAddr"; 
  }

  /*if (!install_hook_absolute(swapIdAddr, m_enemy_swapper_hook7, &swap_id_detour, &swapIdRet, 0x7)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.swapIdAddr"; 
  }*/

  if (!install_hook_absolute(gameModeAddr.value(), m_gamemode_hook, &gamemode_detour, &gameModeRet, 0x6)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.gameModeAddr"; 
  }

  if (!install_hook_absolute(m19CheckAddr.value(), m_m19check_hook, &load_Dante_ai_detour, &bossDanteAiRet, 0x6)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySwapper.gameModeAddr"; 
  }

  seed_rnd_gen(-1);
  EnemySwapper::setDataAddrs.reserve(reservedForReswap);
  //setDataAddrs = new std::vector<uintptr_t>();
    return Mod::on_initialize();
}