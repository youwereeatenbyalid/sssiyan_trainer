#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include "mods/GameFunctions/GameFunc.hpp"
#include "events/Events.hpp"
namespace gf = GameFunctions;
class PlayerTracker : public Mod {
public:
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

  //uintptr_t threadCtxt, uintptr_t pl
  template<typename T>
  static bool pl_added_event_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t>> handler)
  {
	  if (handler != nullptr && _mod != nullptr)
	  {
		  _mod->_playerAdded.subscribe(handler);
		  return true;
	  }
	  return false;
  }

  //uintptr_t threadCtxt, uintptr_t pl
  template<typename T>
  static bool pl_added_event_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t>> handler)
  {
	  if (handler != nullptr && _mod != nullptr)
	  {
		  _mod->_playerAdded.unsubscribe(handler);
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
  template<typename T>
  static void after_pl0800_guard_check_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t, uintptr_t, int*/*origFuncRes*/>> handler)
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

  template<typename T>
  static void on_pl_die_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t>> handler)
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

  template<typename T>
  static void on_pl_lock_on_update_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t>> handler)
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

  template<typename T>
  static void after_pl0000_quicksilver_slow_world_action_start_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t, uintptr_t>> handler)
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

  template<typename T>
  static void after_pl0000_quicksilver_stop_world_action_start_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, uintptr_t, uintptr_t>> handler)
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

  template<typename T>
  static void pl_on_just_escape_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t,/*pl*/ uintptr_t /*hitCntrl*/>> handler)
  {
	  if (handler != nullptr && _mod != nullptr)
		  _mod->_onPlJustEscape.subscribe(handler);
  }

  template<typename T>
  static void pl_on_just_escape_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, /*threadCtxt*/ uintptr_t,/*pl*/ uintptr_t /*hitCntrl*/>> handler)
  {
	  if (handler != nullptr && _mod != nullptr)
		  _mod->_onPlJustEscape.unsubscribe(handler);
  }

private:

  // function hook instance for our detour, convinient wrapper 
  // around minhook
  void init_check_box_info() override;

  Events::Event<uintptr_t/*threadCtxt*/, uintptr_t/*player*/> _playerAdded;
  Events::Event<uintptr_t, bool, bool*/*skip base func call*/> _beforeResetPadInput;
  Events::Event<uintptr_t, uintptr_t/*hitCtrlDamageInfo*/, bool*/*skip base func call*/, bool*/*isUsingResFromDefaultFunc*/, int*/*return value*/ > _onPl0800GuardCheck;
  Events::Event<uintptr_t, uintptr_t, /*pl0800*/ uintptr_t/*hitCtrlDamageInfo*/, int*/*return value*/ > _afterPl0800GuardCheck;
  Events::Event<uintptr_t, uintptr_t/*player*/> _onPlSetDie;
  Events::Event<uintptr_t, uintptr_t/*player*/> _onPlLockOnUpdate;
  Events::Event<uintptr_t, uintptr_t/*pl0000.shell.QuicksilverWorldSlowAction*/, uintptr_t /*behaviortree.ActionArg*/> _afterPl0000QuickSilverWorldSlowActionStart;
  Events::Event<uintptr_t, uintptr_t/*pl0000.shell.QuicksilverWorldStopAction*/, uintptr_t /*behaviortree.ActionArg*/> _afterPl0000QuickSilverWorldStopActionStart;
  Events::Event<uintptr_t,/*Dante*/ PlDanteStyleType /*style*/> _plDanteSetStyleRequest;
  Events::Event<uintptr_t, /*threadCtxt*/ uintptr_t,/*pl*/ float* /*val*/, int/*dtAddType*/, bool/*fixedValue*/> _onPlAddDtGauge;
  Events::Event<uintptr_t, /*threadCtxt*/ uintptr_t,/*pl*/ uintptr_t /*hitCntrl*/> _onPlJustEscape;

  static void pl_reset_pad_input_hook(uintptr_t vm, uintptr_t pl, bool clearAutoPad);

  static void pl_manager_add_pl_hook(uintptr_t threadCtxt, uintptr_t plManager, uintptr_t pl);

  static void pl_set_die_hook(uintptr_t threadCtxt, uintptr_t pl);

  static void pl_update_lock_on_hook(uintptr_t threadCtxt, uintptr_t pl);

  static void pl0000_quicksilver_slow_world_action_start_hook(uintptr_t threadCtxt, uintptr_t shellQuicksilverWorldSlowAction, uintptr_t behaviorTreeArg);

  static void pl0000_quicksilver_stop_world_action_start_hook(uintptr_t threadCtxt, uintptr_t shellQuicksilverWorldStopAction, uintptr_t behaviorTreeArg);

  static void pl_add_dt_gauge_hook(uintptr_t threadCtxt, uintptr_t pl, float val, int dtAddType, bool fixedValue);

  static void pl_just_escape_hook(uintptr_t threadCtxt, uintptr_t pl, uintptr_t hitCntrl);

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

  static inline PlayerTracker *_mod = nullptr;

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
};