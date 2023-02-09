#pragma once
#include "Mod.hpp"
#include "events/Events.hpp"

//clang-format off
namespace f = GameFunctions;
class VergilDoppelInitSetup : public Mod
{
public:
	enum DoppelDelayState
	{
		None,
		Fast,
		Default,
		Slow
	};

private:
	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	std::unique_ptr<FunctionHook> _setupDoppelModeHook;

	static inline VergilDoppelInitSetup* _mod = nullptr;

	Events::Event<uintptr_t, DoppelDelayState, bool*/*skipSummon*/> _onSummonEvent{};

	static void setup_doppel_mode_hook(uintptr_t threadCtxt, uintptr_t pl0800, DoppelDelayState delayState)
	{
		bool skip = false;
		_mod->_onSummonEvent.invoke(pl0800, delayState, &skip);
		if (!skip)
			_mod->_setupDoppelModeHook->get_original<decltype(setup_doppel_mode_hook)>()(threadCtxt, pl0800, delayState);
	}

public:
	VergilDoppelInitSetup()
	{
		_mod = this;
	}

	std::string_view get_name() const override
	{
		return "VergilDoppelInitSetup";
	}
	std::string get_checkbox_name() override
	{
		return m_check_box_name;
	};
	std::string get_hotkey_name() override
	{
		return m_hot_key_name;
	};

	std::optional<std::string> on_initialize() override
	{
		init_check_box_info();
		auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

		auto setupDoppelModeAddr = m_patterns_cache->find_addr(base, "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 50 48 8B 41 50 41 8B F0 48 8B DA");
		//DevilMayCry5.app_PlayerVergilPL__setupDoppelMode113918 
		if (!setupDoppelModeAddr)
		{
			return "Unable to find VergilDoppelInitSetup.setupDoppelModeAddr pattern.";
		}

		_setupDoppelModeHook = std::make_unique<FunctionHook>(setupDoppelModeAddr.value(), &VergilDoppelInitSetup::setup_doppel_mode_hook);
		_setupDoppelModeHook->create();

		return Mod::on_initialize();
	}

	//Pass false if you need to let game generate doppel like how it's works by default. Othewise pass true to skip summon doppel
	template<class T>
	static void on_doppel_summon_sub(std::shared_ptr<Events::EventHandler<T, uintptr_t, DoppelDelayState, bool*>> eh)
	{
		_mod->_onSummonEvent.subscribe(eh);
	}

	template<class T>
	static void on_doppel_summon_unsub(std::shared_ptr<Events::EventHandler<T, uintptr_t, DoppelDelayState, bool*>> eh)
	{
		_mod->_onSummonEvent.unsubscribe(eh);
	}
};
//clang-format on