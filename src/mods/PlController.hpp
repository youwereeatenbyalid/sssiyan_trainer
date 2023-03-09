#pragma once
#include <cstdint>
#include "Mod.hpp"
#include "GameFunctions/GameFunc.hpp"
#include "GameFunctions/PositionController.hpp"
#include "PrefabFactory/PrefabFactory.hpp"

namespace PlCntr
{
	namespace gf = GameFunctions;
	struct HitControllerSettings
	{
		float baseAttackRate = 1.0f;//0x188
		float baseDmgReactionRate = 0;//0x1A8
		bool isAttackNoDie = true;// 0x1DF, (IsDamageZero)0x1DC
	};

	enum class DT
	{
		Human,
		Devil,
		SDT
	};

	enum class InterpolationMode
	{
		None = 0,
		FrontFade = 1,
		CrossFade = 2,
		SyncCrossFade = 3,
		SyncPointCrossFade = 4,
	};

	enum class InterpolationCurve
	{
		Linear = 0,
		Smooth = 1,
		EaseIN_ = 2,
		EaseOut = 3,
	};

	enum class ActionPriority
	{
		Normal,
		Death
	};

	enum class CharGroup
	{
		Player,
		Enemy
	};

	class PlController
	{
	private:
		uintptr_t _pl;

		REManagedObject* _missionSettingsManager = nullptr;
		REManagedObject* _playerManager = nullptr;

		static inline bool _isStaticInitRequested = true;		

	protected:

		static inline sdk::REMethodDefinition* _gameModelSetHPMethod = nullptr;
		static inline sdk::REMethodDefinition* _gameModelGetHPMethod = nullptr;
		static inline sdk::REMethodDefinition* _plSetActionMethod = nullptr;
		static inline sdk::REMethodDefinition* _gameObjDestroyMethod = nullptr;
		static inline sdk::REMethodDefinition* _gameModelSetDrawSelfMethod = nullptr;
		static inline sdk::REMethodDefinition* _gameModelSetEnableMethod = nullptr;
		static inline sdk::REMethodDefinition* _networkBBUpdateMethod = nullptr;
		static inline sdk::REMethodDefinition* _networkBBUpdateNetworkTypeMethod = nullptr;
		static inline sdk::REMethodDefinition* _plGotoWaitMethod = nullptr;
		static inline sdk::REMethodDefinition* _plEndCutSceneMethod = nullptr;
		static inline sdk::REMethodDefinition* _characterEndCutSceneMethod = nullptr;
		static inline sdk::REMethodDefinition* _plSetCommandActionMethod = nullptr;
		static inline sdk::REMethodDefinition* _plResetStatusMethod = nullptr;
		static inline sdk::REMethodDefinition* _gameObjGetComponentsArrayMethod = nullptr;
		static inline sdk::REMethodDefinition* _componentDestroyStaticMethod = nullptr;

		PlController(uintptr_t pl) : _pl(pl)
		{
			_missionSettingsManager = sdk::get_managed_singleton<REManagedObject>("app.MissionSettingManager");
			_playerManager = sdk::get_managed_singleton<REManagedObject>("app.PlayerManager");
			if (_isStaticInitRequested)
			{
				_gameModelSetHPMethod = sdk::find_method_definition("app.GameModel", "set_hp(System.Single)");				
				_gameModelGetHPMethod = sdk::find_method_definition("app.GameModel", "get_hp()");				
				_plSetActionMethod = sdk::find_method_definition("app.Player", "setAction(System.String, System.UInt32, System.Single, System.Single, "
					"via.motion.InterpolationMode, via.motion.InterpolationCurve, System.Boolean, System.Boolean, System.Boolean, app.GameModel.ActionPriority)");				
				_gameObjDestroyMethod = sdk::find_method_definition("via.GameObject", "destroy(via.GameObject)");				
				_gameModelSetDrawSelfMethod = sdk::find_method_definition("app.GameModel", "set_drawSelf(System.Boolean)");				
				_gameModelSetEnableMethod = sdk::find_method_definition("app.GameModel", "set_Enable(System.Boolean)");				
				_networkBBUpdateMethod = sdk::find_method_definition("app.NetworkBaseBehavior", "update()");				
				_networkBBUpdateNetworkTypeMethod = sdk::find_method_definition("app.NetworkBaseBehavior", "updateNetworkType()");				
				_plGotoWaitMethod = sdk::find_method_definition("app.Player", "gotoWait(System.String, System.String, System.Single, System.Boolean, System.Boolean)");				
				_plEndCutSceneMethod = sdk::find_method_definition("app.Player", "endCutScene(System.Int32, System.Single, app.character.Character.WetType, System.Single)");	
				_characterEndCutSceneMethod = sdk::find_method_definition("app.character.Character", "endCutScene(System.Int32, System.Single, app.character.Character.WetType, System.Single)");
				_plResetStatusMethod = sdk::find_method_definition("app.Player", "resetStatus(app.GameModel.ResetType)");				
				_plSetCommandActionMethod = sdk::find_method_definition("app.Player", "setCommandAction(System.String, System.Boolean, System.Boolean, System.Boolean, app.GameModel.ActionPriority, "
					"System.Single, via.motion.InterpolationMode)");
				_gameObjGetComponentsArrayMethod = sdk::find_method_definition("via.GameObject", "get_Components()");
				_componentDestroyStaticMethod = sdk::find_method_definition("via.Component", "destroy(via.Component)");

				_isStaticInitRequested = false;
			}
		}

		virtual void destroy_game_obj()
		{
			if (_gameObjDestroyMethod != nullptr)
			{
				auto gameObj = *(uintptr_t*)(_pl + 0x10);
				_gameObjDestroyMethod->call(sdk::get_thread_context(), gameObj);
			}
		}
		
	public:

		PlController() = delete;
		PlController(const PlController& other) = delete;

		virtual ~PlController()
		{
			//destroy_game_obj();
			_missionSettingsManager = _playerManager = nullptr;
			_pl = 0;
		}

		static inline sdk::REMethodDefinition* get_game_model_set_draw_self_method() noexcept { return _gameModelSetDrawSelfMethod; }

		static inline sdk::REMethodDefinition* get_game_model_set_enable_method() noexcept { return _gameModelSetEnableMethod; }

		static inline sdk::REMethodDefinition* get_network_base_bhvr_update_method() noexcept { return _networkBBUpdateMethod; }

		static inline sdk::REMethodDefinition* get_network_base_bhvr_update_network_type_method() noexcept { return _networkBBUpdateNetworkTypeMethod; }

		static inline sdk::REMethodDefinition* get_pl_set_action_method() noexcept { return _plSetActionMethod; }

		static inline sdk::REMethodDefinition* get_game_model_set_hp_method() noexcept { return _gameModelSetHPMethod; }

		static inline sdk::REMethodDefinition* get_game_model_get_hp_method() noexcept { return _gameModelGetHPMethod; }

		static inline sdk::REMethodDefinition* get_pl_goto_wait_method() noexcept { return _plGotoWaitMethod; }

		static inline sdk::REMethodDefinition* get_pl_reset_status_method() noexcept { return _plResetStatusMethod; }

		static inline sdk::REMethodDefinition* get_pl_end_cutscene_method() noexcept { return _plEndCutSceneMethod; }

		static inline sdk::REMethodDefinition* get_character_end_cutscene_method() noexcept { return _characterEndCutSceneMethod; }

		static inline sdk::REMethodDefinition* get_pl_set_command_action_method() noexcept { return _plSetCommandActionMethod; }

		static inline sdk::REMethodDefinition* get_game_obj_get_components_method() noexcept { return _gameObjGetComponentsArrayMethod; }

		static inline sdk::REMethodDefinition* get_component_destroy_static_method() noexcept { return _componentDestroyStaticMethod; }

		inline uintptr_t get_pl() const noexcept { return _pl; }

		virtual bool set_dt(DT dt, bool isNotProduction = false) = 0;

		inline CharGroup get_char_group() const noexcept { return *(CharGroup*)(_pl + 0x108); }

		inline DT get_cur_dt() { return *(DT*)(_pl + 0x9B0); }

		inline void change_character_group(CharGroup group) { *(int*)(_pl + 0x108) = (int)group; }

		inline void set_hp(float val)
		{
			if (_gameModelSetHPMethod != 0)
				_gameModelSetHPMethod->call(sdk::get_thread_context(), (REManagedObject*)_pl, val);
		}

		inline bool is_control() const noexcept { return *(bool*)(_pl + 0x4C6); }

		inline bool get_is_no_die() const noexcept { return *(bool*)(_pl + 0x11F1); }

		inline void set_is_control(bool val) noexcept { *(bool*)(_pl + 0x4C6) = val; }

		//Check if PlayerManager.plList has only 1 character.
		bool is_only_1_pl_character()
		{
			if (_playerManager == 0)
				throw std::exception("PlayerManager is null.");
			return gf::ListController::get_list_count(*(uintptr_t*)((uintptr_t)_playerManager + 0x70)) == 1;
		}

		inline bool is_in_players_list() noexcept
		{
			update_pl_manager();
			if (_playerManager == 0)
				throw std::exception("PlayerManager is null.");
			auto list = *(uintptr_t*)((uintptr_t)_playerManager + 0x70);
			auto lstCount = gf::ListController::get_list_count(list);
			for (int i = 0; i < lstCount; i++)
			{
				if (gf::ListController::get_item<uintptr_t>(list, i) == _pl)
					return true;
			}
			return false;
		}

		virtual void set_action(const wchar_t* actionStr, int layerNo = 0, float startFrame = 0.0f, float interpolationFrame = 0.0f, InterpolationMode mode = InterpolationMode::SyncCrossFade,
			InterpolationCurve curve = InterpolationCurve::Smooth, bool isImmediate = false, bool passSelect = true, bool isPuppetTransition = false,
			ActionPriority actionPriority = ActionPriority::Normal) const noexcept
		{
			if (_plSetActionMethod != nullptr)
			{
				gf::SysString str(actionStr);
				_plSetActionMethod->call(sdk::get_thread_context(), (REManagedObject*)_pl, str.get_net_str(), layerNo, startFrame, interpolationFrame, mode, curve, isImmediate,
					passSelect, isPuppetTransition, actionPriority);
			}
		}

		gf::Vec3 get_pl_pos() const noexcept { return *(gf::Vec3*)(get_transform() + 0x30); }

		gf::Vec3 get_ground_pos() const noexcept { return *(gf::Vec3*)(_pl + 0x3E0); }

		gf::Quaternion get_rot() const noexcept { return *(gf::Quaternion*)(get_transform() + 0x40); }

		inline uintptr_t get_transform() const noexcept
		{
			auto gameObj = *(uintptr_t*)(_pl + 0x10);
			return *(uintptr_t*)(gameObj + 0x18);
		}

		virtual void destroy_all_related_shells()
		{
			auto shellMgr = sdk::get_managed_singleton<REManagedObject>("app.ShellManager");
			if (shellMgr == nullptr)
				return;
			size_t shellListCap = 0;
			auto shellList = gf::ListController::get_dotnet_list<REManagedObject*>(*(uintptr_t*)((uintptr_t)shellMgr + 0x60), shellListCap);
			if (!shellList.has_value())
				return;
			for (const auto i : shellList.value())
			{
				if (*(uintptr_t*)((uintptr_t)i + 0x2E0) == _pl)
				{
					sdk::call_object_func_easy<void*>(shellMgr, "requestRemoveObject(app.Shell)", i);
					sdk::call_object_func_easy<void*>(shellMgr, "doUpdate()");
					if (_gameObjDestroyMethod != nullptr)//ThreadCntx is not needed here
						_gameObjDestroyMethod->call((REManagedObject*)(*(uintptr_t*)((uintptr_t)i + 0x10)), (REManagedObject*)(*(uintptr_t*)((uintptr_t)i + 0x10)));
				}
			}
		}

		//Changes gameObj pos & colliders pos
		void set_pos_full(gf::Vec3 pos) const noexcept { gf::Transform_SetPosition::set_character_pos(_pl, pos); }

		virtual void set_hitcontroller_settings(HitControllerSettings settings)
		{
			auto hitCtrl = *(uintptr_t*)(_pl + 0x1F8);
			*(float*)(hitCtrl + 0x188) = settings.baseAttackRate;
			*(float*)(hitCtrl + 0x1A8) = settings.baseDmgReactionRate;
			*(bool*)(hitCtrl + 0x1DF) = settings.isAttackNoDie;
		}

		inline REManagedObject* get_pl_manager() const noexcept { return _playerManager; }

		inline REManagedObject* get_mission_setting_manager() const noexcept { return _missionSettingsManager; }

		virtual inline void set_is_no_die(bool val) noexcept 
		{
			*(bool*)(_pl + 0x11F1) = val;
		}

		// Calls GameModel.set_draw_self(...)
		inline void set_draw_self(bool val) const noexcept
		{
			if (_gameModelSetDrawSelfMethod != nullptr)
				_gameModelSetDrawSelfMethod->call(sdk::get_thread_context(), _pl, val);
		}

		//Calls GameModel.set_enable(...)
		inline void set_enable(bool val) const noexcept
		{
			if (_gameModelSetEnableMethod != nullptr)
				_gameModelSetEnableMethod->call(sdk::get_thread_context(), _pl, val);
		}

		inline void enable_physics_char_controller(bool val)
		{
			auto charController = *(uintptr_t*)(_pl + 0x2F0);
			auto subCharController = *(uintptr_t*)(_pl + 0x2F8);
			*(bool*)(charController + 0x30) = *(bool*)(subCharController + 0x30) = val;
		}

		//Activate/deactivate network type and physics char controller
		inline void set_network_base_active(bool val)
		{
			auto networkSubType = *(uintptr_t*)(_pl + 0x60);
			int* networkType = (int*)(_pl + 0x54);
			int* networkRequestType = (int*)(_pl + 0x58);
			int* networkObjectType = (int*)(_pl + 0x5C);
			unsigned int* networkSubTypeBit = (unsigned int*)(networkSubType + 0x10);
			int* mediationType = (int*)(_pl + 0xF4);
			if (val)
			{
				//*networkType = 4;
				*networkRequestType = 4;
				*networkObjectType = 16;
				*networkSubTypeBit = 0;
				*mediationType = 2;
			}
			else
			{
				//*networkType = 0;
				*networkRequestType = 0;
				*networkObjectType = 0;
				*networkSubTypeBit = 1;
				*mediationType = 3;
			}
			*(bool*)(_pl + 0x98) = true;// need to call special func actually
			enable_physics_char_controller(val);
			if (_networkBBUpdateNetworkTypeMethod != nullptr)
				_networkBBUpdateNetworkTypeMethod->call(sdk::get_thread_context(), (REManagedObject*)_pl);
		}

		inline int get_cur_network_type() const noexcept { return *(int*)(_pl + 0x54); }

		inline int get_cur_mediation_type() const noexcept { return *(int*)(_pl + 0xF4); }

		inline void pl_manager_request_remove(bool unloadRequest = false)
		{
			if (_playerManager != nullptr)
				sdk::call_object_func_easy<void*>(_playerManager, "removePlayer(app.Player, System.Boolean)", (REManagedObject*)_pl, unloadRequest);
		}

		inline void goto_wait(const wchar_t* actionName1 = L"", const wchar_t* actionName2 = L"", float interpolationFrame = 0.0f, bool forceInterpolationFrameZero = true, bool puppetTransition = true)
		{
			if (_plGotoWaitMethod == nullptr)
				return;
			gf::SysString str1(actionName1);
			gf::SysString str2(actionName2);
			_plGotoWaitMethod->call(sdk::get_thread_context(), get_pl(), str1.get_net_str(), str2.get_net_str(), interpolationFrame, forceInterpolationFrameZero, puppetTransition);
		}

		inline void goto_wait(uintptr_t actionName1, uintptr_t actionName2, float interpolationFrame = 0.0f, bool forceInterpolationFrameZero = true, bool puppetTransition = true)
		{
			if (_plGotoWaitMethod == nullptr)
				return;
			_plGotoWaitMethod->call(sdk::get_thread_context(), get_pl(), actionName1, actionName2, interpolationFrame, forceInterpolationFrameZero, puppetTransition);
		}

		virtual void end_cutscene(int actionId = 0, float commonTimer = 0.0F, int charWetType = 0, float startFrame = 0.0F)
		{
			if (_plEndCutSceneMethod == nullptr)
				return;
			_plEndCutSceneMethod->call(sdk::get_thread_context(), _pl, actionId, commonTimer, charWetType, startFrame);
		}

		inline void pl_reset_status()
		{
			if (_plResetStatusMethod == nullptr)
				return;
			_plResetStatusMethod->call(sdk::get_thread_context(), _pl);
		}

		inline void update_pl_manager()
		{
			_playerManager = sdk::get_managed_singleton<REManagedObject>("app.PlayerManager");
		}
	};
}