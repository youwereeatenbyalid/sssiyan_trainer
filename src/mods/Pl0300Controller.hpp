#pragma once
#include "PlController.hpp"
#include "events/Events.hpp"

namespace gf = GameFunctions;

namespace PlCntr
{
	namespace Pl0300Cntr
	{
		enum class Pl0300Type
		{
			Em6000Friendly,//Regular pl0300 with m21 AI
			PlHelper //Pl0300 with (optional) main player's pad input and special settings for using it as playable character
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

		class Pl0300Controller : public PlController
		{
		private:

			friend class Pl0300ControllerManager;
			friend class std::allocator<Pl0300ControllerManager>;

			Pl0300Type _pl0300Type;

			bool _isIgnorePadInputRestoring = false;
			bool _isStarted = false;
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
			bool _isIgnoringReleaseOnTrainingReset = false;
			bool _isDoppelDestroyRequested = false;
			bool _isExCostume;

			static inline bool _isStaticInitRequested = true;

			int _requestedJcNum = 1;

			DT _requestedDT = DT::Human;

			TeleportTimingParams _teleportTimingParamsRequested;

			static inline sdk::REMethodDefinition* _pl0300SetDtMethod = nullptr;
			static inline sdk::REMethodDefinition* _pl0300GenDoppelMethod = nullptr;
			static inline sdk::REMethodDefinition* _pl0300DestroyDoppelMethod = nullptr;
			static inline sdk::REMethodDefinition* _pl0300RequestDestroyDoppelMethod = nullptr;
			static inline sdk::REMethodDefinition* _pl0300DoStartMethod = nullptr;
			static inline sdk::REMethodDefinition* _baseBehaviorStartMethod = nullptr;
			static inline sdk::REMethodDefinition* _pl0300startAirRaidMethod = nullptr;
			static inline sdk::REMethodDefinition* _pl0300SetActionMethod = nullptr;
			static inline sdk::REMethodDefinition* _pl0300SetActionFromThinkMethod = nullptr;
			static inline sdk::REMethodDefinition* _pl0300UpdateEmTeleportMethod = nullptr;

			std::weak_ptr<Pl0300Controller> _doppel;
			std::weak_ptr<Pl0300Controller> _owner;

			static inline Pl0300ControllerManager* _pl0300Manager = nullptr;

			HitControllerSettings _hcLastSettings;

			Events::Event<const Pl0300Controller*> _onDestroyEvent;

			//static inline const wchar_t* _jceStr = L"Zigenzan_Zetsu_Start";//I just want to keep this move's name here

			Pl0300Controller(uintptr_t pl0300, Pl0300Cntr::Pl0300Type type, bool exCostume, bool isKeepingOriginalPadInput = false);

			void check_doppel_ref_correct();

		public:

			Pl0300Controller() = delete;
			Pl0300Controller(const Pl0300Controller& other) = delete;

			~Pl0300Controller() override;

			template<class T>
			void on_destroy_event_sub(std::shared_ptr<Events::EventHandler<T, const Pl0300Controller*>> eh)
			{
				_onDestroyEvent.subscribe(eh);
			}

			template<class T>
			void on_destroy_event_unsub(std::shared_ptr<Events::EventHandler<T, const Pl0300Controller*>> eh)
			{
				_onDestroyEvent.unsubscribe(eh);
			}

			//get doppel's controller if exists
			inline std::weak_ptr<Pl0300Controller> get_doppel_ctrl() { return _doppel; }

			//get doppel's owner controller
			inline std::weak_ptr<Pl0300Controller> get_owner_ctrl() { return _owner; }

			inline void set_center_floor(gf::Vec3 pos)
			{
				auto emParam = *(uintptr_t*)(get_pl() + 0x1768);
				if (emParam == 0)
					return;
				*(gf::Vec3*)(emParam + 0xA0) = pos;
			}

			inline bool set_dt(DT dt, bool isNotProduction = false) override
			{
				if (!_isStarted)
				{
					_isSetDtRequested = true;
					_requestedDT = dt;
					return false;
				}
				if (_pl0300SetDtMethod != nullptr && !is_doppel())
					return _pl0300SetDtMethod->call<bool>(sdk::get_thread_context(), (REManagedObject*)get_pl(), (int)dt, isNotProduction);
				else
					*(int*)(get_pl() + 0x9B0) = (int)dt;
				return false;
			}

			inline bool is_enemy() const { return *(bool*)(get_pl() + 0x17E0); }

			//Is GameModel's Start() function called;
			inline bool is_started() const noexcept { return _isStarted; }

			inline bool is_done_air_raid() const { return *(bool*)(get_pl() + 0x1BB2); }

			inline bool is_keeping_original_pad_input() const noexcept { return _isKeepingOriginalPadInput; }

			inline bool is_ignoring_training_reset() const noexcept { return _isIgnoringReleaseOnTrainingReset; }

			void ignore_release_on_trainig_reset(bool val) noexcept { _isIgnoringReleaseOnTrainingReset = val; }

			inline bool is_doppel_destroy_requested() const noexcept { return _isDoppelDestroyRequested; }

			inline bool is_ex_costume() const noexcept { return _isExCostume; }

			inline void set_is_no_die(bool val) noexcept override
			{
				if (!_isStarted)
				{
					_isSetIsNoDieRequested = true;
					_setIsNoDieRequestedVal = val;
				}
				PlController::set_is_no_die(val);
			}

			std::optional<TeleportTimingParams> get_teleport_timing_params() const noexcept
			{
				auto emParam = *(uintptr_t*)(get_pl() + 0x1768);
				if (emParam == 0)
					return std::nullopt;
				auto teleportParam = *(uintptr_t*)(emParam + 0xE0);
				if (teleportParam == 0)
					return std::nullopt;
				return std::make_optional<TeleportTimingParams>(*(TeleportTimingParams*)(teleportParam + 0x60));
			}

			TeleportTimingParams* get_p_teleport_timing_params() noexcept
			{
				auto emParam = *(uintptr_t*)(get_pl() + 0x1768);
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
				auto emParam = *(uintptr_t*)(get_pl() + 0x1768);
				if (emParam == 0)
					return;
				auto teleportParam = *(uintptr_t*)(emParam + 0xE0);
				if (teleportParam == 0)
					return;
				*(TeleportTimingParams*)(teleportParam + 0x60) = params;
			}

			inline std::optional<bool> is_em_step_enabled() const noexcept
			{
				auto plData = *(uintptr_t*)(get_pl() + 0xD20);
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
				auto plData = *(uintptr_t*)(get_pl() + 0xD20);
				auto saveData = *(uintptr_t*)(get_pl() + 0x1F10);
				if (plData != 0 && saveData != 0)
					*(bool*)(plData + 0x19) = *(bool*)(saveData + 0x19) = val;
			}

			inline int get_jcut_num() const noexcept { return *(int*)(get_pl() + 0x1F5C); }

			inline void set_jcut_num(int num) noexcept
			{
				if (!_isStarted)
				{
					_isJcNumSetRequested = true;
					_requestedJcNum = num;
					return;
				}
				*(int*)(get_pl() + 0x1F5C) = num;
			}

			inline void set_is_enemy(bool isEm) noexcept { *(bool*)(get_pl() + 0x17E0) = isEm; }

			inline void pl0300_do_start()
			{
				if (_pl0300DoStartMethod != nullptr)
					_pl0300DoStartMethod->call(sdk::get_thread_context(), get_pl());
			}

			inline Pl0300Type get_pl0300_type() const noexcept { return _pl0300Type; }

			inline bool is_doppels_owner(uintptr_t pl0300Doppel)
			{
				if (pl0300Doppel == 0)
					return false;
				return *(uintptr_t*)(get_pl() + 0x1C10) == pl0300Doppel;
			}

			bool is_doppel() const noexcept { return *(bool*)(get_pl() + 0x1F60); }

			void set_action(const wchar_t* actionStr, int layerNo = 0, float startFrame = 0.0f, float interpolationFrame = 0.0f, InterpolationMode mode = InterpolationMode::SyncCrossFade,
				InterpolationCurve curve = InterpolationCurve::Smooth, bool isImmediate = false, bool passSelect = true, bool isPuppetTransition = false,
				ActionPriority actionPriority = ActionPriority::Normal) const noexcept override
			{
				if (_pl0300SetActionMethod != nullptr)
				{
					gf::SysString str(actionStr);
					_pl0300SetActionMethod->call(sdk::get_thread_context(), (REManagedObject*)get_pl(), str.get_net_str(), layerNo, startFrame, interpolationFrame, mode, curve, isImmediate,
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
					_pl0300SetActionMethod->call(sdk::get_thread_context(), (REManagedObject*)get_pl(), actionStr->get_net_str(), layerNo, startFrame, interpolationFrame, mode, curve, isImmediate,
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
					_pl0300SetActionFromThinkMethod->call(sdk::get_thread_context(), (REManagedObject*)get_pl(), action->get_net_str(), hash);
			}

			void update_em_teleport(bool isFirst = false)
			{
				if (_pl0300UpdateEmTeleportMethod != nullptr)
					_pl0300UpdateEmTeleportMethod->call(sdk::get_thread_context(), get_pl(), isFirst);
			}

			void set_hitcontroller_settings(HitControllerSettings settings) override
			{
				_hcLastSettings = settings;
				if (!_isStarted)
				{
					_isHitCtrlDataSetRequested = true;
					return;
				}
				uintptr_t yamatoHitCtrl = 0;
				auto yamatoWeapon = *(uintptr_t*)(get_pl() + 0x2018);
				if (yamatoWeapon != 0)
					yamatoHitCtrl = *(uintptr_t*)(yamatoWeapon + 0x1F8);
				if (yamatoHitCtrl != 0)
				{
					*(float*)(yamatoHitCtrl + 0x188) = settings.baseAttackRate;
					*(float*)(yamatoHitCtrl + 0x1A8) = settings.baseDmgReactionRate;
					*(bool*)(yamatoHitCtrl + 0x1DF) = settings.isAttackNoDie;
				}
				PlController::set_hitcontroller_settings(settings);
			}

			void generate_doppel(bool isFirst, float hp = 250.0f, float attackRate = 0.35f);

			void destroy_doppel();

			uintptr_t get_doppel() const noexcept { return *(uintptr_t*)(get_pl() + 0x1C10); }

			void set_air_raid_settings(const AirRaidSettings& settings, gf::Vec3 centerOfFloor, float workRateOnHit = 1.0F)
			{
				auto emParam = *(uintptr_t*)(get_pl() + 0x1768);
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
				if (!_isStarted)
				{
					_isAddToPlListRequested = true;
					return;
				}
				auto plMngr = get_pl_manager();
				if (plMngr != nullptr)
					sdk::call_object_func_easy<void*>(plMngr, "addPlayer(app.Player)", (REManagedObject*)get_pl());
			}
		};
	}
}
