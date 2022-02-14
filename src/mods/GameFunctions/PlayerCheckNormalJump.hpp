#pragma once
#include "mods/GameFunctions/GameFunc.hpp"
//clang-format off
namespace GameFunctions
{
	class PlayerCheckNormalJump : public GameFunc<bool>
	{
	private:
		uintptr_t pl;
		typedef bool (__cdecl* f_pl_check_normal_jump)(uintptr_t rcx, uintptr_t player);
		f_pl_check_normal_jump func;
	public:
		PlayerCheckNormalJump(uintptr_t player) : pl(player)
		{
			fAddr += 0x4C11A0;
			func = (f_pl_check_normal_jump)fAddr;
		}

		bool invoke() override
		{
			auto rcx = get_thread_context();
			return func(rcx.value(), pl);
		}

		bool operator()() override
		{
			return invoke();
		}
	};
}
//clang-format on
