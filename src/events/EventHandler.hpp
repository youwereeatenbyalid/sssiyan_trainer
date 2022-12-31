#pragma once

namespace Events
{
	namespace Impl
	{
		template<typename... Args>
		class BaseEventHandler
		{
		public:
			virtual bool operator==(const BaseEventHandler<Args...>& other) const = 0;
			virtual void operator()(Args... args) = 0;
			virtual ~BaseEventHandler() = 0;
		};
	}

	template<typename T, typename... Args>
	class EventHandler : public Impl::BaseEventHandler<Args...>
	{
	protected:
		void (T::* _func)(Args...) = nullptr;
		T* _instance = nullptr;

	public:
		EventHandler(T* instance, void(T::* function)(Args...)) : _func(function), _instance(instance)
		{
		}

		virtual ~EventHandler() override
		{
			_instance = nullptr;
			_func = nullptr;
		}

		virtual void operator()(Args... args) override
		{
			if (_instance != nullptr && _func != nullptr)
				(_instance->*_func)(args...);
		}

		virtual bool operator ==(const Impl::BaseEventHandler<Args...>& other) const override
		{
			const EventHandler<T, Args...>* castedOther = static_cast<const EventHandler<T, Args...>*>(&other);
			if (castedOther == nullptr)
				return false;
			return (_instance == castedOther->_instance && _func == castedOther->_func);
		}
	};
}

template<typename... Args>
Events::Impl::BaseEventHandler<Args...>::~BaseEventHandler()
{
}