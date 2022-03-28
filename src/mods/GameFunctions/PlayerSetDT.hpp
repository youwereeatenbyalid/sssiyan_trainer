#pragma once
#include "GameFunc.hpp"

namespace GameFunctions
{
	class PlayerSetDT : public GameFunc<bool>
	{
	public:
		enum DevilTrigger : uint32_t
		{
			Human,
			Devil,
			SDT
		};

	protected:
		uintptr_t pl;

		typedef bool (__cdecl *f_set_devil_trigger)(uintptr_t treadVM, uintptr_t player, DevilTrigger dt, bool isNotProduction);
		f_set_devil_trigger setDT;

		PlayerSetDT() { }

	private:
		bool invoke() override { }
		bool operator()() override { }

	public:

		PlayerSetDT(uintptr_t player)
		{ 
			pl = player;
			fAddr += 0x16A3040; //0x532B30;
			setDT = (f_set_devil_trigger)fAddr;
		}

		bool invoke(DevilTrigger dt, bool isNotProduction) noexcept
		{
			if(!utility::isGoodReadPtr(pl, 8))
				return false;
			return setDT(get_thread_context(), pl, dt, isNotProduction);
		}

		bool operator()(DevilTrigger dt, bool isNotProduction) noexcept { return invoke(dt, isNotProduction); }
	};

	class PlVergilSetDT : public PlayerSetDT
	{
	public:
		PlVergilSetDT(uintptr_t player)
		{
			pl = player;
			fAddr += 0x532B30;
			setDT = (f_set_devil_trigger)fAddr;
		}
	};
}