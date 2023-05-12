#pragma once
#include "GameFunc.hpp"

namespace GameFunctions
{
	class GameModelRequestSetEffect : public GameFunc<volatile void*>
	{
	public:
		class EffectID
		{
		private:
			char mng[0x10];

		public:

			int DataContainerIndx = -1;
			int ContainerID = 0;
			int ElementID = 0;

			bool IsDataContainerIndxOnly = true;
			bool IsContainerIDOnly = false;
			bool IsElementIDOnly = false;
			
		private:
			bool mng1F = false;
			REManagedObject* effect_id;
		public:
			EffectID()
			{
				effect_id = sdk::create_instance("via.effect.script.EffectID");
				utility::re_managed_object::add_ref(effect_id);
				//auto id = g_framework->get_module().as<uintptr_t>() + 0x7E60C88;
				//memcpy(mng, (const void*)(*(uintptr_t*)id), 0x10);
			}

			EffectID(int dataContainerIndx, int containerID, int elementID, bool isDataContainerIndxOnly, bool isContainerIDOnly, bool isElementIDOnly) : EffectID()
			{
				
				DataContainerIndx = dataContainerIndx;
				ContainerID = containerID;
				ElementID = elementID;
				IsDataContainerIndxOnly = isDataContainerIndxOnly;
				IsElementIDOnly = isElementIDOnly;
				IsContainerIDOnly = isContainerIDOnly;
				*(int*)(effect_id + 0x10) = DataContainerIndx;
				*(int*)(effect_id + 0x14) = ContainerID;
				*(int*)(effect_id + 0x18) = ElementID;
				*(bool*)(effect_id + 0x1C) = IsDataContainerIndxOnly;
				*(bool*)(effect_id + 0x1D) = IsContainerIDOnly;
				*(bool*)(effect_id + 0x1E) = IsElementIDOnly;

			}

			EffectID(int containerID, int elementID) : EffectID(-1, containerID, elementID, true, false, false)
			{
			}
			REManagedObject* get_effectIDObject() {
				return effect_id;
			}
		};

	private:
		uintptr_t model;
		uintptr_t killEfxAddr = 0x12F5CE0;
		//DevilMayCry5.app_GameModel__killEffect173892 "4D 85 C0 0F 84 35 01 00 00 55"
		//killEffect(via.effect.script.EffectID)
		typedef volatile void* (__cdecl* f_request_set_effect)(uintptr_t threadVM, uintptr_t gameModel, EffectID* effectID, Vec3 pos, Quaternion rot);
		typedef void(__cdecl* f_kill_effect)(uintptr_t threadVM, uintptr_t gameModel, EffectID* effectID);

		f_request_set_effect request_set_effect;
		f_kill_effect kill_effect;

		std::shared_ptr<EffectID> effectID;

		Vec3 pos;
		Quaternion rot;

	public:
		/// <summary>
		/// Setup to apply effect to a model
		/// </summary>
		/// <param name="gameModel">model to apply effect to</param>
		/// <param name="staticEffectID">ID of effect</param>
		GameModelRequestSetEffect(uintptr_t gameModel, std::shared_ptr<EffectID> staticEffectID) : model(gameModel), effectID(staticEffectID)
		{
			if (staticEffectID == nullptr)
				throw std::runtime_error("staticEffectID can't be NULL");
			killEfxAddr += fAddr;

			//DevilMayCry5.app_GameModel__requestEffect173889 "48 8B C4 55 56 57 41 56 48 81 EC D8"
			//requestEffect(via.effect.script.EffectID, via.vec3, via.Quaternion)
			fAddr += 0x12F3E00;
			request_set_effect = (f_request_set_effect)fAddr;
			kill_effect = (f_kill_effect)killEfxAddr;
		}
		/// <summary>
		/// create effect and previously specified position and rotation.
		/// </summary>
		/// <returns></returns>
		volatile void* invoke() override
		{
			if (!utility::isGoodReadPtr(model, 8))
				return nullptr;
			return sdk::call_object_func_easy<void*>((REManagedObject*)model, 
				"requestEffect(via.effect.script.EffectID, via.vec3, via.Quaternion)",
				effectID.get()->get_effectIDObject(), pos, rot);
			//auto game_model = reinterpret_cast<API::ManagedObject*>(model);
			//return game_model->call<API::ManagedObject*>("requestEffect(via.effect.script.EffectID, via.vec3, via.Quaternion)", effectID.get(), pos, rot);
			//return request_set_effect(get_thread_context(), model, effectID.get(), pos, rot);
		}

		/// <summary>
		/// operator call for class, create effect and previously specified position and rotation
		/// </summary>
		/// <returns></returns>
		volatile void* operator()() override
		{
			return invoke();
		}

		/// <summary>
		/// create effect at specified position and rotation
		/// </summary>
		/// <param name="pos"></param>
		/// <param name="rot"></param>
		/// <returns></returns>
		volatile void* invoke(Vec3 pos, Quaternion rot)
		{
			this->pos = pos;
			this->rot = rot;
			return invoke();
		}
		/// <summary>
		/// kill the effect being created by the class.
		/// </summary>
		void kill_efx()
		{
			if (!utility::isGoodReadPtr(model, 8))
				return;
			sdk::call_object_func_easy<void*>((REManagedObject*)model,
				"killEffect(via.effect.script.EffectID)",
				effectID.get()->get_effectIDObject());
			//game_model->call<API::ManagedObject*>("killEffect(via.effect.script.EffectID)", effectID.get());
			//kill_effect(get_thread_context(), model, effectID.get());
			return;
			
			
		}
		/// <summary>
		/// operator call for class, create effect at specified position and rotation
		/// </summary>
		/// <param name="pos"></param>
		/// <param name="rot"></param>
		/// <returns></returns>
		volatile void* operator()(Vec3 pos, Quaternion rot)
		{
			return invoke(pos, rot);
		}
		/// <summary>
		/// get ID of effect assigned
		/// </summary>
		/// <returns></returns>
		std::shared_ptr<EffectID> get_effect_id()
		{
			return effectID;
		}
		/// <summary>
		/// get address of kill effect function
		/// </summary>
		/// <returns></returns>
		uintptr_t get_kill_efx_addr() const noexcept
		{
			return killEfxAddr;
		}
	};
}