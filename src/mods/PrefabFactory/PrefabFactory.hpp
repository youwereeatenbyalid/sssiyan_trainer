#pragma once
#include <memory>
#include "mods/GameFunctions/GameFunc.hpp"
#include "utility"

namespace PfbFactory
{
	/// <summary>
	/// class for creating prefabs to instatiate
	/// </summary>
	class PrefabFactory
	{
	private:
		typedef void(__cdecl* f_add_ref)(REManagedObject*);//praydog's first pattern scan & call for addRef(...) is reaaaally slow
		typedef void(__cdecl* f_release)(REManagedObject*);

	public:
		PrefabFactory() = delete;
		PrefabFactory(const PrefabFactory&& other) = delete;

		//Don't forget to delete pfb manually. This already calls add_ref(...) so you don't need to do it.
		template<typename TRet = REManagedObject*>
		static TRet create_prefab(const GameFunctions::SysString* path, uintptr_t threadCtxt = 0, bool simplify = false)
		{
			if (path == nullptr)
				return 0;
			auto pfb = sdk::create_instance("via.Prefab", simplify);
			if (pfb == nullptr)
				return 0;
			add_ref(pfb);
			if (threadCtxt == 0)
				threadCtxt = (uintptr_t)sdk::get_thread_context();
			static sdk::REMethodDefinition* _setPathMethod = sdk::find_method_definition("via.Prefab", "set_Path(System.String)");
			_setPathMethod->call(threadCtxt, pfb, path->get_net_str());
			return static_cast<TRet>(pfb);
		}

		//Same stuff what ReSdk has but using absolute adress instead of slow AOB scan for first time
		static void add_ref(REManagedObject* obj)
		{
			if (obj == nullptr)
				return;
			static f_add_ref addRef = (f_add_ref)(g_framework->get_module().as<uintptr_t>() + 0x2526820);
			addRef(obj);
		}

		static inline void release(REManagedObject* obj)
		{
			static f_release release = (f_release)(g_framework->get_module().as<uintptr_t>() + 0x2526FA0);
			release(obj);
		}
	};
}