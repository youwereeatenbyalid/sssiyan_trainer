#pragma once
#include "GameFunc.hpp"
namespace GameFunctions
{
	/// <summary>
	/// Gemnerate Doppelganger
	/// </summary>
	class GenerateDoppel : public GameFunc<void>
	{
	private:
		uintptr_t pl;
		typedef void (__cdecl *f_gen_doppel)(uintptr_t threadVM, uintptr_t vergil, Vec3 createOffs, bool isProvoke);
		f_gen_doppel gen_doppel;

		void invoke() override { }
		void operator ()() override { }
	public:
		GenerateDoppel(uintptr_t plVergil)
		{
			pl = plVergil;
			//DevilMayCry5.app_PlayerVergilPL__generateDoppelGanger113976 
			//48 8B C4 48 89 58 18 56 57 41 56
			//generateDoppelGanger(via.vec3, System.Boolean)
			fAddr += 0x54CCC0;
			gen_doppel = (f_gen_doppel)fAddr;
		}

		void invoke(Vec3 createOffs, bool isProvoke)
		{
			if(!utility::isGoodReadPtr(pl, 8))
				return;
			sdk::call_object_func_easy<void*>((REManagedObject*)pl, "generateDoppelGanger(via.vec3, System.Boolean)", createOffs, isProvoke);
			//gen_doppel(get_thread_context(), pl, createOffs, isProvoke);
		}

		void operator()(Vec3 createOffs, bool isProvoke) { invoke(createOffs, isProvoke); }
	};

	/// <summary>
	/// Set Playable Vergil Doppelganger mode
	/// </summary>
	class SetDoppelMode : public GameFunc<void>
	{
	private:
		uintptr_t pl;
		typedef void (__cdecl *f_gen_doppel)(uintptr_t threadVM, uintptr_t vergil, bool flag);
		f_gen_doppel gen_doppel;

		void invoke() override { }
		void operator ()() override { }

	public:
		SetDoppelMode(uintptr_t plVergil)
		{
			pl = plVergil;
			//DevilMayCry5.app_PlayerVergilPL__setDoppelMode113919 
			//40 53 57 41 57 48 83 EC 40 48 8B 41 50 45
			//setDoppelMode(System.Boolean)
			fAddr += 0x534500;
			gen_doppel = (f_gen_doppel)fAddr;
		}
		/// <summary>
		/// Set PL Vergil Doppel State
		/// </summary>
		/// <param name="flag">activate or deactivate(?)</param>
		void invoke(bool flag)
		{
			if (!utility::isGoodReadPtr(pl, 8))
				return;
			sdk::call_object_func_easy<bool>((REManagedObject*)pl, "setDoppelMode(System.Boolean)", flag);
			//gen_doppel(get_thread_context(), pl, flag);
		}

		void operator()(bool flag)
		{
			invoke(flag);
		}
	};
}