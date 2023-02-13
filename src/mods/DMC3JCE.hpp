#pragma once
#include "Mod.hpp"
#include <random>
#include "CheckpointPos.hpp"
#include "mods/GameFunctions/CreateShell.hpp"
#include <condition_variable>
#include "VergilSDTFormTracker.hpp"
#include "PlayerTracker.hpp"
#include "mods/GameFunctions/PlayerSetDT.hpp"
#include "VergilInstantSDT.hpp"
#include "mods/GameFunctions/GameModelRequestSetEffect.hpp"
#include "GameplayStateTracker.hpp"
#include "EndLvlHooks.hpp"
#include "PlSetActionData.hpp"
#include "InputSystem.hpp"
#include "Mods.hpp"
#include "events/EventHandler.hpp"
#include "mods/Coroutine/Coroutines.hpp"
#include "EnemyData.hpp"

//clang-format off
namespace func = GameFunctions;


class DMC3JCE : public Mod, private EndLvlHooks::IEndLvl
{
public:
	enum AutoSDTType
	{
		None,
		LessEfx,
		SDTBurst
	};

	class JCEController : private EndLvlHooks::IEndLvl
	{

	public:
		enum Type
		{
			Random,
			Track,
			Dynamic
		};

	private:
		void execute_3jce(Type jceType, uintptr_t workRateSys)
		{
			if (!can_execute())
			{
				return;
			}
			if (_jcPfb == 0)
			{
				stop_jce(true);
				return;
			}
			uintptr_t jcShell = (uintptr_t)_jcSpawn(_curJcPos, _defaultJcRot);
			switch (jceType)
			{
				case DMC3JCE::JCEController::Random:
				{
					if (jcShell == 0)
					{
						stop_jce(true);
						return;
					}
					*(bool*)((uintptr_t)jcShell + 0x440) = _isRndJust; //isJust
					*(float*)((uintptr_t)jcShell + 0x444) = _rndAttackRate; //attackRate
					rndjc_pos_update();
					break;
				}
				case DMC3JCE::JCEController::Track:
				{
					if (jcShell == 0)
					{
						stop_jce(true);
						return;
					}
					*(bool*)((uintptr_t)jcShell + 0x440) = _isTrackJust; //isJust
					*(float*)((uintptr_t)jcShell + 0x444) = _trackAttackRate; //attackRate
					track_pos_update();
					break;
				}
				default:
				{
					stop_jce(true);
					break;
				}
			}
			_3jceCoroutine->set_delay(update_delay(workRateSys));
		}

		using ActionType = decltype(&JCEController::execute_3jce);

		std::string_view _errorData;

		InputSystem *_inputSystem;

		Type jceType;

		std::unique_ptr<Coroutines::Coroutine<ActionType, JCEController*, Type, uintptr_t>> _3jceCoroutine = nullptr;

		func::Vec3 maxOffset;
		func::Vec3 minOffset;
		func::Vec3 moveTrackStepOffs;
		gf::Vec3 _prevJcPos;
		gf::Vec3 _curJcPos;
		gf::Vec3 _pl0800Pos;

		std::uniform_real_distribution<float> _xDist;
		std::uniform_real_distribution<float> _yDist;
		std::uniform_real_distribution<float> _zDist;
		std::uniform_real_distribution<float> _trackZDist{ -0.95f, 0.95f };

		gf::Quaternion _defaultJcRot;

		std::mt19937 _rndGen;

		func::CreateShell _jcSpawn;

		uintptr_t _jcPfb = 0;
		uintptr_t _pl0800 = 0;

		int _rndDelayTime = defaultRndDelay; //fancy coroutine time
		int _trackDelayTime = defaultTrackDelay; //fancy coroutine time
		int _rndJcSpawnCount = 0;
		int _tmpDelay = 0;

		float _curExecutionTime;

		const float _rndZTrackOffs = 0.82f;
		const float _rndEmMaxDist = 34.5f;
		const float _rndAttackRate = 2.5f;
		const float _trackAttackRate = 1.2f;

		bool _isInJceSdt = false;
		bool _isStartJCE = false;
		const bool _isRndJust = true;
		const bool _isTrackJust = false;

		static inline std::shared_ptr<func::GameModelRequestSetEffect::EffectID> _sdtBurstEffect = nullptr;

		void reset(EndLvlHooks::EndType end) override
		{
			stop_jce(true);
			_isInJceSdt = false;
			_jcPfb = 0;
			_pl0800 = 0;
		}

		void set_sdt(GameFunctions::PlVergilSetDT::DevilTrigger dtState)
		{
			GameFunctions::PlVergilSetDT setDt{ _pl0800 };
			std::array<uintptr_t, 1> offs { 0x1B20 };
			auto curSdtOp = func::PtrController::get_ptr_val<float>(_pl0800, offs, true);
			if(!curSdtOp)
				return;
			float curSdt = curSdtOp.value();
			func::PtrController::try_to_write(_pl0800, offs, 10000.0f, true);
			bool isInstantSdtOn = VergilInstantSDT::cheaton;
			VergilInstantSDT::cheaton = true;
			setDt(dtState, false);
			func::PtrController::try_to_write<float>(_pl0800, offs, curSdt, true);
			VergilInstantSDT::cheaton = isInstantSdtOn;
			if (dtState == gf::PlVergilSetDT::DevilTrigger::SDT)
				_isInJceSdt = true;
			else
				_isInJceSdt = false;
		}

		func::Vec3 get_lockon_pos(bool& isNullPtr, func::Quaternion &rot)
		{
			uintptr_t lockOnObj = *(uintptr_t*)(_pl0800 + 0x428);
			if (lockOnObj == 0 /*IsBadReadPtr((void*)lockOnObj, 0x8)*/)
			{
				isNullPtr = true;
				return func::Vec3(0, 0, 0);
			}
			isNullPtr = false;
			auto lockOnTarget = *(uintptr_t*)(lockOnObj + 0x10);//LockOnTargetWork target
			auto transform = *(uintptr_t*)(lockOnTarget + 0x50);//cachedTransform
			rot = *(func::Quaternion*)(transform + 0x40);
			auto cachedCharacter = *(uintptr_t*)(lockOnTarget + 0x58);
			if (cachedCharacter != 0 && EnemyData::get_em_id(cachedCharacter) == EnemyData::Behemoth && gf::StringController::str_cmp(*(uintptr_t*)(cachedCharacter + 0x190), L"HideChase"))
				return *(gf::Vec3*)(transform + 0x30);
			return _getLockOnPosMethod->call<gf::Vec3>(sdk::get_thread_context(), lockOnObj);
		}

		inline void set_pl0800(uintptr_t pl0800)
		{
			_pl0800 = pl0800;
			auto gameObj = *(uintptr_t*)(_pl0800 + 0x10);
			auto transform = *(uintptr_t*)(gameObj + 0x18);
			_pl0800Pos = *(gf::Vec3*)(transform + 0x30);
			_jcPfb = get_jc_pfb(_pl0800);
		}

		void track_pos_update()
		{
			func::Vec3 lockOnPos;
			bool isNoLockOn = false;
			lockOnPos = get_lockon_pos(isNoLockOn, _defaultJcRot);
			if(isNoLockOn)
				return;
			float length = func::Vec3::vec_length(_curJcPos, lockOnPos);
			float acc = 0.0f;
			if(length >= 10.4f)
				acc = 2.3f;
			if(abs(lockOnPos.x - _curJcPos.x) >= 2.3f)
				_curJcPos.x = lockOnPos.x >= _curJcPos.x ? _curJcPos.x + moveTrackStepOffs.x + acc : _curJcPos.x - moveTrackStepOffs.x - acc;
			else
				_curJcPos.x = lockOnPos.x + _trackZDist(_rndGen);
			if (abs(lockOnPos.y - _curJcPos.y) >= 2.3f)
				_curJcPos.y = lockOnPos.y >= _curJcPos.y ? _curJcPos.y + moveTrackStepOffs.y + acc : _curJcPos.y - moveTrackStepOffs.y - acc;
			else
				_curJcPos.y = lockOnPos.y + +_trackZDist(_rndGen);
			if (abs(lockOnPos.z - _curJcPos.z) >= 2.1f)
				_curJcPos.z = lockOnPos.z >= _curJcPos.z ? _curJcPos.z + moveTrackStepOffs.z : _curJcPos.z - moveTrackStepOffs.z;
			else
				_curJcPos.z = lockOnPos.z + +_trackZDist(_rndGen);
		}

		void rndjc_pos_update()
		{
			bool isNull = false;
			auto lockOnPos = get_lockon_pos(isNull, _defaultJcRot);
			if (_rndJcSpawnCount >= rndEmTrackInterval)
			{
				_curJcPos = isNull ? _pl0800Pos : lockOnPos;
				_rndJcSpawnCount = 0;
			}
			else
			{
				if (isNull)
				{
					_curJcPos.x = _pl0800Pos.x + _xDist(_rndGen);
					_curJcPos.y = _pl0800Pos.y + _yDist(_rndGen);
					_curJcPos.z = _pl0800Pos.z + _zDist(_rndGen);
				}
				else
				{
					_curJcPos.x = lockOnPos.x + _xDist(_rndGen);
					_curJcPos.y = lockOnPos.y + _yDist(_rndGen);
					_curJcPos.z = lockOnPos.z + _zDist(_rndGen);
				}
			}
			_rndJcSpawnCount++;
		}

		bool can_execute()
		{
			if (GameplayStateTracker::nowFlow == 0x16 && !GameplayStateTracker::isCutscene && !GameplayStateTracker::isExecutePause)
			{
				return true;
			}
			else return false;
		}

		int update_delay(uintptr_t workRateSys)
		{
			if(workRateSys == 0 || *(uintptr_t*)(workRateSys + 0x58) == 0)
				return _tmpDelay;
			uintptr_t workRateData = *(uintptr_t*)(workRateSys + 0x58);
			float rate = *(float*)(workRateData + 0x18);
			if(rate == 0)
				return _tmpDelay;
			return (float)_tmpDelay / rate;
		}

		uintptr_t get_jc_pfb(uintptr_t pl0800) const noexcept
		{
			auto weaponContainer = *(uintptr_t*)(pl0800 + 0x1970);
			if (weaponContainer == 0)
				return 0;
			auto dictionaryContainer = *(uintptr_t*)(weaponContainer + 0x18);
			auto weaponData = *(uintptr_t*)(dictionaryContainer + 0x30);
			if (weaponData == 0)
				return 0;
			auto weaponYamato = *(uintptr_t*)(weaponData + 0x10);
			if (weaponYamato == 0)
				return 0;
			auto yamatoCommonParam = *(uintptr_t*)(weaponYamato + 0x300);
			if (yamatoCommonParam == 0)
				return 0;
			auto yamatoPlShellData = *(uintptr_t*)(yamatoCommonParam + 0x38);
			if (yamatoPlShellData == 0)
				return 0;
			return *(uintptr_t*)(yamatoPlShellData + 0x10);
		}

		bool setup_to_start(uintptr_t pl0800, Type jcType) noexcept
		{
			_rndJcSpawnCount = 0;
			if (_pl0800 == 0)
				return false;
			set_pl0800(pl0800);
			if (_jcPfb == 0)
			{
				stop_jce(true);
				return false;
			}

			switch (jcType)
			{
				case Track:
				{
					bool lockOnNull;
					gf::Quaternion emRot;
					auto emPos = get_lockon_pos(lockOnNull, emRot);
					if (!lockOnNull)
					{
						_curJcPos.x = _prevJcPos.x = (emPos.x + _pl0800Pos.x) / 2.0f;
						_curJcPos.y = _prevJcPos.y = (emPos.y + _pl0800Pos.y) / 2.0f;
					}
					else
						_curJcPos = _pl0800Pos;
					break;
				}
				case Random:
				{
					rndjc_pos_update();
					break;
				}
			}

			_jcSpawn.set_params(_jcPfb, _prevJcPos, _defaultJcRot, _pl0800, 0, 0); //1, 105
			return true;
		}

	public:
		const int defaultRndDelay = 120;
		const int defaultTrackDelay = 187;

		int rndEmTrackInterval = 22;

		const float rndExeTimeModDefault = 6.8f;
		const float trackExeTimeModDefault = 5.95f;

		float rndExeTime = rndExeTimeModDefault;
		float trackExeTime = trackExeTimeModDefault;

		JCEController()
		{
			maxOffset.x = 10.7f;
			maxOffset.y = 10.6;
			maxOffset.z = 4.21f;
			minOffset.x = -10.7f;
			minOffset.y = -10.6f;
			minOffset.z = -0.85f;

			moveTrackStepOffs.x = 1.18f;
			moveTrackStepOffs.y = 1.18F;
			moveTrackStepOffs.z = 0.98;

			_xDist = std::uniform_real_distribution<float>(minOffset.x, maxOffset.x);
			_yDist = std::uniform_real_distribution<float>(minOffset.y, maxOffset.y);
			_zDist = std::uniform_real_distribution<float>(minOffset.z, maxOffset.z);
			_curExecutionTime = rndExeTimeModDefault;
			auto base = g_framework->get_module().as<uintptr_t>();
			_inputSystem = (InputSystem*)g_framework->get_mods()->get_mod("InputSystem");
			_sdtBurstEffect = std::make_shared<func::GameModelRequestSetEffect::EffectID>(3, 6);
			_3jceCoroutine = std::make_unique<Coroutines::Coroutine<ActionType, JCEController*, Type, uintptr_t>>
				(&JCEController::execute_3jce, true);
			_3jceCoroutine->ignoring_update_on_pause(true);
		}

		~JCEController()
		{
			//PlSetActionData::new_action_event_unsub(new Events::EventHandler<JCEController, const std::array<char, PlSetActionData::ACTION_STR_LENGTH>&, uintptr_t>(this, &JCEController::on_action_update));
		}

		bool request_set_jce_dt(uintptr_t plVergil, GameFunctions::PlVergilSetDT::DevilTrigger dtState, DMC3JCE::AutoSDTType efxType = LessEfx)
		{
			set_pl0800(plVergil);
			if (dtState == GameFunctions::PlVergilSetDT::DevilTrigger::Devil)
				return false;
			if (dtState == GameFunctions::PlVergilSetDT::DevilTrigger::SDT && _isInJceSdt)
				return false;
			if (dtState == GameFunctions::PlVergilSetDT::DevilTrigger::Human && !_isInJceSdt)
				return false;
			set_sdt(dtState);
			if (autoSdtType == SDTBurst && dtState == GameFunctions::PlVergilSetDT::DevilTrigger::SDT)
			{
				func::GameModelRequestSetEffect setBurst{ plVergil, _sdtBurstEffect };
				auto transform = *(uintptr_t*)(plVergil + 0x10);
				transform = *(uintptr_t*)(transform + 0x18);
				setBurst(*(func::Vec3*)(transform + 0x30), *(func::Quaternion*)(transform + 0x40));
			}
			return true;
		}

		void force_jcesdt_health_update(uintptr_t vergil)
		{
			if (!_isInJceSdt)
				return;
			*(float*)(vergil + 0x1B28) = -0.1f;
		}

		volatile void start_jce(uintptr_t pl0800)
		{
			if (is_executing())
				return;
			Type tmp = get_jce_type();
			if (tmp == Dynamic)
			{
				if (_inputSystem->is_action_button_pressed(InputSystem::PadInputGameAction::AttackS))
					tmp = Track;
				else
					tmp = Random;
			}
			if (!setup_to_start(pl0800, tmp))
			{
				stop_jce(true);
				return;
			}
			uintptr_t workRateSys = (uintptr_t)reframework::get_globals()->get("app.WorkRateSystem");
			switch (tmp)
			{
				case Random:
				{
					_curExecutionTime = rndExeTime;
					_3jceCoroutine->set_delay(rndDelay);
					break;
				}
				case Track:
				{
					_curExecutionTime = trackExeTime;
					_3jceCoroutine->set_delay(trackDelay);
					break;
				}
			}
			_tmpDelay = _3jceCoroutine->get_delay();
			isJceRunning = true;
			_3jceCoroutine->start(this, tmp, workRateSys);
		}

		void stop_jce(bool endSdt = true)
		{
			if (endSdt && _isInJceSdt)
				set_sdt(GameFunctions::PlVergilSetDT::Human);
			if (!is_executing())
				return;
			_3jceCoroutine->stop();
			isJceRunning = false;
			_rndJcSpawnCount = 0;
		}

		bool is_executing() const noexcept
		{
			return _3jceCoroutine->is_started();
		}

		inline void set_rndspawn_delay(int delay) noexcept { _rndDelayTime = delay; }

		inline void set_trackspawn_delay(int delay) noexcept { _trackDelayTime = delay; }

		inline int get_rndspawn_delay() const noexcept { return _rndDelayTime; }

		inline int get_trackspawn_delay() const noexcept { return _trackDelayTime; }

		inline bool is_in_jce_sdt() const noexcept { return _isInJceSdt; }

		void set_jce_type(Type type, bool isDefaultTime = true, float time = 0) 
		{
			jceType = type;
			if (isDefaultTime)
			{
				switch (type)
				{
					case DMC3JCE::JCEController::Random:
					{
						JCEController::_curExecutionTime = rndExeTime;
						break;
					}

					case DMC3JCE::JCEController::Track:
					{
						JCEController::_curExecutionTime = trackExeTime;
						break;
					}
					default:
						break;
				}
			}
			else
				_curExecutionTime = time;
		}

		inline Type get_jce_type() const noexcept {return jceType; }

		inline std::string_view get_error() noexcept { return _errorData; }

		inline float get_current_execution_time() const noexcept { return _curExecutionTime; }
};

	static inline AutoSDTType autoSdtType = LessEfx;

    static inline std::unique_ptr<JCEController> jceController{nullptr};
	static inline bool cheaton = true;
	static inline bool isJceRunning = false;//For tracking this shit in __naked__ without calling funcs
	static inline bool isCrashFixEnabled = true;
	static inline bool isSetCustomCapacity = false;

	static inline uintptr_t canExeJceRet = 0;
	static inline uintptr_t canExeJceRet1 = 0;
	static inline uintptr_t subSdtRet = 0;
	static inline uintptr_t jcePfbRet = 0;
	static inline uintptr_t jcePfb1Ret = 0;
	static inline uintptr_t jcePfbJeJmp = 0;
	static inline uintptr_t jcePfbJneJmp = 0;
	static inline uintptr_t jcePfb2Ret = 0;
	static inline uintptr_t jceTimerRet = 0;
	static inline float jceTimerStaticBase = 0;
	static inline uintptr_t jceFinishPfbRet = 0;

	static inline float humanJCECost = 3000.0f;
	static inline float minSdt = 3000.0f;
	static inline float curJceTimerAsm = 0;

	static inline int rndDelay = 0;
	static inline int trackDelay = 0;
	static inline int prevRayCastSize = 64;
	static inline int curRayCastSize = 64;
	static inline int newCapacity = 256;

	static inline JCEController::Type jcTypeUi{};
	static bool use_default_behaviour_asm();
	static bool can_execute_asm();
	static void end_jcesdt_asm(uintptr_t vergil);

	//PlVergil +1978 - curWeapon;

	DMC3JCE() = default;

	std::string_view get_name() const override
	{
		return "DMC3 JCE";
	}
	std::string get_checkbox_name() override
	{
		return m_check_box_name;
	};
	std::string get_hotkey_name() override
	{
		return m_hot_key_name;
	};

	std::optional<std::string> on_initialize() override;

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override;
	void on_config_save(utility::Config& cfg) override;

	// on_frame() is called every frame regardless whether the gui shows up.
	// void on_frame() override;
	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override;
	// on_draw_debug_ui() is called when debug window shows up
	// void on_draw_debug_ui() override;

	static sdk::REMethodDefinition const* get_lock_on_pos_method() noexcept { return _getLockOnPosMethod; }

private:
	float rndExeDuration = 0;
	float trackExeDuration = 0;
	float _jceTimer = 0;

	std::string _jceControllerErrorStr;

	static inline DMC3JCE *_mod = nullptr;

	static void jce_cancel_hook(uintptr_t threadCtxt, uintptr_t vergil);

	static void pl0800_start_jce_update(uintptr_t threadCtxt, uintptr_t fsmStartJce, uintptr_t actionArg);

	static inline const sdk::REMethodDefinition* _getLockOnPosMethod = nullptr;

	void reset(EndLvlHooks::EndType type) override
	{
		_jceTimer = 0;
	}

	void on_sdk_init() override
	{
		_getLockOnPosMethod = sdk::find_method_definition("app.LockOnObject", "get_lockOnPosition()");
	}

	void after_all_inits() override
	{
		PlayerTracker::pl_add_dt_gauge_sub(std::make_shared<Events::EventHandler<DMC3JCE, uintptr_t, uintptr_t, float*, int, bool>>
			(this, &DMC3JCE::on_pl_add_dt));
	}

	void on_pl_add_dt(uintptr_t threadCtxt, uintptr_t pl, float* val, int dtAddType, bool fixedValue);

	void init_check_box_info() override;

	std::unique_ptr<FunctionHook> m_can_exe_jce_hook;
	std::unique_ptr<FunctionHook> m_can_exe_jce1_hook;
	std::unique_ptr<FunctionHook> m_sub_human_jce_hook;
	std::unique_ptr<FunctionHook> m_jce_timer_hook;
	std::unique_ptr<FunctionHook> m_jce_crashpoint_hook;
	std::unique_ptr<FunctionHook> m_jce_finishpfb_hook;
	std::unique_ptr<FunctionHook> m_jce_prefab2_hook;
	std::unique_ptr<FunctionHook> m_jce_cancel_hook;

	std::unique_ptr<FunctionHook> m_update_jce_hook;
};
//clang-format on

