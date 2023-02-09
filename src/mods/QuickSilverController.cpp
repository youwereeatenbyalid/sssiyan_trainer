#include "QuickSilverController.hpp"

REManagedObject* QuickSilverCtrl::QuickSilverControllerBase::create_slow_time_shell(gf::Vec3 pos, gf::Quaternion rot, uintptr_t owner)
{
	set_owner(owner);
	return create_slow_time_shell(pos, rot);
}

REManagedObject* QuickSilverCtrl::QuickSilverControllerBase::create_slow_time_shell(gf::Vec3 pos, gf::Quaternion rot)
{
	auto res = (uintptr_t)_createShell((uintptr_t)_shellPfb, pos, rot, _owner, 0, 0);
	if (res != 0)
		_shellCurLifeTime = (volatile float*)(res + 0x338);
	_curQuickSilverShell = (REManagedObject*)res;
	return _curQuickSilverShell;
}

QuickSilverCtrl::QuickSilverControllerBase::QuickSilverControllerBase(const wchar_t* pfbPath)
{
	_pathStr = std::make_unique<gf::SysString>(pfbPath);
	_shellPfb = PfbFactory::PrefabFactory::create_prefab(_pathStr.get());
	_shellPfbBackup = _shellPfb;
	if (_gameObjDestroyMethod == nullptr)
		_gameObjDestroyMethod = sdk::find_method_definition("via.GameObject", "destroy(via.GameObject)");
}

QuickSilverCtrl::QuickSilverControllerBase::~QuickSilverControllerBase()
{
	if (_shellPfb != nullptr)
		//utility::re_managed_object::release(_shellPfb);
		PfbFactory::PrefabFactory::release(_shellPfb);
	_shellPfb = _shellPfbBackup = nullptr;
	_curQuickSilverShell = nullptr;
}

void QuickSilverCtrl::QuickSilverControllerBase::destroy_game_obj(REManagedObject* shell)
{
	if (_gameObjDestroyMethod == nullptr || shell == nullptr)
		return;
	auto gameObj = *(uintptr_t*)((uintptr_t)shell + 0x10);
	_gameObjDestroyMethod->call(gameObj, gameObj);
}

void QuickSilverCtrl::QuickSilverControllerBase::set_shell_pfb(REManagedObject* shell)
{
	if (shell == _shellPfb)
		return;
	_shellPfbBackup = _shellPfb;
	_shellPfb = shell;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------//

void QuickSilverCtrl::QuickSilverSlowWorldController::setup_slow_shell(REManagedObject* shell)
{
	if (shell == nullptr)
		return;
	*_shellLifeTime = 0;
	auto uShell = (uintptr_t)shell;
	auto cachedQuickSilverCntrl = *(uintptr_t*)(uShell + 0x448);
	if (cachedQuickSilverCntrl == 0)
		return;
	if (_slowWorldType == SlowWorldType::StopOnSlowPfb)
	{
		*(int*)(cachedQuickSilverCntrl + 0x60) = 2;
		*(int*)(uShell + 0x440) = 6;
	}
	*(float*)(uShell + 0x334) = *(float*)(uShell + 0x338) = 0; //timer & lifeTime
}

QuickSilverCtrl::QuickSilverSlowWorldController::~QuickSilverSlowWorldController()
{
	PlayerTracker::after_pl0000_quicksilver_slow_world_action_start_unsub(_slowWorldActionEventHandler);
	PlayerTracker::after_pl0000_quicksilver_stop_world_action_start_unsub(_stopWorldActionEventHandler);
	stop_quicksilver();
	if (_worldStopShellPfb != nullptr)
		//utility::re_managed_object::release(_worldStopShellPfb);
		PfbFactory::PrefabFactory::release(_worldStopShellPfb);
	_worldStopShellPfb = nullptr;
	_curFsmAction = nullptr;
}

void QuickSilverCtrl::QuickSilverSlowWorldController::after_pl0000_slow_world_action_start(uintptr_t threadCtxt, uintptr_t fsmSlowWorld, uintptr_t behaciorTreeArg)
{
	if (!_isStarted || (uintptr_t)_curQuickSilverShell != *(uintptr_t*)(fsmSlowWorld + 0x20) || *(uintptr_t*)(fsmSlowWorld + 0x38) != get_owner())
		return;
	_curFsmAction = (REManagedObject*)fsmSlowWorld;
	_shellLifeTime = (volatile float*)(fsmSlowWorld + 0x48);
	auto shell = *(uintptr_t*)(fsmSlowWorld + 0x20);
	setup_slow_shell((REManagedObject*)shell);
	_updateQuickSilverCoroutine.start(this);
	PlayerTracker::after_pl0000_quicksilver_slow_world_action_start_unsub(_slowWorldActionEventHandler);
}

void QuickSilverCtrl::QuickSilverSlowWorldController::after_pl0000_stop_world_action_start(uintptr_t threadCtxt, uintptr_t fsmStopWorld, uintptr_t behaciorTreeArg)
{
	if (!_isStarted || (uintptr_t)_curQuickSilverShell != *(uintptr_t*)(fsmStopWorld + 0x20) || *(uintptr_t*)(fsmStopWorld + 0x38) != get_owner())
		return;
	_curFsmAction = (REManagedObject*)fsmStopWorld;
	_shellLifeTime = (volatile float*)(fsmStopWorld + 0x40);
	auto shell = *(uintptr_t*)(fsmStopWorld + 0x20);
	setup_slow_shell((REManagedObject*)shell);
	_updateQuickSilverCoroutine.start(this);
	PlayerTracker::after_pl0000_quicksilver_stop_world_action_start_unsub(_stopWorldActionEventHandler);
}

void QuickSilverCtrl::QuickSilverSlowWorldController::update_quicksilver_world_behavior()
{
	if (GameplayStateTracker::isCutscene)
	{
		stop_quicksilver();
	}
}

void QuickSilverCtrl::QuickSilverSlowWorldController::update_quicksilver_dt_consume()
{
	if (GameplayStateTracker::isCutscene || _dtGauge == nullptr)
	{
		_updateQuickSilverDTCoroutine.stop();
		_curQuickSilverShell = _curFsmAction = nullptr;
	}
	if (*_dtGauge > _dtConsumption)
		*_dtGauge -= _dtConsumption;
	else
	{
		*_dtGauge = 0;
		stop_quicksilver();
	}
}

void QuickSilverCtrl::QuickSilverSlowWorldController::stop_quicksilver()
{
	if (!_isStarted)
		return;
	_updateQuickSilverCoroutine.stop();
	_updateQuickSilverDTCoroutine.stop();
	_isStarted = false;
	*_shellLifeTime = 1.0f;
	_curQuickSilverShell = _curFsmAction = nullptr;
	_shellLifeTime = _shellCurLifeTime = nullptr;
}

REManagedObject* QuickSilverCtrl::QuickSilverSlowWorldController::create_slow_time_shell(gf::Vec3 pos, gf::Quaternion rot)
{
	if (_isStarted || (_isDtConsume && *_dtGauge < _dtMinLimit))
	{
		return nullptr;
	}
	if (_slowWorldType == SlowWorldType::StopPfb)
		set_shell_pfb(_worldStopShellPfb);
	else
		set_shell_pfb(get_shell_pfb_backup());
	QuickSilverControllerBase::create_slow_time_shell(pos, rot);
	if (_curQuickSilverShell != nullptr)
	{
		_isStarted = true;
		if (_isDtConsume)
			_updateQuickSilverDTCoroutine.start(this);
	}
	if(_slowWorldType != SlowWorldType::StopPfb)
		PlayerTracker::after_pl0000_quicksilver_slow_world_action_start_sub(_slowWorldActionEventHandler);
	else
		PlayerTracker::after_pl0000_quicksilver_stop_world_action_start_sub(_stopWorldActionEventHandler);
	return _curQuickSilverShell;
}