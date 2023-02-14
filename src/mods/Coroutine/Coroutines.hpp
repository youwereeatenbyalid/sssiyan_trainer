#pragma once
#include <execution>
#include <tuple>
#include <memory>
#include "InitPatternsManager.hpp"
#include "utility/FunctionHook.hpp"
#include "mods/GameplayStateTracker.hpp"
#include "mods/DeepTurbo.hpp"

namespace Coroutines
{
	namespace Impl
	{
		class IDelayedAction
		{
		public:
			virtual void invoke() = 0;
		};

		//Installing hook to main game loop(?) and calls all coroutine from there so thread context is always valid.
		class CoroutineBase
		{
		private:

			//Main engine loop method probably. Calls all coroutines. TC is always valid.
			static void* _sub_1425A9B00_detour(uintptr_t rcx, uintptr_t behaviorGroup)
			{
				run();
				return _sub_1425A9B00_hook->get_original<decltype(_sub_1425A9B00_detour)>()(rcx, behaviorGroup);
			}

			using _subOrig = decltype(&CoroutineBase::_sub_1425A9B00_detour);

			static inline std::vector<CoroutineBase*> _coroutines;

			static inline std::unique_ptr<FunctionHook> _sub_1425A9B00_hook = nullptr;

			//Inc timer for this val on every sub func call;
			static inline const float _timeStep = 0.1f;

			static inline uintptr_t _sub_1425A9B00Addr;

			static inline bool _isHookCreated = false;
			bool _isIgnoringGlobalTurboSpeedSetting = false;

			//Run all coroutines in _sub_1425A9B00(...)
			static void run()
			{
				//Main loop is mt so recursive lock is a must (coroutine can be disabled in action method);
				std::lock_guard<std::recursive_mutex> lock(_mtx);
				for (auto& item : _coroutines)
				{
					if (item->_isIgnoringUpdateOnPause && GameplayStateTracker::isExecutePause)
						continue;
					if (item->_isFirstExe && item->_isInstantStartFirst)
					{
						item->start_action();
						item->_isFirstExe = false;
						item->update_timer();
					}
					else if (item->check_timer())
					{
						item->start_action();
						item->reset_timer();
					}
					else
						item->update_timer();
				}
			}

			static inline void create_hook()
			{
				_sub_1425A9B00_hook = std::make_unique<FunctionHook>(_sub_1425A9B00Addr, &CoroutineBase::_sub_1425A9B00_detour, true);
				_sub_1425A9B00_hook->create();
			}

		protected:

			//Recursive mutex for any operation with coroutine when it's running;
			static inline std::recursive_mutex _mtx{};

			std::shared_ptr<IDelayedAction> _action;

			bool _isStarted = false;
			bool _isFirstExe = false;
			bool _isInstantStartFirst = true;
			bool _isIgnoringUpdateOnPause = false;

			float _timer = 0;
			float _delay = 0;

			static inline bool is_hook_created() noexcept { return _sub_1425A9B00_hook != nullptr; }

			static inline uintptr_t get_sub_f_addr() noexcept { return _sub_1425A9B00Addr; }

			void unregister_coroutine(const CoroutineBase* obj) noexcept
			{
				_coroutines.erase(std::remove(_coroutines.begin(), _coroutines.end(), obj), _coroutines.end());
			}

			void register_coroutine(CoroutineBase* obj) noexcept
			{
				_coroutines.push_back(obj);
				if (!is_hook_created())
					create_hook();
			}

			void reset_timer()
			{
				_timer = 0;
			}

			virtual bool check_timer()
			{
				float delay = _delay;
				if (!_isIgnoringGlobalTurboSpeedSetting && DeepTurbo::cheaton)
				{
					if (DeepTurbo::turbospeed <= 0)
						return false;
					delay /= DeepTurbo::turbospeed;
				}
				if (_timer >= delay)
					return true;
				return false;
			}

			virtual void update_timer()
			{
				_timer += _timeStep;
			}

		public:

			//Call this once before mods initialization 
			static void init_sub_f_addr(InitPatternsManager* manager, HMODULE base)
			{
				if (manager == 0)
					throw std::exception("manager is nullptr");
				if (_sub_1425A9B00Addr != 0)
					return;
				_sub_1425A9B00Addr = manager->find_addr(base, "48 8B C4 48 89 48 08 53 48 81 EC B0").value_or(0);
				create_hook();
			}

			CoroutineBase(bool isInstantStartFirst = true, bool isIgnoringGlobalTurboSpeedSetting = false) : _isInstantStartFirst(isInstantStartFirst), _isIgnoringGlobalTurboSpeedSetting(isIgnoringGlobalTurboSpeedSetting) {}

			CoroutineBase(std::shared_ptr<Impl::IDelayedAction> action, bool isInstantStartFirst = true, bool isIgnoringGlobalTurboSpeedSetting = false) :
				CoroutineBase(isInstantStartFirst, isIgnoringGlobalTurboSpeedSetting)
			{
				_action = action;
			}

			CoroutineBase(const CoroutineBase& other) = delete;

			~CoroutineBase()
			{
				stop();
			}

			inline bool is_started() const noexcept { return _isStarted; }

			inline bool ignoring_timer_update_on_pause() const noexcept { return _isIgnoringUpdateOnPause; }

			inline bool is_ignoring_global_turbo_speed() const noexcept { return _isIgnoringGlobalTurboSpeedSetting; }

			inline float get_timer() const noexcept { return _timer; }

			inline float get_delay() const noexcept { return _delay; }

			inline void set_delay(float delay) noexcept { _delay = delay; }

			//How much coroutines are currently running
			static inline int get_running_count() noexcept { return _coroutines.size(); }

			//Do not run action when pause menu is opened
			inline void ignoring_update_on_pause(bool val) { _isIgnoringUpdateOnPause = val; }

			virtual void stop() noexcept
			{
				std::lock_guard<std::recursive_mutex> lck(_mtx);
				if (!_isStarted)
					return;
				_isStarted = false;
				unregister_coroutine(this);
			}

			void set_action(std::shared_ptr<Impl::IDelayedAction> action)
			{
				stop();
				_action = action;
			}

			virtual void start_action() = 0;
		};

	}

	template<class TAction, typename ...Args>
	class DelayedAction : public Impl::IDelayedAction
	{
	private:
		TAction _action = nullptr;

		std::tuple<Args...> _args;

	public:
		DelayedAction() {}

		DelayedAction(TAction action) : _action(action) {}

		inline void set_action(TAction&& action) noexcept { _actioon = &action; }

		inline void set_args(Args &&...args)
		{
			_args = std::forward_as_tuple(args...);
		}

		inline std::tuple<Args...> get_args() const noexcept { return _args; }

		void invoke() override
		{
			if (_action != nullptr)
				std::apply(_action, _args);//Ty cpp 17
		}
	};


	template<class TAction, typename ...Args>
	class Coroutine : public virtual Impl::CoroutineBase
	{
	public:
		//isInstantStartFirst - start action immediately on sub_f executing after start(...) was called;
		//isIgnoringGlobalTurboSpeedSetting - always execute curoutine with setted delay independing of current turbo speed;
		Coroutine(bool isInstantStartFirst = false, bool isIgnoringGlobalTurboSpeedSetting = false) : CoroutineBase(isInstantStartFirst, isIgnoringGlobalTurboSpeedSetting) {}

		//Action - coroutine's action
		//isInstantStartFirst - start action immediately on sub_f executing after start(...) was called;
		//isIgnoringGlobalTurboSpeedSetting - always execute curoutine with setted delay independing of current turbo speed;
		Coroutine(std::shared_ptr<DelayedAction<TAction, Args...>> action, bool isInstantStartFirst = true, bool isIgnoringGlobalTurboSpeedSetting = false) :
			CoroutineBase(action, isInstantStartFirst, isIgnoringGlobalTurboSpeedSetting) {}

		//Action - pointer to action method/function for coroutine;
		//isInstantStartFirst - start action immediately on sub_f executing after start(...) was called;
		//isIgnoringGlobalTurboSpeedSetting - always execute curoutine with setted delay independing of current turbo speed;
		Coroutine(TAction action, bool isInstantStartFirst = false, bool isIgnoringGlobalTurboSpeedSetting = false) :
			CoroutineBase(std::make_shared<DelayedAction<TAction, Args...>>(action), isInstantStartFirst, isIgnoringGlobalTurboSpeedSetting) {}

		Coroutine(const Coroutine<TAction, Args...>& other) = delete;

		void set_action(std::shared_ptr<DelayedAction<TAction, Args...>> action)
		{
			_action = action;
		}

		void set_action(TAction&& action)
		{
			(std::static_pointer_cast<DelayedAction>(_action))->set_action(action);
		}

		void start(Args ...args)
		{
			std::lock_guard<std::recursive_mutex> lck(_mtx);
			if (!_isStarted)
			{
				if (_action == nullptr)
					return;
				register_coroutine(this);
				reset_timer();
				_isFirstExe = true;
				auto downcasted = std::static_pointer_cast<DelayedAction<TAction, Args...>>(_action);
				downcasted->set_args(std::forward<Args>(args)...);
				_isStarted = true;
			}
		}

		//Call setted coroutine's action directly
		void start_action() override
		{
			if (_action == nullptr)
				throw std::exception("_action is nullptr");
			_action->invoke();
		}

		inline std::shared_ptr<DelayedAction<TAction, Args...>> get_action() const noexcept { return std::static_pointer_cast<DelayedAction<TAction, Args...>>(_action); }
	};
}