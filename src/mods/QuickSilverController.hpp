#pragma once
#include "mods/PrefabFactory/PrefabFactory.hpp"
#include "Mod.hpp"
#include "GameFunctions/CreateShell.hpp"
#include "GameFunctions/GameModelRequestSetEffect.hpp"
#include "PlayerTracker.hpp"
#include "Coroutine/Coroutines.hpp"
#include "GameplayStateTracker.hpp"

namespace QuickSilverCtrl
{
	class QuickSilverControllerBase
	{
	private:
		gf::CreateShell _createShell;
		
		std::unique_ptr<gf::SysString> _pathStr = nullptr;

		uintptr_t _owner = 0;

		static inline sdk::REMethodDefinition* _gameObjDestroyMethod = nullptr;

		//QS shell pfb to spawn via shellManager;
		REManagedObject* _shellPfb = nullptr;
		//Backup of basic shell if you putt smth else to _shellPfb;
		REManagedObject* _shellPfbBackup = nullptr;

	protected:

		//Shell that was spawned by create_slow_time_shell(...);
		REManagedObject* _curQuickSilverShell = nullptr;
		
		//ptr to app.Shell.lifeTime
		volatile float* _shellCurLifeTime = nullptr;
		//ptr to shell life time max
		volatile float* _shellLifeTime = nullptr;

		//do some initial setup on special shell;
		virtual void setup_slow_shell(REManagedObject* shell) = 0;

		void destroy_game_obj(REManagedObject* shell);

		//Use this to set main qs pfb to spawn. If you have a few pfb, prev can be accessed via get_shell_pfb_backup();
		void set_shell_pfb(REManagedObject* shell);

		REManagedObject* get_shell_pfb_backup() const noexcept { return _shellPfbBackup; }

	public:
		QuickSilverControllerBase() = delete;
		QuickSilverControllerBase(const wchar_t* pfbPath);

		virtual ~QuickSilverControllerBase();

		//calls set_owner(uintptr_t) and create_slow_time_shell(gf::Vec3, gf::Quaternion);
		REManagedObject* create_slow_time_shell(gf::Vec3 pos, gf::Quaternion rot, uintptr_t owner);

		virtual REManagedObject* create_slow_time_shell(gf::Vec3 pos, gf::Quaternion rot);

		//Get last shell that was spawned and handled by create_slow_time_shell();
		REManagedObject* get_cur_quicksilver_shell() const noexcept { return _curQuickSilverShell; }

		//Get shell's character owner
		inline uintptr_t get_owner() const noexcept { return _owner; }

		//Set shell's character owner
		inline void set_owner(uintptr_t owner) noexcept { _owner = owner; }
	};
	/// <summary>
	/// Controller for Slow World Shell. Relies on Events from PlayerTracker to function correctly, any mod using this must add PlayerTracker as a dependency.
	/// </summary>
	class QuickSilverSlowWorldController : public virtual QuickSilverControllerBase
	{
	public:
		enum class SlowWorldType
		{
			Slow,
			StopOnSlowPfb,
			StopPfb
		};

	private:
		static constexpr wchar_t* _wcPfbWorldSlowPath = L"Prefab/Character/Player/Shell/shl00_011_qs_worldslow.pfb";
		static constexpr wchar_t* _wcPfbWorldStopPath = L"Prefab/Character/Player/Shell/shl00_011_qs_worldstop.pfb";

		//Nero's slow world/stop world shells are uses special fsm methods, there are hooked in PlayerTracker
		std::shared_ptr<Events::EventHandler<QuickSilverSlowWorldController, uintptr_t, uintptr_t, uintptr_t>> _slowWorldActionEventHandler;
		std::shared_ptr<Events::EventHandler<QuickSilverSlowWorldController, uintptr_t, uintptr_t, uintptr_t>> _stopWorldActionEventHandler;

		static inline std::shared_ptr<gf::GameModelRequestSetEffect::EffectID> _slowWorldStartEfx = nullptr;

		static inline REManagedObject* _worldStopShellPfb = nullptr;
		REManagedObject* _curFsmAction = nullptr;

		static inline std::unique_ptr<gf::SysString> _pfbWorldStopPath = nullptr; 

		SlowWorldType _slowWorldType = SlowWorldType::Slow;

		bool _isDtConsume = false;
		bool _isStarted = false;

		volatile float *_dtGauge = nullptr;

		float _dtConsumption = 500.0f;
		float _dtMinLimit = 3000.0f;

		void after_pl0000_slow_world_action_start(uintptr_t threadCtxt, uintptr_t fsmSlowWorld, uintptr_t behaciorTreeArg);

		void after_pl0000_stop_world_action_start(uintptr_t threadCtxt, uintptr_t fsmStopWorld, uintptr_t behaciorTreeArg);

		void update_quicksilver_world_behavior();

		void update_quicksilver_dt_consume();

		Coroutines::Coroutine<decltype(&QuickSilverSlowWorldController::update_quicksilver_world_behavior), QuickSilverSlowWorldController*> 
			_updateQuickSilverCoroutine{ &QuickSilverSlowWorldController::update_quicksilver_world_behavior };

		Coroutines::Coroutine<decltype(&QuickSilverSlowWorldController::update_quicksilver_world_behavior), 
			QuickSilverSlowWorldController*> _updateQuickSilverDTCoroutine{ &QuickSilverSlowWorldController::update_quicksilver_dt_consume, true };

	public:
		QuickSilverSlowWorldController() : QuickSilverControllerBase(_wcPfbWorldSlowPath)
		{
			if (_pfbWorldStopPath == nullptr)
				_pfbWorldStopPath = std::make_unique<gf::SysString>(_wcPfbWorldStopPath);

			_slowWorldActionEventHandler = std::make_shared<Events::EventHandler<QuickSilverSlowWorldController, uintptr_t, uintptr_t, uintptr_t>>
				(this, &QuickSilverSlowWorldController::after_pl0000_slow_world_action_start);
			_stopWorldActionEventHandler = std::make_shared<Events::EventHandler<QuickSilverSlowWorldController, uintptr_t, uintptr_t, uintptr_t>>
				(this, &QuickSilverSlowWorldController::after_pl0000_stop_world_action_start);

			_worldStopShellPfb = PfbFactory::PrefabFactory::create_prefab(_pfbWorldStopPath.get());
			_updateQuickSilverCoroutine.ignoring_update_on_pause(true);
			_updateQuickSilverCoroutine.set_delay(0);

			_updateQuickSilverDTCoroutine.ignoring_update_on_pause(true);
			_updateQuickSilverDTCoroutine.set_delay(225.0f);
		}

		~QuickSilverSlowWorldController() override;

		void setup_slow_shell(REManagedObject* shell) override;

		REManagedObject* create_slow_time_shell(gf::Vec3 pos, gf::Quaternion rot) override;

		void stop_quicksilver();

		REManagedObject* get_cur_fsm_slow_world() const noexcept { return _curFsmAction; }

		SlowWorldType get_slow_world_type() const noexcept { return _slowWorldType; }

		void set_slow_world_type(SlowWorldType type) noexcept { _slowWorldType = type; }

		bool get_is_dt_consume() const noexcept { return _isDtConsume; }

		void set_is_dt_consume(bool val) noexcept { _isDtConsume = val; }

		volatile float* get_dt_gauge() const noexcept { return _dtGauge; }

		void set_dt_gauge_ptr(volatile float* dtGauge) { _dtGauge = dtGauge; }

		float get_dt_consumption() const noexcept { return _dtConsumption; }

		void set_dt_consumption(float dtConsumption) noexcept { _dtConsumption = dtConsumption; }

		float get_dt_consumption_update_frequency() const noexcept { return _updateQuickSilverDTCoroutine.get_delay(); }

		void set_dt_consumption_update_frequency(float val) noexcept { _updateQuickSilverDTCoroutine.set_delay(val); }

		bool is_started() const noexcept { return _isStarted; }

		void set_dt_min_limit(float val) noexcept { _dtMinLimit = val; }

		float get_dt_min_limit() const noexcept { return _dtMinLimit; }

		static inline constexpr wchar_t* const get_world_slow_pfb_path() noexcept { return _wcPfbWorldSlowPath; }

		static inline constexpr wchar_t* const get_world_stop_pfb_path() noexcept { return _wcPfbWorldStopPath; }

		//Create some cool timer & time wave efx like Nero do with rigtime.
		static uintptr_t request_slow_world_efx(uintptr_t slowWorldGameModel, gf::Vec3 pos, gf::Quaternion rot)
		{
			if (_slowWorldStartEfx == nullptr)
				_slowWorldStartEfx = std::make_shared<gf::GameModelRequestSetEffect::EffectID>(-1, 0, 0, false, false, false);
			gf::GameModelRequestSetEffect efx{ slowWorldGameModel, _slowWorldStartEfx };
			return (uintptr_t)efx(pos, rot);
		}
	};
}