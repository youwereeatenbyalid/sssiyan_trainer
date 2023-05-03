#pragma once
#include "GameFunc.hpp"
namespace GameFunctions
{
	/// <summary>
	/// Gemnerate Boss Doppelganger
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
			fAddr += 0x54CCC0;
			gen_doppel = (f_gen_doppel)fAddr;
		}

		void invoke(Vec3 createOffs, bool isProvoke)
		{
			if(!utility::isGoodReadPtr(pl, 8))
				return;
			gen_doppel(get_thread_context(), pl, createOffs, isProvoke);
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
			gen_doppel(get_thread_context(), pl, flag);
		}

		void operator()(bool flag)
		{
			invoke(flag);
		}
	};
}