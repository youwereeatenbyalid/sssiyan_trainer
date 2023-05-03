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

		public:
			EffectID()
			{
				auto id = g_framework->get_module().as<uintptr_t>() + 0x7E60C88;
				memcpy(mng, (const void*)(*(uintptr_t*)id), 0x10);
			}

			EffectID(int dataContainerIndx, int containerID, int elementID, bool isDataContainerIndxOnly, bool isContainerIDOnly, bool isElementIDOnly) : EffectID()
			{
				DataContainerIndx = dataContainerIndx;
				ContainerID = containerID;
				ElementID = elementID;
				IsDataContainerIndxOnly = isDataContainerIndxOnly;
				IsElementIDOnly = isElementIDOnly;
			}

			EffectID(int containerID, int elementID) : EffectID(-1, containerID, elementID, true, false, false)
			{
			}
		};

	private:
		uintptr_t model;
		uintptr_t killEfxAddr = 0x12F5CE0;

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
			return request_set_effect(get_thread_context(), model, effectID.get(), pos, rot);
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
			kill_effect(get_thread_context(), model, effectID.get());
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