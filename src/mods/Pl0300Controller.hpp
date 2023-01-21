#pragma once
#include "Mod.hpp"
#include "PlSetActionData.hpp"
#include "GameFunctions/GameFunc.hpp"
#include "GameFunctions/PositionController.hpp"

namespace gf = GameFunctions;

namespace Pl0300Controller
{
	class Pl0300Controller final
	{
	public:

		struct HitControllerSettings
		{
			float baseAttackRate = 1.0f;//0x188
			float baseDmgReactionRate = 0;//0x1A8
			bool isAttackNoDie = true;// 0x1DF, (IsDamageZero)0x1DC
		};

		//size - 0x30, app.pl0300....TeleportParameter - 0xA0
		struct TeleportTimingParams
		{
			float appearsSec = 1.0f; //0x60
			float appearsSecLong = 4.0f; //0x64
			float appearsSecOnBack = 0.1F; //0x68
			float appearsSecOnCameraFront = 0.5F;//0x6C
			float appearsSecOfOverhead = 0.1F;//0x70
			float appearsSecOnStab = 0.25F;//0x74
			float appearsSecOnCenter = 0.1F;//0x78
			float apeearsSec2Owner = 1.0F;//0x7C
			float appearsSecOnCommandCombo = 0.1F;//0x80
			float appearsSecOnAirRaid = 0.5F;//0x84
			float appearsSecOnDead = 0.1f;//0x88
			float appearsSecOnPlayerDead = 0.1;//0x8C

			void set_all(float val) noexcept
			{
				appearsSec = appearsSecLong = appearsSecOnBack = appearsSecOnCameraFront = appearsSecOfOverhead = appearsSecOnStab = appearsSecOnCenter = appearsSecOnCommandCombo =
					appearsSecOnAirRaid = appearsSecOnDead = appearsSecOnPlayerDead = val;
			}
		};
		static_assert(sizeof(TeleportTimingParams) == 0x30);

		struct AirRaidSettings
		{
			bool useSummonedSwords = false;//0x90
			bool useOptionalTrick = true;//0xA0

			float secConcentrate = 2.0f;//0x10, 0x14 - default&onHard

			int attackNum = 3;//0x2C

			float radiusOfArea = 43.0f;//0x70
			float radiusOfRevolution = 80.0f;//0x74
			float heightOfArenaSide = 1.5f;//0x78
			float heightOnOutside = 8.0f;//0x7C;
			float distanceCheckGround = 5.0f;//0x80
			float radiusFinishAttack = 5.0f;//0x50
		};

		enum class Pl0300Type
		{
			Em6000Friendly,//Regular pl0300 with m21 AI
			PlHelper //Pl0300 with (optional) main player's pad input and special settings for using it as playable character
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

	private:

		friend class Pl0300ControllerManager;
		friend class std::allocator<Pl0300ControllerManager>;

		Pl0300Type _pl0300Type;

		uintptr_t _pl0300 = 0;

		bool _isIgnorePadInputRestoring = false;
		bool _isStarted = false;
		bool _isInPlList = false;
		bool _isHitCtrlDataSetRequested = false;
		bool _isJcNumSetRequested = false;
		bool _isEmStepSetRequested = false;
		bool _emStepRequest = false;
		bool _isSetIsNoDieRequested = false;
		bool _setIsNoDieRequestedVal = false;
		bool _isSetDtRequested = false;
		bool _isAddToPlListRequested = false;
		bool _isSetTeleportTimingParamsRequested = false;
		bool _isKeepingOriginalPadInput = false;
		bool _useCustomTrickUpdate = false;

		int _requestedJcNum = 1;

		DT _requestedDT = DT::Human;

		TeleportTimingParams _teleportTimingParamsRequested;

		REManagedObject* _missionSettingsManager = nullptr;
		REManagedObject* _playerManager = nullptr;

		static inline sdk::REMethodDefinition* _plSetActionMethod = nullptr;
		static inline sdk::REMethodDefinition* _pl0300SetDtMethod = nullptr;
		static inline sdk::REMethodDefinition* _pl0300GenDoppelMethod = nullptr;
		static inline sdk::REMethodDefinition* _pl0300DestroyDoppelMethod = nullptr;
		static inline sdk::REMethodDefinition* _pl0300RequestDestroyDoppelMethod = nullptr;
		static inline sdk::REMethodDefinition* _gameModelSetDrawSelfMethod = nullptr;
		static inline sdk::REMethodDefinition* _gameModelSetEnableMethod = nullptr;
		static inline sdk::REMethodDefinition* _gameModelSetHPMethod = nullptr;
		static inline sdk::REMethodDefinition* _gameModelGetHPMethod = nullptr;
		static inline sdk::REMethodDefinition* _gameObjDestroyMethod = nullptr;
		static inline sdk::REMethodDefinition* _networkBBUpdateMethod = nullptr;
		static inline sdk::REMethodDefinition* _networkBBUpdateNetworkTypeMethod = nullptr;
		static inline sdk::REMethodDefinition* _pl0300DoStartMethod = nullptr;
		static inline sdk::REMethodDefinition* _baseBehaviorStartMethod = nullptr;
		static inline sdk::REMethodDefinition* _pl0300startAirRaidMethod = nullptr;
		static inline sdk::REMethodDefinition* _pl0300SetActionMethod = nullptr;
		static inline sdk::REMethodDefinition* _plResetStatusMethod = nullptr;
		static inline sdk::REMethodDefinition* _plGotoWaitMethod = nullptr;
		static inline sdk::REMethodDefinition* _pl0300SetActionFromThinkMethod = nullptr;
		static inline sdk::REMethodDefinition* _plSetCommandActionMethod = nullptr;

		std::weak_ptr<Pl0300Controller> _doppel;
		std::weak_ptr<Pl0300Controller> _owner;

		static inline Pl0300ControllerManager* _pl0300Manager = nullptr;

		HitControllerSettings _hcLastSettings;

		//static inline const wchar_t* _jceStr = L"Zigenzan_Zetsu_Start";//I just want to keep this move's name here

		//dctor also calls this
		void destroy_game_obj();

		Pl0300Controller(uintptr_t pl0300, Pl0300Type type, bool isKeepingOriginalPadInput = false);

		void check_doppel_ref_correct();
		
		typedef void(*f_trick_pos_update)(uintptr_t/*fsmPl0300Teleport*/, const std::shared_ptr<Pl0300Controller>&, bool*/*ignore custom and run default*/);

		f_trick_pos_update _trick_update = nullptr;

	public:

		Pl0300Controller() = delete;
		Pl0300Controller(const Pl0300Controller& other) = delete;

		~Pl0300Controller()
		{
			destroy_game_obj();
			_missionSettingsManager = _playerManager  = nullptr;
		}

		enum class CharGroup
		{
			Player,
			Enemy
		};

		static inline sdk::REMethodDefinition* get_game_model_set_draw_self_method() noexcept { return _gameModelSetDrawSelfMethod; }

		static inline sdk::REMethodDefinition* get_game_model_set_enable_method() noexcept { return _gameModelSetEnableMethod; }

		static inline sdk::REMethodDefinition* get_network_base_bhvr_update_method() noexcept { return _networkBBUpdateMethod; }

		static inline sdk::REMethodDefinition* get_network_base_bhvr_update_network_type_method() noexcept { return _networkBBUpdateNetworkTypeMethod; }

		static inline sdk::REMethodDefinition* get_pl_set_action_method() noexcept { return _plSetActionMethod; }

		static inline sdk::REMethodDefinition* get_game_model_set_hp_method() noexcept { return _gameModelSetHPMethod; }

		static inline sdk::REMethodDefinition* get_game_model_get_hp_method() noexcept { return _gameModelGetHPMethod; }

		static inline sdk::REMethodDefinition* get_pl_goto_wait_method() noexcept { return _plGotoWaitMethod; }

		static inline sdk::REMethodDefinition* get_pl_reset_status_method() noexcept { return _plResetStatusMethod; }

		//get doppel's controller if exists
		inline std::weak_ptr<Pl0300Controller> get_doppel_ctrl() { return _doppel; }

		//get doppel's owner controller
		inline std::weak_ptr<Pl0300Controller> get_owner_ctrl() { return _owner; }

		inline uintptr_t get_pl0300() const noexcept { return _pl0300; }

		inline void change_character_group(CharGroup group) { *(int*)(_pl0300 + 0x108) = (int)group; }

		inline void set_center_floor(gf::Vec3 pos)
		{
			auto emParam = *(uintptr_t*)(_pl0300 + 0x1768);
			if (emParam == 0)
				return;
			*(gf::Vec3*)(emParam + 0xA0) = pos;
		}

		//Function: f(uintptr_t fsmPl0300Teleport, const std::shared_ptr& pl0300, bool* isSkipPl0300_teleport_calc_f), called every physics frame when pl0300 teleporting
		template<typename F>
		inline void set_trick_update_f(F &&func) noexcept { _trick_update = func; }

		inline void set_hp(float val)
		{
			if (_gameModelSetHPMethod != 0)
				_gameModelSetHPMethod->call(sdk::get_thread_context(), (REManagedObject*)_pl0300, val);
		}

		inline CharGroup get_char_group() const noexcept { return *(CharGroup*)(_pl0300 + 0x108); }

		inline DT get_cur_dt() { return *(DT*)(_pl0300 + 0x9B0); }
		
		inline bool set_dt(DT dt, bool isNotProduction = false) 
		{
			if (!_isStarted)
			{
				_isSetDtRequested = true;
				_requestedDT = dt;
				return false;
			}
			if (_pl0300SetDtMethod != nullptr && !is_doppel())
				return _pl0300SetDtMethod->call<bool>(sdk::get_thread_context(), (REManagedObject*)_pl0300, (int)dt, isNotProduction);
			else
				*(int*)(_pl0300 + 0x9B0) = (int)dt;
			return false;
		}
		
		//Check if PlayerManager.plList has only 1 character.
		bool is_only_1_pl_character()
		{
			if (_playerManager == 0)
				throw std::exception("PlayerManager is null.");
			return gf::ListController::get_list_count(*(uintptr_t*)((uintptr_t)_playerManager + 0x70)) == 1;
		}

		inline bool is_enemy() const { return *(bool*)(_pl0300 + 0x17E0); }

		//Is GameModel's Start() function called;
		inline bool is_started() const noexcept { return _isStarted; }

		inline bool is_done_air_raid() const { return *(bool*)(_pl0300 + 0x1BB2); }

		inline bool is_keeping_original_pad_input() const noexcept { return _isKeepingOriginalPadInput; }

		inline bool is_control() const noexcept { return *(bool*)(_pl0300 + 0x4C6); }

		inline bool get_is_no_die() const noexcept { return *(bool*)(_pl0300 + 0x11F1); }

		inline void set_is_no_die(bool val) noexcept 
		{ 
			if (!_isStarted)
			{
				_isSetIsNoDieRequested = true;
				_setIsNoDieRequestedVal = val;
			}
			*(bool*)(_pl0300 + 0x11F1) = val; 
		}

		inline void set_is_control(bool val) noexcept { *(bool*)(_pl0300 + 0x4C6) = val; }

		std::optional<TeleportTimingParams> get_teleport_timing_params() const noexcept
		{
			auto emParam = *(uintptr_t*)(_pl0300 + 0x1768);
			if (emParam == 0)
				return std::nullopt;
			auto teleportParam = *(uintptr_t*)(emParam + 0xE0);
			if(teleportParam == 0)
				return std::nullopt;
			return std::make_optional<TeleportTimingParams>(*(TeleportTimingParams*)(teleportParam + 0x60));
		}

		TeleportTimingParams* get_p_teleport_timing_params() noexcept
		{
			auto emParam = *(uintptr_t*)(_pl0300 + 0x1768);
			if (emParam == 0)
				return nullptr;
			auto teleportParam = *(uintptr_t*)(emParam + 0xE0);
			if (teleportParam == 0)
				return nullptr;
			return (TeleportTimingParams*)(teleportParam + 0x60);
		}

		//Teleport params shares between Vergil owner and doppel;
		void set_teleport_timing_params(const TeleportTimingParams& params)
		{
			if (!_isStarted)
			{
				_isSetTeleportTimingParamsRequested = true;
				_teleportTimingParamsRequested = params;
				return;
			}
			auto emParam = *(uintptr_t*)(_pl0300 + 0x1768);
			if (emParam == 0)
				return;
			auto teleportParam = *(uintptr_t*)(emParam + 0xE0);
			if (teleportParam == 0)
				return;
			*(TeleportTimingParams*)(teleportParam + 0x60) = params;
		}

		inline std::optional<bool> is_em_step_enabled() const noexcept
		{
			auto plData = *(uintptr_t*)(_pl0300 + 0xD20);
			if (plData == 0)
				return std::nullopt;
			return std::make_optional<bool>(*(bool*)(plData + 0x19));
		}

		inline void set_em_step_enabled(bool val)
		{
			if (!_isStarted)
			{
				_emStepRequest = val;
				_isEmStepSetRequested = true;
				return;
			}
			auto plData = *(uintptr_t*)(_pl0300 + 0xD20);
			auto saveData = *(uintptr_t*)(_pl0300 + 0x1F10);
			if (plData != 0 && saveData != 0)
				*(bool*)(plData + 0x19) = *(bool*)(saveData + 0x19) = val;
		}

		inline int get_jcut_num() const noexcept { return *(int*)(_pl0300 + 0x1F5C); }

		inline void set_jcut_num(int num) noexcept 
		{
			if (!_isStarted)
			{
				_isJcNumSetRequested = true;
				_requestedJcNum = num;
				return;
			}
			*(int*)(_pl0300 + 0x1F5C) = num; 
		}

		inline void set_is_enemy(bool isEm) noexcept { *(bool*)(_pl0300 + 0x17E0) = isEm; }

		inline void pl0300_do_start()
		{
			if (_pl0300DoStartMethod != nullptr)
				_pl0300DoStartMethod->call(sdk::get_thread_context(), _pl0300);
		}

		inline Pl0300Type get_pl0300_type() const noexcept { return _pl0300Type; }

		//Is pl_manager_request_add() was called;
		inline bool is_in_players_list() const noexcept { return _isInPlList; }

		inline bool is_doppels_owner(uintptr_t pl0300Doppel) 
		{ 
			if (pl0300Doppel == 0)
				return false;
			return *(uintptr_t*)(_pl0300 + 0x1C10) == pl0300Doppel; 
		}

		bool is_doppel() const noexcept { return *(bool*)(_pl0300 + 0x1F60); }

		//Use custom function for fsm.pl0300TrickUpdate
		void use_custom_trick_update(bool val) noexcept { _useCustomTrickUpdate = val; }

		inline bool is_using_custom_trick_update() const noexcept { return _useCustomTrickUpdate; }

		void set_action(const wchar_t* actionStr, int layerNo = 0, float startFrame = 0.0f, float interpolationFrame = 0.0f, InterpolationMode mode = InterpolationMode::SyncCrossFade, 
			InterpolationCurve curve = InterpolationCurve::Smooth, bool isImmediate = false, bool passSelect = true, bool isPuppetTransition = false, 
			ActionPriority actionPriority = ActionPriority::Normal) const noexcept
		{
			if (_pl0300SetActionMethod != nullptr)
			{
				gf::SysString str(actionStr);
				_pl0300SetActionMethod->call(sdk::get_thread_context(), (REManagedObject*)_pl0300, str.get_net_str(), layerNo, startFrame, interpolationFrame, mode, curve, isImmediate,
					passSelect, isPuppetTransition, actionPriority);
			}
		}

		//Set pl0300 action via Player.SetAction(...). Do not create doppel with this, use generate_doppel() function instead;
		void set_action(const gf::SysString* actionStr, int layerNo = 0, float startFrame = 0.0f, float interpolationFrame = 0.0f, InterpolationMode mode = InterpolationMode::SyncCrossFade,
			InterpolationCurve curve = InterpolationCurve::Smooth, bool isImmediate = false, bool passSelect = true, bool isPuppetTransition = false,
			ActionPriority actionPriority = ActionPriority::Normal) const noexcept
		{
			if (_pl0300SetActionMethod != nullptr)
			{
				_pl0300SetActionMethod->call(sdk::get_thread_context(), (REManagedObject*)_pl0300, actionStr->get_net_str(), layerNo, startFrame, interpolationFrame, mode, curve, isImmediate,
					passSelect, isPuppetTransition, actionPriority);
			}
		}

		void set_action_from_think(const wchar_t* action, UINT32 hash) const noexcept
		{
			gf::SysString actionStr = action;
			set_action_from_think(&actionStr, hash);
		}

		void set_action_from_think(const gf::SysString* action, UINT32 hash) const noexcept
		{
			if (_pl0300SetActionFromThinkMethod != nullptr)
				_pl0300SetActionFromThinkMethod->call(sdk::get_thread_context(), (REManagedObject*)_pl0300, action->get_net_str(), hash);
		}

		void set_pl_command_action(const wchar_t* action, bool initAdd, bool initOverwrite, bool isPuppetTransition, ActionPriority priority, float interpolationFrame, InterpolationMode mode) const noexcept
		{
			if (_plSetCommandActionMethod != nullptr)
			{
				gf::SysString actionStr = action;
				_plSetCommandActionMethod->call(sdk::get_thread_context(), (REManagedObject*)_pl0300, actionStr.get_net_str(), initAdd, initOverwrite, isPuppetTransition, priority, 
					interpolationFrame, mode);
			}
		}

		gf::Vec3 get_pl0300_pos() const noexcept
		{
			auto gameObj = *(uintptr_t*)(_pl0300 + 0x10);
			auto transform = *(uintptr_t*)(gameObj + 0x18);
			return *(gf::Vec3*)(transform + 0x30);
		}

		gf::Vec3 get_ground_pos() const noexcept { return *(gf::Vec3*)(_pl0300 + 0x3E0); }

		gf::Quaternion get_pl0300_rot() const noexcept
		{
			auto gameObj = *(uintptr_t*)(_pl0300 + 0x10);
			auto transform = *(uintptr_t*)(gameObj + 0x18);
			return *(gf::Quaternion*)(transform + 0x40);
		}

		//Summoned swords will not disappear by itself
		void destroy_all_related_shells()
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
				if (*(uintptr_t*)((uintptr_t)i + 0x2E0) == _pl0300)
				{
					sdk::call_object_func_easy<void*>(shellMgr, "requestRemoveObject(app.Shell)", i);
					sdk::call_object_func_easy<void*>(shellMgr, "doUpdate()");
					if (_gameObjDestroyMethod != nullptr)//ThreadCntx is not needed here
						_gameObjDestroyMethod->call((REManagedObject*)(*(uintptr_t*)((uintptr_t)i + 0x10)), (REManagedObject*)(*(uintptr_t*)((uintptr_t)i + 0x10)));
				}
			}
		}

		//Changes gameObj pos & colliders pos
		void set_pos_full(gf::Vec3 pos) const noexcept { gf::Transform_SetPosition::set_character_pos(_pl0300, pos); }

		void set_hitcontroller_settings(HitControllerSettings settings)
		{
			_hcLastSettings = settings;
			if (!_isStarted)
			{
				_isHitCtrlDataSetRequested = true;
				return;
			}
			auto hitCtrl = *(uintptr_t*)(_pl0300 + 0x1F8);
			uintptr_t yamatoHitCtrl = 0;
			auto yamatoWeapon = *(uintptr_t*)(_pl0300 + 0x2018);
			if (yamatoWeapon != 0)
				yamatoHitCtrl = *(uintptr_t*)(yamatoWeapon + 0x1F8);
			*(float*)(hitCtrl + 0x188) = settings.baseAttackRate;
			*(float*)(hitCtrl + 0x1A8) = settings.baseDmgReactionRate;
			*(bool*)(hitCtrl + 0x1DF) = settings.isAttackNoDie;
			if (yamatoHitCtrl != 0)
			{
				*(float*)(yamatoHitCtrl + 0x188) = settings.baseAttackRate;
				*(float*)(yamatoHitCtrl + 0x1A8) = settings.baseDmgReactionRate;
				*(bool*)(yamatoHitCtrl + 0x1DF) = settings.isAttackNoDie;
			}
		}

		void generate_doppel(bool isFirst, float hp = 250.0f, float attackRate = 0.35f);

		void destroy_doppel();

		uintptr_t get_doppel() const noexcept { return *(uintptr_t*)(_pl0300 + 0x1C10); }

		inline REManagedObject* get_pl_manager() const noexcept { return _playerManager; }

		inline REManagedObject* get_mission_setting_manager() const noexcept { return _missionSettingsManager; }

		// Calls GameModel.set_draw_self(...)
		inline void set_draw_self(bool val) const noexcept
		{
			if (_gameModelSetDrawSelfMethod != nullptr)
				_gameModelSetDrawSelfMethod->call(sdk::get_thread_context(), _pl0300, val);
		}

		//Calls GameModel.set_enable(...)
		inline void set_enable(bool val) const noexcept
		{
			if (_gameModelSetEnableMethod != nullptr)
				_gameModelSetEnableMethod->call(sdk::get_thread_context(), _pl0300, val);
		}

		inline void enable_physics_char_controller(bool val)
		{
			auto charController = *(uintptr_t*)(_pl0300 + 0x2F0);
			auto subCharController = *(uintptr_t*)(_pl0300 + 0x2F8);
			*(bool*)(charController + 0x30) = *(bool*)(subCharController + 0x30) = val;
		}

		//Activate/deactivate network type and physics char controller
		inline void set_network_base_active(bool val)
		{
			auto networkSubType = *(uintptr_t*)(_pl0300 + 0x60);
			int* networkType = (int*)(_pl0300 + 0x54);
			int* networkRequestType = (int*)(_pl0300 + 0x58);
			int* networkObjectType = (int*)(_pl0300 + 0x5C);
			unsigned int* networkSubTypeBit = (unsigned int*)(networkSubType + 0x10);
			int* mediationType = (int*)(_pl0300 + 0xF4);
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
			//*(bool*)(_pl0300 + 0x98) = true;// need to call special func actually
			enable_physics_char_controller(val);
			if (_networkBBUpdateNetworkTypeMethod != nullptr)
				_networkBBUpdateNetworkTypeMethod->call(sdk::get_thread_context(), (REManagedObject*)_pl0300);
			/*if (_networkBBUpdateMethod != nullptr)
				_networkBBUpdateMethod->call(sdk::get_thread_context(), (REManagedObject*)_pl0300);*/
		}

		inline int get_cur_network_type() const noexcept { return *(uintptr_t*)(_pl0300 + 0x54); }

		inline int get_cur_mediation_type() const noexcept { return *(uintptr_t*)(_pl0300 + 0xF4); }

		void set_air_raid_settings(const AirRaidSettings& settings, gf::Vec3 centerOfFloor, float workRateOnHit = 1.0F)
		{
			auto emParam = *(uintptr_t*)(_pl0300 + 0x1768);
			if (emParam == 0)
				return;
			auto airRaidParam = *(uintptr_t*)(emParam + 0x108);
			if (airRaidParam == 0)
				return;
			*(gf::Vec3*)(emParam + 0xA0) = centerOfFloor;
			*(float*)(airRaidParam + 0x70) = settings.radiusOfArea;
			*(float*)(airRaidParam + 0x10) = *(float*)(airRaidParam + 0x14) = settings.secConcentrate;
			*(float*)(airRaidParam + 0x74) = settings.radiusOfRevolution;
			*(float*)(airRaidParam + 0x78) = settings.heightOfArenaSide;
			*(float*)(airRaidParam + 0x7C) = settings.heightOnOutside;
			*(float*)(airRaidParam + 0x80) = settings.distanceCheckGround;
			*(float*)(airRaidParam + 0x50) = settings.radiusFinishAttack;
			*(int*)(airRaidParam + 0x2C) = settings.attackNum;
			*(bool*)(airRaidParam + 0x90) = settings.useSummonedSwords;
			*(bool*)(airRaidParam + 0xA0) = settings.useOptionalTrick;

			auto workRate = *(uintptr_t*)(airRaidParam + 0x40);
			if (workRate != 0)
				*(float*)(workRate + 0x18) = workRateOnHit;
		}

		inline void pl_manager_request_add()
		{
			if (_isInPlList)
				return;
			if (!_isStarted)
			{
				_isAddToPlListRequested = true;
				return;
			}
			if(_playerManager != nullptr && !_isInPlList)
				sdk::call_object_func_easy<void*>(_playerManager, "addPlayer(app.Player)", (REManagedObject*)_pl0300);
			_isInPlList = true;
		}

		inline void goto_wait(const wchar_t *actionName1 = L"", const wchar_t* actionName2 = L"", float interpolationFrame = 0.0f, bool forceInterpolationFrameZero = true, bool puppetTransition = true)
		{
			if (_plGotoWaitMethod == nullptr)
				return;
			gf::SysString str1(actionName1);
			gf::SysString str2(actionName2);
			_plGotoWaitMethod->call(sdk::get_thread_context(), _pl0300, str1.get_net_str(), str2.get_net_str(), interpolationFrame, forceInterpolationFrameZero, puppetTransition);
		}

		inline void pl0300_reset_status()
		{
			if (_plResetStatusMethod == nullptr)
				return;
			_plResetStatusMethod->call(sdk::get_thread_context(), _pl0300);
		}
	};
}