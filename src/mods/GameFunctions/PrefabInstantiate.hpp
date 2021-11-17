#pragma once
#include <mods/GameFunctions/GameFunc.hpp>
//clang-format off
namespace GameFunctions
{
	class PrefabInstantiate : public GameFunc
	{
		private:
		uintptr_t pfb;

		typedef void(__cdecl* prefabInstance)(void* rcx, void* pfb, Vec3* pos);
		prefabInstance prefab_instance;

		public:
		Vec3 pos;
		PrefabInstantiate(uintptr_t staticBase, uintptr_t prefab, Vec3 pos) : GameFunc(staticBase)
		{
			this->pos = pos;
			pfb = prefab;
		}

		inline uintptr_t get_prefab() const {return pfb; }
		inline void set_prefab(uintptr_t prefab){pfb = prefab; }

		void execute() override
		{
			prefab_instance(NULL, (void*) pfb, &pos);
		}
	};
}
//clang-format on
