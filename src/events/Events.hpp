#pragma once
#include <algorithm>
#include <vector>
#include <memory>
#include <execution>
#include "EventHandler.hpp"

namespace Events
{
	template<typename... Args>
	class Event
	{
		using eh = Impl::BaseEventHandler<Args...>;
	private:
		std::vector<std::shared_ptr<eh>> _subscribed;

		bool _isAsyncInvoke = false;

	public:
		Event(bool isAsyncInvoke = false) : _isAsyncInvoke(isAsyncInvoke)
		{
		}

		void subscribe(std::shared_ptr<eh> eventHandler) noexcept
		{
			auto pos = std::find_if(_subscribed.begin(), _subscribed.end(), [&](const std::shared_ptr<eh> &other)
			{
				return *other == *eventHandler;
			});
			if (pos != _subscribed.end())
			{
				return;
			}
			_subscribed.push_back(eventHandler);
		}

		void unsubscribe(std::shared_ptr<eh> eventHandler) noexcept
		{
			auto pos = std::find_if(_subscribed.begin(), _subscribed.end(), [&](const std::shared_ptr<eh> &other)
			{
				return *other == *eventHandler;
			});
			if (pos == _subscribed.end())
				return;
			_subscribed.erase(pos);
		}

		template<typename... Args>
		void invoke(Args... args)
		{
			if (!_isAsyncInvoke)
			{
				for (auto& listener : _subscribed)
				{
					(*listener)(args...);
				}
			}
			else
			{
				std::for_each(std::execution::par, _subscribed.begin(), _subscribed.end(), [&](auto&& item)
				{
					(*item)(args...);
				});
			}
		}
	};
}