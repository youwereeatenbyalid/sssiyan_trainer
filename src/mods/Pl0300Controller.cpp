#include "Pl0300Controller.hpp"
#include "Pl0300ControllerManager.hpp"

void PlCntr::Pl0300Cntr::Pl0300Controller::destroy_game_obj()
{
	destroy_all_related_shells();
	if (_isDoppelDestroyRequested)
		return;
	if (get_pl() != 0 && !is_doppel())
	{
		if (is_in_players_list())
		{
			sdk::call_object_func_easy<void*>(sdk::get_managed_singleton<REManagedObject>("app.PlayerManager"), "removePlayer(app.Player, System.Boolean)", get_pl(), false);
		}
		if (auto dp = _doppel.lock(); dp != nullptr)
		{
			destroy_doppel();
		}
		PlCntr::PlController::destroy_game_obj();
	}
}

void PlCntr::Pl0300Cntr::Pl0300Controller::generate_doppel(bool isFirst, float hp, float attackRate)
{
	check_doppel_ref_correct();
	if (_pl0300GenDoppelMethod == nullptr || get_doppel() != 0)
		return;
	auto emParam = *(uintptr_t*)(get_pl() + 0x1768);
	if (emParam != 0)
	{
		auto doppelParam = *(uintptr_t*)(emParam + 0x128);
		if (doppelParam != 0)
		{
			*(float*)(doppelParam + 0x10) = hp;
			*(float*)(doppelParam + 0x14) = attackRate;
			*(float*)(doppelParam + 0x20) = *(float*)(doppelParam + 0x24) = *(float*)(doppelParam + 0x50) = 0;//no interpolate shit for help owner
			*(bool*)(doppelParam + 0x2D) = *(bool*)(doppelParam + 0x2E) = true; //Parry attacks and shells, useless for m21AI, except vs Boss Dante :/
			*(bool*)(doppelParam + 0x4C) = false;//No boss camera on help owner;
			*(float*)(doppelParam + 0x30) = 0;//SecItervalDoppel;
		}
	}
	auto groupTmp = get_char_group();
	change_character_group(PlCntr::CharGroup::Enemy);//can't generate doppel if group is pl;
	_pl0300GenDoppelMethod->call(sdk::get_thread_context(), get_pl(), isFirst);
	if(_pl0300Manager != nullptr)
		_doppel = _pl0300Manager->register_doppelganger(this);
	change_character_group(groupTmp);
}

PlCntr::Pl0300Cntr::Pl0300Controller::Pl0300Controller(uintptr_t pl0300, PlCntr::Pl0300Cntr::Pl0300Type type, bool isKeepingOriginalPadInput) : PlCntr::PlController(pl0300)
{
	_isKeepingOriginalPadInput = isKeepingOriginalPadInput;
	if (pl0300 == 0)
		throw std::exception("pl0300 is null");
	_pl0300Type = type;

	if(_isStaticInitRequested)
	{
		if (_pl0300SetDtMethod == nullptr)
			_pl0300SetDtMethod = sdk::find_method_definition("app.player.pl0300.PlayerVergil", "setDevilTrigger(app.player.PlayerBase.DevilTrigger, System.Boolean)");

		if (_pl0300GenDoppelMethod == nullptr)
			_pl0300GenDoppelMethod = sdk::find_method_definition("app.player.pl0300.PlayerVergil", "generateDoppleGanger(System.Boolean)");

		if (_pl0300DestroyDoppelMethod == nullptr)
			_pl0300DestroyDoppelMethod = sdk::find_method_definition("app.player.pl0300.PlayerVergil", "destroyDoppleGanger()");

		if (_pl0300DoStartMethod == nullptr)
			_pl0300DoStartMethod = sdk::find_method_definition("app.player.pl0300.PlayerVergil", "doStart()");

		if (_pl0300RequestDestroyDoppelMethod == nullptr)
			_pl0300RequestDestroyDoppelMethod = sdk::find_method_definition("app.player.pl0300.PlayerVergil", "requestDestroyDoppelGanger()");

		if (_baseBehaviorStartMethod == nullptr)
			_baseBehaviorStartMethod = sdk::find_method_definition("app.BaseBehavior", "start()");

		if (_pl0300startAirRaidMethod == nullptr)
			_pl0300startAirRaidMethod = sdk::find_method_definition("app.player.pl0300.PlayerVergil", "startAirRaid()");

		if (_pl0300SetActionMethod == nullptr)
			_pl0300SetActionMethod = sdk::find_method_definition("app.player.pl0300.PlayerVergil", "setAction(System.String, System.UInt32, System.Single, System.Single, "
				"via.motion.InterpolationMode, via.motion.InterpolationCurve, System.Boolean, System.Boolean, System.Boolean, app.GameModel.ActionPriority)");

		if (_pl0300SetActionFromThinkMethod == nullptr)
			_pl0300SetActionFromThinkMethod = sdk::find_method_definition("app.player.pl0300.PlayerVergil", "setActionFromThink(System.String, System.UInt32)");

		if (_pl0300UpdateEmTeleportMethod == nullptr)
			_pl0300UpdateEmTeleportMethod = sdk::find_method_definition("app.player.pl0300.PlayerVergil", "updateEnemyTeleport(System.Boolean)");

		if (_pl0300Manager == nullptr)
			_pl0300Manager = static_cast<Pl0300ControllerManager*>(g_framework->get_mods()->get_mod("Pl0300ControllerManager"));

		_isStaticInitRequested = false;
	}
}

void PlCntr::Pl0300Cntr::Pl0300Controller::destroy_doppel()
{
	check_doppel_ref_correct();
	if (*(uintptr_t*)(get_pl() + 0x1C10) == 0)
		return;
	auto groupTmp = get_char_group();
	change_character_group(PlCntr::CharGroup::Enemy);
	/*if (_pl0300RequestDestroyDoppelMethod != nullptr)
		_pl0300RequestDestroyDoppelMethod->call(sdk::get_thread_context(), _doppel.lock()->get_pl());*/
	if (_pl0300DestroyDoppelMethod != nullptr)
		_pl0300DestroyDoppelMethod->call(sdk::get_thread_context(), get_pl());
	_doppel.lock()->_isDoppelDestroyRequested = true;
	_pl0300Manager->remove_doppelganger(_doppel.lock().get());
	change_character_group(groupTmp);
}

inline void PlCntr::Pl0300Cntr::Pl0300Controller::check_doppel_ref_correct()
{
	if (*(uintptr_t*)(get_pl() + 0x1C10) == 0 && _doppel.lock() != nullptr)
		_pl0300Manager->remove_doppelganger(_doppel.lock().get());
}