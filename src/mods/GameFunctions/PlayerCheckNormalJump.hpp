#pragma once
#include "mods/GameFunctions/GameFunc.hpp"
//clang-format off
namespace GameFunctions
{
	/// <summary>
	/// GameFunc implementation for PlayerCheckNormalJump 
	/// </summary>
	class PlayerCheckNormalJump : public GameFunc<bool>
	{
	private:
		uintptr_t pl;
		typedef bool (__cdecl* f_pl_check_normal_jump)(uintptr_t rcx, uintptr_t player);
		f_pl_check_normal_jump func;
	public:
		/// <summary>
		/// Constructor function
		/// </summary>
		/// <param name="player">pass player to call jump function on</param>
		PlayerCheckNormalJump(uintptr_t player) : pl(player)
		{
			//DevilMayCry5.app_Player__checkNormalJump171482
			//checkNormalJump()
			//40 53 48 83 EC 20 48 8B D9 48 8B 8A B8 03

			fAddr += 0x4C11A0;
			func = (f_pl_check_normal_jump)fAddr;
		}
		/// <summary>
		/// call check jump function
		/// </summary>
		/// <returns></returns>
		bool invoke() override
		{
			//auto rcx = get_thread_context();
			//return func(rcx, pl);
			return sdk::call_object_func_easy<bool>((REManagedObject*)pl, "checkNormalJump");
		}
		/// <summary>
		/// operator call. call check jump function
		/// </summary>
		/// <returns></returns>
		bool operator()() override
		{
			return invoke();
		}
	};
}
//clang-format on
