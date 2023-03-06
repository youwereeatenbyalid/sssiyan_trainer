#pragma once
#include <mods/GameFunctions/GameFunc.hpp>
//clang-format off
namespace GameFunctions
{
	class PrefabInstantiate : public GameFunc<uintptr_t>
	{
	private:
		uintptr_t pfb = 0;

		typedef uintptr_t(__cdecl* prefabInstantiate)(void* rcx, void* pfb, const Vec3& pos);//should return GameObject?
		prefabInstantiate prefab_instantiate;

	public:
		Vec3 pos;
		PrefabInstantiate()
		{
			fAddr += 0x252100;
			prefab_instantiate = (prefabInstantiate)fAddr;
		}
		PrefabInstantiate(uintptr_t prefab, Vec3 pos) : PrefabInstantiate()
		{
			this->pos = pos;
			pfb = prefab;
		}

		inline uintptr_t get_prefab() const
		{
			return pfb;
		}
		inline void set_prefab(uintptr_t prefab)
		{
			pfb = prefab;
		}

		uintptr_t invoke() override
		{
			if (fAddr != NULL && pfb != 0)
				return prefab_instantiate(NULL, (void*)pfb, pos);
			return 0;
		}

		uintptr_t invoke(uintptr_t prefab, const Vec3& pos)
		{
			pfb = prefab;
			if (fAddr != NULL && pfb != 0)
				return prefab_instantiate(NULL, (void*)pfb, pos);
			return 0;
		}

		uintptr_t operator()(uintptr_t prefab, const Vec3& pos)
		{
			return invoke(prefab, pos);
		}
	};

	class PrefabInstantiateScale : public GameFunc<uintptr_t>
	{
	private:
		uintptr_t pfb = 0;
		typedef uintptr_t(__cdecl* prefabInstantiateScale)(void* rcx, void* pfb, const Vec3& pos, const Quaternion& rotation, const Vec3& scale);//should return GameObject?
		prefabInstantiateScale prefab_instantiate_scale;
	public:
		Vec3 pos;
		Quaternion rot;
		Vec3 scale;
		PrefabInstantiateScale()
		{
			fAddr += 0xA6E740;
			prefab_instantiate_scale = (prefabInstantiateScale)fAddr;
		}

		PrefabInstantiateScale(uintptr_t prefab, Vec3 pos, Vec3 scale, Quaternion rotation) : PrefabInstantiateScale()
		{
			pfb = prefab;
			rot = rotation;
			this->scale = scale;
		}

		inline uintptr_t get_prefab() const
		{
			return pfb;
		}

		inline void set_prefab(uintptr_t prefab)
		{
			pfb = prefab;
		}

		uintptr_t invoke() override
		{
			if (fAddr != 0 && pfb != 0)
				return prefab_instantiate_scale(NULL, (void*)pfb, pos, rot, scale);
			return 0;
		}

		uintptr_t invoke(uintptr_t prefab, const Vec3& position, const Quaternion& rotation, const Vec3& scale)
		{
			pfb = prefab;
			pos = position;
			rot = rotation;
			this->scale = scale;
			return invoke();
			/*if (fAddr != 0 && prefab != 0)
				return prefab_instantiate_scale(NULL, (void*)prefab, position, rotation, scale);
			return 0;*/
		}

		uintptr_t operator()(uintptr_t prefab, const Vec3& position, const Quaternion& rotation, const Vec3& scale)
		{
			return invoke(prefab, position, rotation, scale);
		}

		uintptr_t invoke(const Vec3& position, const Quaternion& rotation, const Vec3& scale)
		{
			pos = position;
			rot = rotation;
			this->scale = scale;
			return invoke();
			/*if (fAddr != 0 && pfb != 0)
				return prefab_instantiate_scale(NULL, (void*)pfb, position, rotation, scale);
			return 0;*/
		}

		uintptr_t operator()(const Vec3& position, const Quaternion& rotation, const Vec3& scale)
		{
			return invoke(position, rotation, scale);
		}
	};
}
//clang-format on
