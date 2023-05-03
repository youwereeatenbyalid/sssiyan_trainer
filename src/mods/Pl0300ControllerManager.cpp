#include "Pl0300ControllerManager.hpp"
#include "BossVergilSettings.hpp"
#include "EnemyFixes.hpp"

void PlCntr::Pl0300Cntr::Pl0300ControllerManager::is_pl0300_controller_asm(uintptr_t threadCtxt, uintptr_t emManager, uintptr_t pl0300)
{
    for (const auto i : _mod->_pl0300List)
    {
        if (i->get_pl() == pl0300 && (i->get_pl0300_type() == Pl0300Type::PlHelper || i->get_pl0300_type() == Pl0300Type::Em6000Friendly))
            return;
    }
    ((_EmManager_RequestAddObj)_mod->requestAddEmFuncAddr)(threadCtxt, emManager, pl0300);
}

naked void PlCntr::Pl0300Cntr::Pl0300ControllerManager::em6000_request_add_em_detour()
{
    __asm
    {
        push rax
        push rcx
        push rdx
        push rsp
        push r8
        push r9
        push r10
        push r11
        sub rsp, 32
        call qword ptr [Pl0300ControllerManager::is_pl0300_controller_asm]
        add rsp, 32
        pop r11
        pop r10
        pop r9
        pop r8
        pop rsp
        pop rdx
        pop rcx
        pop rax
        jmp qword ptr [Pl0300ControllerManager::requestAddEmRet]
    }
}

bool PlCntr::Pl0300Cntr::Pl0300ControllerManager::check_pl0300_asm(uintptr_t pl0300)
{
    for (const auto &i : _mod->_pl0300List)//Must take shared_ptr by ref cause doppel destoy called in dctor
    {
        if (i->get_pl() == pl0300)
            return true;
    }
    return false;
}

naked void PlCntr::Pl0300Cntr::Pl0300ControllerManager::em6000_damage_check_detour()
{
    __asm
    {
        sete al
        movaps [rsp + 0x110], xmm6
        push rax
        push rcx
        push rdx
        push r8
        push r9
        push rsp
        mov rcx, rdi
        sub rsp, 32
        call qword ptr [Pl0300ControllerManager::check_pl0300_asm]
        add rsp, 32
        pop rsp
        pop r9
        pop r8
        pop rdx
        pop rcx
        cmp al, 01
        pop rax
        je in_list
        jmp qword ptr[Pl0300ControllerManager::damageCheckRet]

        in_list:
        mov al, 01
        jmp qword ptr [Pl0300ControllerManager::damageCheckRet]
    }
}

naked void PlCntr::Pl0300Cntr::Pl0300ControllerManager::pl0300_destroy_doppel_request_boss_camera_detour()
{
    __asm
    {
        cmp byte ptr [EnemyFixes::cheaton], 0
        je pl0300CntrlCheck
        cmp byte ptr [EnemyFixes::isDoppelCameraFix], 01
        je skip

        pl0300CntrlCheck:
        push rax
        push rcx
        push rdx
        push r8
        push r9
        push rsp
        mov rcx, rbx
        sub rsp, 32
        call qword ptr [Pl0300ControllerManager::check_pl0300_asm]
        add rsp, 32
        pop rsp
        pop r9
        pop r8
        pop rdx
        pop rcx
        cmp al, 00
        pop rax
        je originalcode
        skip:
        jmp qword ptr[Pl0300ControllerManager::doppelDestroyReqBossCamRet]

        originalcode:
        call qword ptr [Pl0300ControllerManager::requestBossCameraFunc]
        jmp qword ptr [Pl0300ControllerManager::doppelDestroyReqBossCamRet]
    }
}

void PlCntr::Pl0300Cntr::Pl0300ControllerManager::reset(EndLvlHooks::EndType resetType)
{
    _doppelRemoveCoroutinesList.clear();
    if (resetType == EndLvlHooks::EndType::ResetTraining)
    {
        _pl0300List.erase(std::remove_if(_pl0300List.begin(), _pl0300List.end(), [](const std::shared_ptr<Pl0300Controller> &obj)
            {
                if (obj->is_ignoring_training_reset())
                {
                    obj->destroy_all_related_shells();
                    obj->destroy_doppel();
                    obj->end_cutscene();
                    return false;
                }
                return true;
            }), _pl0300List.end());
    }
    else
        _pl0300List.clear();

    if (resetType == EndLvlHooks::CheckpointMission || resetType == EndLvlHooks::RetryMission || resetType == EndLvlHooks::ResetTraining)
        return;

    if (_pl0300Pfb != nullptr)
    {
        sdk::call_object_func_easy<void*>(_pl0300Pfb, "set_Standby(System.Boolean)", false);
        PfbFactory::PrefabFactory::release(_pl0300Pfb);
        _pl0300Pfb = nullptr;
        _pl0300PathStr = nullptr;
    }
    if (_pl0300C00Pfb != nullptr)
    {
        sdk::call_object_func_easy<void*>(_pl0300C00Pfb, "set_Standby(System.Boolean)", false);
        PfbFactory::PrefabFactory::release(_pl0300C00Pfb);
        _pl0300C00Pfb = nullptr;
        _pl0300C00PathStr = nullptr;
    }
}

void PlCntr::Pl0300Cntr::Pl0300ControllerManager::pl0300_start_func_hook(uintptr_t threadCntx, uintptr_t pl0300)
{
    _mod->_pl0300StartHook->get_original<decltype(Pl0300ControllerManager::pl0300_start_func_hook)>()(threadCntx, pl0300);
    //pl0300.Start() setting up many fields, so i need to reset it after Start() was called for controlled pl0300.
    for (const auto i : _mod->_pl0300List)
    {
        if (i->get_pl() == pl0300)
        {
            if (!i->_isStarted)
            {
                i->_isStarted = true;
                if (i->get_pl0300_type() == Pl0300Type::PlHelper && !i->is_keeping_original_pad_input())
                {
                    auto manualPl = *(uintptr_t*)((uintptr_t)(i->get_pl_manager()) + 0x60);
                    if (manualPl != 0)
                        *(uintptr_t*)(i->get_pl() + 0xEF0) = *(uintptr_t*)(manualPl + 0xEF0);
                }

                if (i->_isHitCtrlDataSetRequested)
                {
                    i->set_hitcontroller_settings(i->_hcLastSettings);
                    i->_isHitCtrlDataSetRequested = false;
                }

                if (i->_isJcNumSetRequested)
                {
                    i->set_jcut_num(i->_requestedJcNum);
                    i->_isJcNumSetRequested = false;
                }

                if (i->_isEmStepSetRequested)
                {
                    i->set_em_step_enabled(i->_emStepRequest);
                    i->_isEmStepSetRequested = false;
                }

                if (i->_isSetDtRequested)
                {
                    i->set_dt(i->_requestedDT);
                    i->_isSetDtRequested = false;
                }

                if (i->_isSetTeleportTimingParamsRequested)
                {
                    i->set_teleport_timing_params(i->_teleportTimingParamsRequested);
                    i->_isSetTeleportTimingParamsRequested = false;
                }

                if (i->_isSetIsNoDieRequested)
                {
                    i->set_is_no_die(i->_setIsNoDieRequestedVal);
                    i->_isSetIsNoDieRequested = false;
                }

                if (i->_isAddToPlListRequested)
                {
                    i->pl_manager_request_add();
                    i->_isAddToPlListRequested = false;
                }
            }
            return;
        }
    }
    BossVergilSettings::after_pl0300_start(threadCntx, pl0300);
    if (EnemyFixes::cheaton && EnemyFixes::isDoppelCameraFix)
    {
        auto emParam = *(uintptr_t*)(pl0300 + 0x1768);
        if (emParam != 0)
        {
            auto doppelParam = *(uintptr_t*)(emParam + 0x128);
            if (doppelParam != 0)
            {
                *(float*)(doppelParam + 0x20) = *(float*)(doppelParam + 0x24) = *(float*)(doppelParam + 0x50) = 0;//no interpolate shit for help owner
                *(bool*)(doppelParam + 0x4C) = false;//No boss camera on help owner;
            }
        }
    }
}

int PlCntr::Pl0300Cntr::Pl0300ControllerManager::pl0300_get_mission_n_hook(uintptr_t threadCntx, uintptr_t pl0300)
{
    if (EnemyFixes::cheaton && EnemyFixes::isFriendlyVergilAI)
    {
        return 0x3E;
    }
    for (const auto pl0300Ctrl : _mod->_pl0300List)
    {
        if (pl0300 != pl0300Ctrl->get_pl())
            continue;
        if (auto plType = pl0300Ctrl->get_pl0300_type(); plType == Pl0300Type::PlHelper || plType == Pl0300Type::Em6000Friendly)
            return 0x3E;
    }
    return _mod->_pl0300GetMissionHook->get_original<decltype(pl0300_get_mission_n_hook)>()(threadCntx, pl0300);
}

bool PlCntr::Pl0300Cntr::Pl0300ControllerManager::pl0300_check_dt_cancel_hook(uintptr_t threadCtxt, uintptr_t pl0300)
{
    auto charGroup = *(CharGroup*)(pl0300 + 0x108);
    for (const auto i : _mod->_pl0300List)
    {
        if (i->get_pl() == pl0300 && i->get_pl0300_type() == Pl0300Type::PlHelper && charGroup == CharGroup::Enemy)
        {
            i->change_character_group(CharGroup::Player);
            break;
        }
    }
    bool res = _mod->_pl0300CheckDtCancelHook->get_original<decltype(pl0300_check_dt_cancel_hook)>()(threadCtxt, pl0300);
    *(CharGroup*)(pl0300 + 0x108) = charGroup;
    return res;
}

bool PlCntr::Pl0300Cntr::Pl0300ControllerManager::check_em_think_off_hook(uintptr_t threadCtxt, uintptr_t character)
{
    bool res = _mod->_checkEmThinkOffHook->get_original<decltype(check_em_think_off_hook)>()(threadCtxt, character);
    for (const auto i : _mod->_pl0300List)
    {
        if (i->get_pl() == character)
        {
            if (i->is_doppel() && i->get_pl0300_type() == Pl0300Type::Em6000Friendly)
                return i->is_doppel_destroy_requested();
            else if (i->get_pl0300_type() == Pl0300Type::PlHelper)
                return true;
        }
    }
    
    return res;
}

void PlCntr::Pl0300Cntr::Pl0300ControllerManager::pl0300_update_lock_on_hook(uintptr_t threadCtxt, uintptr_t pl0300)
{
    _mod->_pl0300UpdateLockOnHook->get_original<decltype(pl0300_update_lock_on_hook)>()(threadCtxt, pl0300);
    for (auto i : _mod->_pl0300List)
    {
        if (i->get_pl() == pl0300 && !i->is_doppel())
        {
            _mod->_afterPl0300UpdateLockOnEvent.invoke(threadCtxt, i);
            return;
        }
    }
}

void PlCntr::Pl0300Cntr::Pl0300ControllerManager::pl0300_update_lock_on_target_on_enemy_hook(uintptr_t threadCtxt, uintptr_t pl0300)
{
    for (auto i : _mod->_pl0300List)
    {
        if (i->get_pl() == pl0300 && !i->is_doppel())
        {
            bool skipCall = false;
            _mod->_pl0300UpdateLockOnTargetEvent.invoke(threadCtxt, i, &skipCall);
            if (!skipCall)
                _mod->_pl0300UpdateLockOnTargetOnEnemyHook->get_original<decltype(pl0300_update_lock_on_target_on_enemy_hook)>()(threadCtxt, pl0300);
            return;
        }
    }
    _mod->_pl0300UpdateLockOnTargetOnEnemyHook->get_original<decltype(pl0300_update_lock_on_target_on_enemy_hook)>()(threadCtxt, pl0300);
}

void PlCntr::Pl0300Cntr::Pl0300ControllerManager::pl0300_teleport_calc_dest_hook(uintptr_t threadCtxt, uintptr_t fsmPl0300Teleport)
{
    auto pl0300 = *(uintptr_t*)(fsmPl0300Teleport + 0x60);
    for (auto i : _mod->_pl0300List)
    {
        if (i->get_pl() == pl0300 && !i->is_doppel())
        {
            bool skipCall = false;
            _mod->_pl0300OnTeleportCalcDestinationEvent.invoke(threadCtxt, fsmPl0300Teleport, i, &skipCall);
            if (!skipCall)
                _mod->_pl0300TeleportCalcDestHook->get_original<decltype(pl0300_teleport_calc_dest_hook)>()(threadCtxt, fsmPl0300Teleport);
            return;
        }
    }
    _mod->_pl0300TeleportCalcDestHook->get_original<decltype(pl0300_teleport_calc_dest_hook)>()(threadCtxt, fsmPl0300Teleport);
}

PlCntr::Pl0300Cntr::Pl0300ControllerManager::Pl0300ControllerManager()
{
    _mod = this;
}

bool PlCntr::Pl0300Cntr::Pl0300ControllerManager::destroy_game_obj(const std::weak_ptr<Pl0300Controller> &obj)
{
    auto elevated = obj.lock();
    if (elevated == nullptr)
        return false;
    std::lock_guard<std::recursive_mutex> lck(_pl0300ListChangeMtx);
    if (elevated->is_doppel())
    {
        if (auto elevatedOwner = elevated->get_owner_ctrl().lock(); elevatedOwner != nullptr)
            elevatedOwner->destroy_doppel();
        return true;
    }
    for (int i = 0; i < _pl0300List.size(); i++)
    {
        if (_pl0300List[i] == elevated)
        {
            _pl0300List.erase(_pl0300List.begin() + i);
            return true;
        }
    }
    return false;
}
/// <summary>
/// Create a prefab for instatiating a boss vergil instance.
/// </summary>
/// <param name="exCostume">if the prefab should be the EX variant</param>
void PlCntr::Pl0300Cntr::Pl0300ControllerManager::load_pfb(bool exCostume)
{
    if (exCostume)
    {
        if (_pl0300C00Pfb != nullptr && _pl0300C00Pfb->referenceCount > 0)
            return;
        _pl0300C00PathStr = std::make_unique<gf::SysString>(_pl0300C00Path);
        _pl0300C00Pfb = PfbFactory::PrefabFactory::create_prefab(_pl0300C00PathStr.get(), (uintptr_t)sdk::get_thread_context());
    }
    else
    {
        if (_pl0300Pfb != nullptr && _pl0300Pfb->referenceCount > 0)
            return;
        _pl0300PathStr = std::make_unique<gf::SysString>(_pl0300Path);
        _pl0300Pfb = PfbFactory::PrefabFactory::create_prefab(_pl0300PathStr.get(), (uintptr_t)sdk::get_thread_context());
    }
}

std::weak_ptr<PlCntr::Pl0300Cntr::Pl0300Controller> PlCntr::Pl0300Cntr::Pl0300ControllerManager::create_em6000(Pl0300Type controllerType, gf::Vec3 pos, bool isKeepingOrigPadInput, bool exCostume)
{
    load_pfb(exCostume);
    auto curPfb = exCostume ? _pl0300C00Pfb : _pl0300Pfb;
    if(!sdk::call_object_func_easy<bool>(curPfb, "get_Valid()"))
        return std::weak_ptr<Pl0300Controller>();
    gf::PrefabInstantiate pfb;
    auto pl0300GameObj = pfb((uintptr_t)curPfb, pos);
    if (pl0300GameObj == 0)
        return std::weak_ptr<Pl0300Controller>();
    auto bossType = sdk::find_type_definition("app.player.pl0300.PlayerVergil")->get_runtime_type();
    auto pl0300 = (uintptr_t)sdk::call_object_func_easy<REManagedObject*>((REManagedObject*)pl0300GameObj, "getComponent(System.Type)", bossType);//Get pl0300 script from GameObj
    try
    {
        std::lock_guard<std::recursive_mutex> lck(_pl0300ListChangeMtx);
        _pl0300List.emplace_back(std::shared_ptr<Pl0300Controller>(new Pl0300Controller(pl0300, controllerType, exCostume, isKeepingOrigPadInput)));//I cant use make_shared for friend class ctor :(
    }
    catch (const std::exception& e)
    {
        return std::weak_ptr<Pl0300Controller>();
    }
    if (controllerType == Pl0300Type::PlHelper)
    {
        *(bool*)(_pl0300List[_pl0300List.size() - 1]->get_pl() + 0x4C6) = true;//isControl
        *(bool*)(_pl0300List[_pl0300List.size() - 1]->get_pl() + 0x17E0) = false;//IsEnemy
        auto manualPl = *(uintptr_t*)((uintptr_t)_pl0300List[_pl0300List.size() - 1]->get_pl_manager() + 0x60);
        if(manualPl != 0)
            *(uintptr_t*)(_pl0300List[_pl0300List.size() - 1]->get_pl() + 0xEF0) = *(uintptr_t*)(manualPl + 0xEF0);
    }
    return std::weak_ptr<Pl0300Controller>(_pl0300List[_pl0300List.size() - 1]);
}

std::weak_ptr<PlCntr::Pl0300Cntr::Pl0300Controller> PlCntr::Pl0300Cntr::Pl0300ControllerManager::get_pl0300_controller(uintptr_t pl0300)
{
    auto iter = std::find_if(_pl0300List.begin(), _pl0300List.end(), [=](const auto& obj)
        {
            return obj->get_pl() == pl0300;
        });
    if (iter != _pl0300List.end())
        return std::weak_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>(*iter);
    else 
        return std::weak_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>();
}

std::weak_ptr<PlCntr::Pl0300Cntr::Pl0300Controller> PlCntr::Pl0300Cntr::Pl0300ControllerManager::register_doppelganger(const Pl0300Controller* controllerOwner)
{
    if (controllerOwner == nullptr || controllerOwner->get_doppel() == 0)
        return std::weak_ptr<Pl0300Controller>();
    std::unique_lock<std::recursive_mutex> lck(_pl0300ListChangeMtx);
    int indx = 0;
    for (int i = 0; i < _pl0300List.size(); i++)
    {
        if (_pl0300List[i]->get_pl() == controllerOwner->get_doppel())//Doppel already summoned
            return std::weak_ptr<Pl0300Controller>();
        if (_pl0300List[i].get() == controllerOwner)
            indx = i;
    }
    auto doppel = std::shared_ptr<Pl0300Controller>(new Pl0300Controller(controllerOwner->get_doppel(), Pl0300Type::Em6000Friendly, _pl0300List[indx]->is_ex_costume()));
    _pl0300List.push_back(doppel);
    lck.unlock();
    doppel->_owner = std::weak_ptr<Pl0300Controller>(_pl0300List[indx]);
    return std::weak_ptr<Pl0300Controller>(doppel);
}

void PlCntr::Pl0300Cntr::Pl0300ControllerManager::remove_doppel_routine(const Pl0300Controller* doppelController)
{
    if (GameplayStateTracker::isCutscene)
        return;
    std::lock_guard<std::recursive_mutex> lck(_pl0300ListChangeMtx);
    _pl0300List.erase(std::remove_if(_pl0300List.begin(), _pl0300List.end(), [&](const std::shared_ptr<Pl0300Controller>& obj)
        {
            return obj->get_pl() == doppelController->get_pl();
        }), _pl0300List.end());
    _doppelRemoveCoroutinesList.erase(std::remove_if(_doppelRemoveCoroutinesList.begin(), _doppelRemoveCoroutinesList.end(),
        [&](const std::unique_ptr<Coroutines::Coroutine<void(Pl0300ControllerManager::*)(const Pl0300Controller*), Pl0300ControllerManager*, const Pl0300Controller*>>& coroutine)
        {
            return std::get<1>(coroutine->get_action()->get_args()) == doppelController;
        }), _doppelRemoveCoroutinesList.end());
}

void PlCntr::Pl0300Cntr::Pl0300ControllerManager::remove_doppelganger(const Pl0300Controller* doppelController)
{
    if (doppelController == nullptr || !doppelController->is_doppel())
        return;
    _doppelRemoveCoroutinesList.emplace_back(std::make_unique<Coroutines::Coroutine<void(Pl0300ControllerManager::*)(const Pl0300Controller*), Pl0300ControllerManager*, const Pl0300Controller*>>
        (&Pl0300ControllerManager::remove_doppel_routine, false));
    _doppelRemoveCoroutinesList[_doppelRemoveCoroutinesList.size() - 1]->set_delay(10000.0f);
    _doppelRemoveCoroutinesList[_doppelRemoveCoroutinesList.size() - 1]->ignoring_update_on_pause(true);
    _doppelRemoveCoroutinesList[_doppelRemoveCoroutinesList.size() - 1]->start(this, doppelController);
}

void PlCntr::Pl0300Cntr::Pl0300ControllerManager::set_pos_to_all(gf::Vec3 pos, Pl0300Type type)
{
    for (const auto i : _pl0300List)
    {
        if (i->get_pl0300_type() == type)
            i->set_pos_full(pos);
    }
}

void PlCntr::Pl0300Cntr::Pl0300ControllerManager::kill_all_friendly_em6000()
{
    std::lock_guard<std::recursive_mutex> lck(_pl0300ListChangeMtx);
    _pl0300List.erase(std::remove_if(_pl0300List.begin(), _pl0300List.end(), [&](const std::shared_ptr<Pl0300Controller>& obj) {return obj->get_pl0300_type() == Pl0300Type::Em6000Friendly; }), _pl0300List.end());
}

void PlCntr::Pl0300Cntr::Pl0300ControllerManager::on_pl_pad_input_reset(uintptr_t pl, bool isAutoPad, bool* callOrig)
{
    for (const auto i : _pl0300List)
    {
        if (!i->is_keeping_original_pad_input() && i->get_pl0300_type() == Pl0300Type::PlHelper && pl == i->get_pl())
        {
            auto manualPl = *(uintptr_t*)((uintptr_t)(i->get_pl_manager()) + 0x60);
            if (manualPl != 0)
            {
                *(uintptr_t*)(pl + 0xEF0) = *(uintptr_t*)(manualPl + 0xEF0);
                *callOrig = false;
                return;
            }
        }
    }
}

std::optional<std::string> PlCntr::Pl0300Cntr::Pl0300ControllerManager::on_initialize()
{
    init_check_box_info();
    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
    m_is_enabled = &enabled;
    m_on_page = Page_None;
    m_full_name_string = "";
    m_author_string = "V.P. Zadov";
    m_description_string = "";

    //set_up_hotkey();

    auto pl300MissionNo = m_patterns_cache->find_addr(base, "48 89 5C 24 08 57 48 83 EC 20 48 8B 41 50 48 8B D9 BF FF"); // app_Em5900Base_AttackRate_AttackRateVergilMode__get_missionNo213806
    if (!pl300MissionNo)
    {
        return "Unanable to find Pl0300ControllerManager.app_Em5900Base_AttackRate_AttackRateVergilMode__get_missionNo213806 pattern.";
    }

    auto addEmFuncAddr = m_patterns_cache->find_addr(base, "B6 00 CC CC CC CC CC CC CC CC 48 89 5C 24 18");
    requestAddEmFuncAddr = addEmFuncAddr ? addEmFuncAddr.value() + 0x0A : (uintptr_t)base + 0x19DD130;

    auto requestBossCam = m_patterns_cache->find_addr(base, "48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 48 8D 68 A1 48 81 EC F0");
    //DevilMayCry5.app_PlayerCameraController__RequestBossCamera147662
    requestBossCameraFunc = requestBossCam ? requestBossCam.value() : (uintptr_t)base + 0xD018E0;

    auto requestAddEmAddr = m_patterns_cache->find_addr(base, "E8 8A A0 00 01"); // DevilMayCry5.exe+9D30A1 
    if (!pl300MissionNo)
    {
        return "Unanable to find Pl0300ControllerManager.requestAddEmAddr pattern.";
    }

    auto pl0300DoStartAddr = m_patterns_cache->find_addr(base, "40 55 56 57 48 8D AC 24 C0 E5 FF FF");//DevilMayCry5.app_player_pl0300_PlayerVergil__doStart218741 
    if (!pl0300DoStartAddr)
    {
        return "Unable to find Pl0300ControllerManager.pl0300DoStartAddr pattern.";
    }

    auto pl0300CheckDevilTriggerCancelAddr = m_patterns_cache->find_addr(base, "00 5F C3 CC CC CC CC CC CC CC 48 89 5C 24 10 57 48 83 EC 60");
    //DevilMayCry5.app_player_pl0300_PlayerVergil__checkDevilTriggerCancel218714 (-0xA)
    if (!pl0300CheckDevilTriggerCancelAddr)
    {
        return "Unable to find Pl0300ControllerManager.pl0300CheckDevilTriggerCancelAddr pattern.";
    }

    auto checkEmThinkOffAddr = m_patterns_cache->find_addr(base, "40 53 48 83 EC 20 48 8B 41 50 48 8B D9 45");//DevilMayCry5.app_character_Character__checkEnemyThinkOff167898
    if (!checkEmThinkOffAddr)
    {
        return "Unable to find Pl0300ControllerManager.checkEmThinkOffAddr pattern.";
    }

    auto pl0300UpdateLockOnAddr = m_patterns_cache->find_addr(base, "48 89 5C 24 10 57 48 83 EC 20 48 8B FA 48 8B D9 E8 7B 35");//DevilMayCry5.app_player_pl0300_PlayerVergil__updateLockOn218720
    if (!checkEmThinkOffAddr)
    {
        return "Unable to find Pl0300ControllerManager.pl0300UpdateLockOnAddr pattern.";
    }

    auto pl0300UpdateLockOnTargetAddr = m_patterns_cache->find_addr(base, "B8 01 EB 8F CC CC CC CC CC 48 89 5C 24 08");//DevilMayCry5.app_player_pl0300_PlayerVergil__updateLockOnTargetOnEnemy218668(-0x9)
    if (!pl0300UpdateLockOnTargetAddr)
    {
        return "Unable to find Pl0300ControllerManager.pl0300UpdateLockOnTargetAddr pattern.";
    }

    auto pl0300TeleportDestAddr = m_patterns_cache->find_addr(base, "48 89 5C 24 10 48 89 74 24 18 55 48 8D 6C 24 A9 48 81 EC 00");//DevilMayCry5.app_fsm2_pl0300_Teleport__calcDestination310338 
    if (!pl0300TeleportDestAddr)
    {
        return "Unable to find Pl0300ControllerManager.pl0300TeleportDestAddr pattern.";
    }

    auto pl0300CheckDamageAddr = m_patterns_cache->find_addr(base, "0F 94 C0 0F 29 B4 24 10 01 00 00");//DevilMayCry5.exe+9DAFC2
    if (!pl0300CheckDamageAddr)
    {
        return "Unable to find Pl0300ControllerManager.pl0300CheckDamageAddr pattern.";
    }

    auto pl0300DoppelDestroyReqBossCamAddr = m_patterns_cache->find_addr(base, "E8 95 26 33 00");//DevilMayCry5.exe+9CF246
    if (!pl0300DoppelDestroyReqBossCamAddr)
    {
        return "Unable to find Pl0300ControllerManager.pl0300DoppelDestroyReqBossCamAddr pattern.";
    }

    if (!install_hook_absolute(requestAddEmAddr.value(), _requestAddEmObjHook, &em6000_request_add_em_detour, &requestAddEmRet, 0x5))
    {
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize Pl0300ControllerManager.requestAddEm";
    }

    if (!install_hook_absolute(pl0300CheckDamageAddr.value(), _pl0300CheckDamageHook, &em6000_damage_check_detour, &damageCheckRet, 0xB))
    {
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize Pl0300ControllerManager.pl0300CheckDamage";
    }

    if (!install_hook_absolute(pl0300DoppelDestroyReqBossCamAddr.value(), _pl0300DestroyDoppelRequestBossCamHook, &pl0300_destroy_doppel_request_boss_camera_detour, &doppelDestroyReqBossCamRet, 0x5))
    {
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize Pl0300ControllerManager.pl0300DoppelDestroyReqBossCam";
    }

    _pl0300GetMissionHook = std::make_unique<FunctionHook>(pl300MissionNo.value(), &pl0300_get_mission_n_hook);
    _pl0300GetMissionHook->create();

    _pl0300StartHook = std::make_unique<FunctionHook>(pl0300DoStartAddr.value(), &pl0300_start_func_hook);
    _pl0300StartHook->create();

    _pl0300CheckDtCancelHook = std::make_unique<FunctionHook>(pl0300CheckDevilTriggerCancelAddr.value() + 0xA, &pl0300_check_dt_cancel_hook);
    _pl0300CheckDtCancelHook->create();

    _checkEmThinkOffHook = std::make_unique<FunctionHook>(checkEmThinkOffAddr.value(), &check_em_think_off_hook);
    _checkEmThinkOffHook->create();

    _pl0300UpdateLockOnHook = std::make_unique<FunctionHook>(pl0300UpdateLockOnAddr.value(), &pl0300_update_lock_on_hook);
    _pl0300UpdateLockOnHook->create();

    _pl0300UpdateLockOnTargetOnEnemyHook = std::make_unique<FunctionHook>(pl0300UpdateLockOnTargetAddr.value() + 0x9, &pl0300_update_lock_on_target_on_enemy_hook);
    _pl0300UpdateLockOnTargetOnEnemyHook->create();

    _pl0300TeleportCalcDestHook = std::make_unique<FunctionHook>(pl0300TeleportDestAddr.value(), &pl0300_teleport_calc_dest_hook);
    _pl0300TeleportCalcDestHook->create();

    PlayerTracker::before_reset_pad_input_sub<Pl0300ControllerManager>(std::make_shared<Events::EventHandler<Pl0300ControllerManager, uintptr_t, bool, bool*>>(this, &Pl0300ControllerManager::on_pl_pad_input_reset));

    return Mod::on_initialize();
}

void PlCntr::Pl0300Cntr::Pl0300ControllerManager::on_draw_ui()
{
}
