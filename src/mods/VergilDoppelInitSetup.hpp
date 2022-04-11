#pragma once
#include "Mod.hpp"
#include "InstantDoppel.hpp"

//clang-format off
namespace f = GameFunctions;
class VergilDoppelInitSetup : public Mod
{
public:
	VergilDoppelInitSetup() = default;

	static inline bool cheaton = true;
	static inline bool isControlledBySpeedState = false;

	static inline uintptr_t ret = 0;
	static inline uintptr_t fVergilSetActionAddr = 0;

	/// <summary>
	/// Call all pre-doppel mods stuff here.
	/// </summary>
	/// <returns>Return false if you need to let game generate doppel like how it's works by default. Othewise return true to skip summon doppel.</returns>
	static bool setup_doppel_asm(uintptr_t plVergil)
	{
		return InstantDoppel::setup_fast_doppel_asm(plVergil);
	}

	static naked void detour()
	{
		__asm {
			cheat:
			cmp r8d, 1
			jne originalcode
			push rax
			push rdx
			push rcx
			push rsi
			push r8
			push r9
			push r10
			push r12
			mov rcx, rdx
			sub rsp, 32
			call qword ptr [VergilDoppelInitSetup::setup_doppel_asm]
			add rsp, 32
			cmp al, 0
			pop r12
			pop r10
			pop r9
			pop r8
			pop rsi
			pop rcx
			pop rdx
			pop rax
			je originalcode
			jmp qword ptr [VergilDoppelInitSetup::ret]
			
			originalcode :
			call qword ptr [VergilDoppelInitSetup::fVergilSetActionAddr]
			jmp qword ptr [VergilDoppelInitSetup::ret]
		}
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

		auto setActionDoppelAddr = m_patterns_cache->find_addr(base, "E8 33 18 01 00");//DevilMayCry5.exe+533E68
		if (!setActionDoppelAddr)
		{
			return "Unable to find VergilDoppelInitSetup.setActionDoppelAddr pattern.";
		}

		fVergilSetActionAddr = m_patterns_cache->find_addr(base, "48 89 5C 24 20 56 57 41 57 48 83 EC 60").value_or(g_framework->get_module().as<uintptr_t>() + 0x5456A0); //DevilMayCry5.exe+5456A0

		if (!install_hook_absolute(setActionDoppelAddr.value(), m_doppel_hook, &detour, &ret, 0x5))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize VergilDoppelInitSetup.setActionDoppel";
		}

		return Mod::on_initialize();
	}

private:
	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}
	std::unique_ptr<FunctionHook> m_doppel_hook;

};
//clang-format on