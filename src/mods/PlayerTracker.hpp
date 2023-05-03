#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include "mods/GameFunctions/GameFunc.hpp"
#include "events/Events.hpp"
namespace gf = GameFunctions;
class PlayerTracker : public Mod {
public:
	inline static bool cheaton{ false };

	enum class PlDanteStyleType
	{
		None = -1,
		Trickster,
		SwordMaster,
		GunSlinger,
		RoyalGuard
	};

	PlayerTracker()
	{
		_mod = this;
	}
	~PlayerTracker()
	{
		_mod = nullptr;
	}

	// mod name string for config
	std::string_view get_name() const override { return "PlayerTracker"; }
	std::string get_checkbox_name() override { return m_check_box_name; };
	std::string get_hotkey_name() override { return m_hot_key_name; };
	// called by m_mods->init() you'd want to override this
	std::optional<std::string> on_initialize() override;
	static uintptr_t player_jmp_ret;
	static uintptr_t summon_jmp_ret;
	static uintptr_t summon_jmp_je;
	static uintptr_t incombat_jmp_ret;
	static uintptr_t sin_jmp_ret;
	static uintptr_t cos_jmp_ret;
	static uintptr_t threshhold_jmp_ret;
	static uintptr_t threshhold_jmp_jb;
	static uintptr_t vergildata_jmp_ret;
	static inline uintptr_t _setTrickStyleRet = 0;
	static inline uintptr_t _setSwordStyleRet = 0;
	static inline uintptr_t _setGunStyleRet = 0;
	static inline uintptr_t _setRoyalStyleRet = 0;

	static uintptr_t playerentity;
	static uint32_t playerid;
	static uintptr_t groundedmem;
	static uint32_t isgrounded;
	static uintptr_t playertransform;
	static uint32_t playermoveid;

	static uintptr_t playerinertiax;
	static uintptr_t playerinertiay;
	static uintptr_t playerinertiaz;

	static uintptr_t neroentity;
	static uintptr_t nerotransform;

	static uintptr_t danteentity;
	static uintptr_t dantetransform;
	static uintptr_t danteweapon;

	static uintptr_t ventity;
	static uintptr_t vtransform;

	static uintptr_t shadowcontroller;
	static uintptr_t shadowentity;
	static uintptr_t shadowtransform;

	static uintptr_t griffoncontroller;
	static uintptr_t griffonentity;
	static uintptr_t griffontransform;

	static uintptr_t nightmarecontroller;
	static uintptr_t nightmareentity;
	static uintptr_t nightmaretransform;

	static uintptr_t vergilentity;
	static uintptr_t vergiltransform;

	static uintptr_t yamatomodel;
	static uintptr_t yamatocommonparameter;
	static uintptr_t forceedgemodel;
	static uintptr_t beowulfmodel;

	static uintptr_t doppelentity;
	static uintptr_t doppeltransform;
	static uintptr_t doppelweaponmodel;

	static uintptr_t playermanager;

	static uint32_t incombat;
	static bool ingameplay;
	static float sinvalue;
	static float cosvalue;
	static bool redirect;
	// Override this things if you want to store values in the config file
	// void on_config_load(const utility::Config& cfg) override;
	// void on_config_save(utility::Config& cfg) override;

	//on_frame() is called every frame regardless whether the gui shows up.
	void on_frame() override;
	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	// void on_draw_ui() override;
	// on_draw_debug_ui() is called when debug window shows up
	void on_draw_debug_ui() override; // used

	static void plDante_request_set_style_asm(uintptr_t plDante, PlDanteStyleType requestedStyle);

	//--------------------------------------Sub methods for events that called from different hooks related to app.Player stuff somehow--------------------------------------//

	//uintptr_t threadCtxt, uintptr_t plManager uintptr_t pl
	template<typename T>
	static bool on_pl_mng_pl_add_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t, uintptr_t>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
		{
			_mod->_onPlMngPlAddEvent.subscribe(handler);
			return true;
		}
		return false;
	}

	//uintptr_t threadCtxt, uintptr_t pl
	template<typename T>
	static bool on_pl_mng_pl_add_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t, uintptr_t>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
		{
			_mod->_onPlMngPlAddEvent.unsubscribe(handler);
			return true;
		}
		return false;
	}

	template<typename T>
	static bool before_reset_pad_input_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, bool, bool*>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
		{
			_mod->_beforeResetPadInput.subscribe(handler);
			return true;
		}
		return false;
	}

	template<typename T>
	static bool before_reset_pad_input_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, bool, bool*>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
		{
			_mod->_beforeResetPadInput.unsubscribe(handler);
			return true;
		}
		return false;
	}

	//Return enum:
	//0 - NoGuard;
	//1 - Guard;
	//2 - DefendFail;
	//3 - AttackFail;
	//4 - AttackFailL;
	//5 - BothFail;
	//6 - GuardBreak;
	//
	
	/// <summary>
	/// Post-PL Vergil Guard Event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">uintptr_t threadccontex, uintptr_t pl, uintptr_t hc.DamageInfo, int *origFuncRes</param>
	/// <returns>enum: 0 - NoGuard, 1 - Guard, 2 - DefendFail, 3 - AttackFail, 4 - AttackFailL, 5 - BothFail, 6 - GuardBreak</returns>
	template<typename T>
	static void after_pl0800_guard_check_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t/*tc*/, uintptr_t/*pl*/, uintptr_t/*hc.DamageInfo*/, int*/*origFuncRes*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_afterPl0800GuardCheck.subscribe(handler);
	}


	template<typename T>
	static void after_pl0800_guard_check_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t, uintptr_t, int*>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_afterPl0800GuardCheck.unsubscribe(handler);
	}

	//uintptr_t tc, uintptr_t player

	/// <summary>
	/// Player Die Event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">uintptr_t threadccontex, uintptr_t player</param>
	template<typename T>
	static void on_pl_die_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t/*tc*/, uintptr_t/*player*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlSetDie.subscribe(handler);
	}

	template<typename T>
	static void on_pl_die_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlSetDie.unsubscribe(handler);
	}

	/// <summary>
	/// Player Update Lock-On event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">uintptr_t threadccontex, uintptr_t player</param>
	template<typename T>
	static void on_pl_lock_on_update_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t/*tc*/, uintptr_t/*player*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlLockOnUpdate.subscribe(handler);
	}

	template<typename T>
	static void on_pl_lock_on_update_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlLockOnUpdate.unsubscribe(handler);
	}

	//uintptr_t tc, uintptr_t pl0000.shell.QuicksilverWorldSlowAction, uintptr_t behaviortree.ActionArg

	/// <summary>
	/// Quicksilver Slow World Shell Event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">uintptr_t threadccontex, uintptr_t pl0000.shell.QuicksilverWorldSlowAction uintptr_t behaviortree.ActionArg</param>
	template<typename T>
	static void after_pl0000_quicksilver_slow_world_action_start_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t/*tc*/,
		uintptr_t/*pl0000.shell.QuicksilverWorldSlowAction*/, uintptr_t/*behaviortree.ActionArg*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_afterPl0000QuickSilverWorldSlowActionStart.subscribe(handler);
	}

	template<typename T>
	static void after_pl0000_quicksilver_slow_world_action_start_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t, uintptr_t>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_afterPl0000QuickSilverWorldSlowActionStart.unsubscribe(handler);
	}

	/// <summary>
	/// Quicksilver Slow World Shell Event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">uintptr_t threadccontex, uintptr_t pl0000.shell.QuicksilverWorldStopAction uintptr_t behaviortree.ActionArg</param>
	template<typename T>
	static void after_pl0000_quicksilver_stop_world_action_start_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t/*tc*/,
		uintptr_t/*pl0000.shell.QuicksilverWorldStopAction*/, uintptr_t/*behaviortree.ActionArg*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_afterPl0000QuickSilverWorldStopActionStart.subscribe(handler);
	}

	template<typename T>
	static void after_pl0000_quicksilver_stop_world_action_start_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t, uintptr_t>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_afterPl0000QuickSilverWorldStopActionStart.unsubscribe(handler);
	}

	//Hook installed before game checks is this style already setted and calls setStyle func
	//uintptr_t plDante, PlDanteStyleType style

	/// <summary>
	/// Player Dante On Change Style event
	/// Occurs before the style is actually set.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">pointer PlayerDante, enum PLDanteStyleType</param>
	template<typename T>
	static void pl0100_style_set_request_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t/*Dante*/, PlDanteStyleType /*style*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_plDanteSetStyleRequest.subscribe(handler);
	}

	template<typename T>
	static void pl0100_style_set_request_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t/*Dante*/, PlDanteStyleType /*style*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_plDanteSetStyleRequest.unsubscribe(handler);
	}

	//uintptr_t threadCtxt uintptr_t pl, float* val, int dtAddType, bool fixedValue

	/// <summary>
	/// On Add DT to Gauge Event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">uintptr_t threadCtxt uintptr_t player, float* val, int dtAddType, bool fixedValue</param>
	template<typename T>
	static void pl_add_dt_gauge_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t,/*pl*/ float* /*val*/, int/*dtAddType*/, bool/*fixedValue*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlAddDtGauge.subscribe(handler);
	}

	template<typename T>
	static void pl_add_dt_gauge_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t,/*pl*/ float* /*val*/, int/*dtAddType*/, bool/*fixedValue*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlAddDtGauge.unsubscribe(handler);
	}

	// uintptr_t threadCtxt, uintptr_t pl, uintptr_t HitInfo

	/// <summary>
	/// On Just Escape Event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">pointer ThreadContext, pointer player, pointer HitInfo</param>
	template<typename T>
	static void pl_on_just_escape_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t,/*pl*/ uintptr_t /*HitInfo*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlJustEscape.subscribe(handler);
	}

	template<typename T>
	static void pl_on_just_escape_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t,/*pl*/ uintptr_t /*HitInfo*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlJustEscape.unsubscribe(handler);
	}

	// uintptr_t threadCtxt, uintptr_t fsm2PlPosCntrAction

	/// <summary>
	/// Fsm2.PlayerPositionControllerAction On Update Speed Event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">threadCtxt, uintptr_t fsm2PlPosCntrAction</param>
	template<typename T>
	static void pl_on_fsm2_pos_cntr_action_update_speed_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t/*fsm2PlPosCntrAction*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlPosCntrActionUpdateSpeed.subscribe(handler);
	}

	template<typename T>
	static void pl_on_fsm2_pos_cntr_action_update_speed_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t/*fsm2PlPosCntrAction*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlPosCntrActionUpdateSpeed.unsubscribe(handler);
	}

	// uintptr_t threadCtxt, uintptr_t fsm2PlPosCntrAction, uintptr_t behavTreeActionArg

	/// <summary>
	/// Fsm2.PlayerPositionControllerAction On Start Event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">uintptr_t threadCtxt, uintptr_t fsm2PlPosCntrAction, uintptr_t behavTreeActionArg</param>
	template<typename T>
	static void pl_on_fsm2_pos_cntr_action_start_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t,/*fsm2PlPosCntrAction*/ uintptr_t /*behavTreeActionArg*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlPosCntrActionStart.subscribe(handler);
	}

	template<typename T>
	static void pl_on_fsm2_pos_cntr_action_start_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t,/*fsm2PlPosCntrAction*/ uintptr_t /*behavTreeActionArg*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlPosCntrActionStart.unsubscribe(handler);
	}

	// uintptr_t threadCtxt, uintptr_t plManager, uintptr_t pl, bool isUnload

	/// <summary>
	/// PlayerManager Player Unload Event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">uintptr_t threadCtxt, uintptr_t plManager, uintptr_t pl, bool isUnload</param>
	template<typename T>
	static void on_pl_manager_pl_unload_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t,/*plManager*/ uintptr_t /*pl*/, bool /*isUnload*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlManagerPlRemove.subscribe(handler);
	}

	template<typename T>
	static void on_pl_manager_pl_unload_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t,/*plManager*/ uintptr_t /*pl*/, bool /*isUnload*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPlManagerPlRemove.unsubscribe(handler);
	}

	// uintptr_t threadCtxt, uintptr_t pl0800, uintptr_t gameObjTarget, bool *skipOrigFuncCall
	
	/// <summary>
	/// Player Vergil Set Air Trick Action Event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">pointer threadCtxt, pointer vergil, pointer Target GameObject, bool skipOrigFuncCall</param>
	template<typename T>
	static void on_pl0800_set_air_trick_action_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t, /*pl*/ uintptr_t, /*gameObjTarget*/ bool* /*skipOrigFuncCall*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_pl0800SetAirTrickAction.subscribe(handler);
	}

	template<typename T>
	static void on_pl0800_set_air_trick_action_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t /*pl*/, uintptr_t, /*gameObjTarget*/ bool* /*skipOrigFuncCall*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_pl0800SetAirTrickAction.unsubscribe(handler);
	}


	// uintptr_t threadCtxt, uintptr_t fsm2.Player.PlayerAction, uintptr_t via.BehaviourTree.ActionArg, bool isNotifyOnly

	/// <summary>
	/// Fsm2 Player Action Notify Action End Event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">pointer threadcontex, pointer fsm2.Player.PlayerAction, pointer via.BehaviourTree.ActionArg, bool isNotifyOnly</param>
	template<typename T>
	static void on_fsm2_player_player_action_notify_action_end_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t, /*fsm2.Player.PlayerAction*/ uintptr_t,
		/*via.BehaviourTree.ActionArg*/ bool /*isNotifyOnly*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onFsmPlActionNotifyActionEnd.subscribe(handler);
	}

	template<typename T>
	static void on_fsm2_player_player_action_notify_action_end_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t, /*fsm2.Player.PlayerAction*/ uintptr_t,
		/*via.BehaviourTree.ActionArg*/ bool /*isNotifyOnly*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onFsmPlActionNotifyActionEnd.unsubscribe(handler);
	}

	// uintptr_t threadCtxt, uintptr_t plNero, bool isSecond
	
	/// <summary>
	/// Table Hopper Event
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="handler">pointer ThreadContext, pointer Nero, bool isSecond</param>
	template<typename T>
	static void pl_nero_set_table_hopper_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t, /*plNero*/ bool /*isSecond*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPLNeroSetTableHopper.subscribe(handler);
	}

	template<typename T>
	static void pl_nero_set_table_hopper_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t, /*plNero*/ bool /*isSecond*/>> handler)
	{
		if (handler != nullptr && _mod != nullptr)
			_mod->_onPLNeroSetTableHopper.unsubscribe(handler);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------//

private:

	void init_check_box_info() override;

	Events::Event<uintptr_t/*threadCtxt*/, uintptr_t/*plManager*/, uintptr_t/*player*/> _onPlMngPlAddEvent;
	Events::Event<uintptr_t, bool, bool*/*skip base func call*/> _beforeResetPadInput;
	Events::Event<uintptr_t, uintptr_t, /*pl0800*/ uintptr_t/*hitCtrlDamageInfo*/, int*/*return value*/ > _afterPl0800GuardCheck;
	Events::Event<uintptr_t, uintptr_t/*player*/> _onPlSetDie;
	Events::Event<uintptr_t, uintptr_t/*player*/> _onPlLockOnUpdate;
	Events::Event<uintptr_t, uintptr_t/*pl0000.shell.QuicksilverWorldSlowAction*/, uintptr_t /*behaviortree.ActionArg*/> _afterPl0000QuickSilverWorldSlowActionStart;
	Events::Event<uintptr_t, uintptr_t/*pl0000.shell.QuicksilverWorldStopAction*/, uintptr_t /*behaviortree.ActionArg*/> _afterPl0000QuickSilverWorldStopActionStart;
	Events::Event<uintptr_t,/*Dante*/ PlDanteStyleType /*style*/> _plDanteSetStyleRequest;
	Events::Event<uintptr_t, /*threadCtxt*/ uintptr_t,/*pl*/ float* /*val*/, int/*dtAddType*/, bool/*fixedValue*/> _onPlAddDtGauge;
	Events::Event<uintptr_t, /*threadCtxt*/ uintptr_t,/*pl*/ uintptr_t /*hitCntrl*/> _onPlJustEscape;
	Events::Event<uintptr_t, /*threadCtxt*/ uintptr_t,/*pl*/ bool /*isSecond*/> _onPLNeroSetTableHopper;
	Events::Event<uintptr_t, /*threadCtxt*/ uintptr_t /*fsm2PlPosCntrAction*/> _onPlPosCntrActionUpdateSpeed;
	Events::Event<uintptr_t, /*threadCtxt*/ uintptr_t /*fsm2PlPosCntrAction*/, uintptr_t /*behavTreeActionArg*/> _onPlPosCntrActionStart;
	Events::Event<uintptr_t, /*threadCtxt*/ uintptr_t,/*plManager*/ uintptr_t /*pl*/, bool /*isUnload*/> _onPlManagerPlRemove;
	Events::Event<uintptr_t, /*threadCtxt*/ uintptr_t /*pl*/, uintptr_t /*gameObjectTarget*/, bool* /*skipOrig*/> _pl0800SetAirTrickAction;
	Events::Event<uintptr_t, /*threadCtxt*/ uintptr_t /*fsm2.Player.PlayerAction*/, uintptr_t /*via.BehaviourTree.ActionArg*/, bool /*isNotifyOnly*/> _onFsmPlActionNotifyActionEnd;

	static void pl_reset_pad_input_hook(uintptr_t vm, uintptr_t pl, bool clearAutoPad);

	static void pl_manager_add_pl_hook(uintptr_t threadCtxt, uintptr_t plManager, uintptr_t pl);

	static void pl_set_die_hook(uintptr_t threadCtxt, uintptr_t pl);

	static void pl_update_lock_on_hook(uintptr_t threadCtxt, uintptr_t pl);

	static void pl0000_quicksilver_slow_world_action_start_hook(uintptr_t threadCtxt, uintptr_t shellQuicksilverWorldSlowAction, uintptr_t behaviorTreeArg);

	static void pl0000_quicksilver_stop_world_action_start_hook(uintptr_t threadCtxt, uintptr_t shellQuicksilverWorldStopAction, uintptr_t behaviorTreeArg);

	static void pl_add_dt_gauge_hook(uintptr_t threadCtxt, uintptr_t pl, float val, int dtAddType, bool fixedValue);

	static void pl_just_escape_hook(uintptr_t threadCtxt, uintptr_t pl, uintptr_t hitCntrl);

	//Check style set for pl0100 (single call)

	static naked void trickster_cmp_detour();

	static naked void swordmaster_cmp_detour();

	static naked void gunslinger_cmp_detour();

	static naked void royalguard_cmp_detour();

	//Return enum:
	//0 - NoGuard;
	//1 - Guard;
	//2 - DefendFail;
	//3 - AttackFail;
	//4 - AttackFailL;
	//5 - BothFail;
	//6 - GuardBreak;
	static int pl0800_on_guard_hook(uintptr_t threadCtxt, uintptr_t vergil, uintptr_t hitCtrlDamageInfo);

	static void fsm2_pl_pos_cntrl_action_start_hook(uintptr_t threadCntxt, uintptr_t fsm2PosCntrAction, uintptr_t behavtreeActionArg);

	static bool fsm2_pl_pos_cntrl_action_update_speed_hook(uintptr_t threadCntxt, uintptr_t fsm2PosCntrAction);

	static void pl_manager_pl_remove_hook(uintptr_t threadCntx, uintptr_t plManager, uintptr_t pl, bool isUnload);

	static void pl0800_set_air_trick_action_hook(uintptr_t threadCntx, uintptr_t pl0800, uintptr_t gameObjTarget);

	static void fsm2_player_player_action_notify_action_end_hook(uintptr_t threadCntx, uintptr_t fsm2PlayerPlayerAction, uintptr_t behaviourTreeActionArg, bool isNotifyOnly);

	static bool pl_nero_set_table_hopper_hook(uintptr_t threadCntxt, uintptr_t pl0000, bool isSecond);

	static inline PlayerTracker* _mod = nullptr;

	std::unique_ptr<FunctionHook> m_player_hook;
	std::unique_ptr<FunctionHook> m_summon_hook;
	std::unique_ptr<FunctionHook> m_incombat_hook;
	std::unique_ptr<FunctionHook> m_sin_hook;
	std::unique_ptr<FunctionHook> m_cos_hook;
	std::unique_ptr<FunctionHook> m_threshhold_hook;
	std::unique_ptr<FunctionHook> m_vergildata_hook;
	std::unique_ptr<FunctionHook> m_pl_manager_add_pl_hook;
	std::unique_ptr<FunctionHook> m_pad_input_reset_hook;
	std::unique_ptr<FunctionHook> m_pl0800_on_guard_hook;
	std::unique_ptr<FunctionHook> m_pl_set_die_hook;
	std::unique_ptr<FunctionHook> m_pl_lock_on_update_hook;
	std::unique_ptr<FunctionHook> m_pl0000_quicksilver_slow_world_action_start_hook;
	std::unique_ptr<FunctionHook> m_pl0000_quicksilver_stop_world_action_start_hook;
	std::unique_ptr<FunctionHook> m_set_trick_style_hook;
	std::unique_ptr<FunctionHook> m_set_sword_style_hook;
	std::unique_ptr<FunctionHook> m_set_gun_style_hook;
	std::unique_ptr<FunctionHook> m_set_royal_style_hook;
	std::unique_ptr<FunctionHook> m_pl_add_dt_gauge_hook;
	std::unique_ptr<FunctionHook> m_pl_just_escape_hook;
	std::unique_ptr<FunctionHook> m_fsm2_pl_pos_cntr_update_speed_hook;
	std::unique_ptr<FunctionHook> m_fsm2_pl_pos_cntr_start_action_hook;
	std::unique_ptr<FunctionHook> m_pl_remove_hook;
	std::unique_ptr<FunctionHook> m_pl0800_set_air_trick_action_hook;
	std::unique_ptr<FunctionHook> m_fsm2_player_player_action_notify_action_end_hook;
	std::unique_ptr<FunctionHook> m_pl_nero_set_table_hopper_hook;
};