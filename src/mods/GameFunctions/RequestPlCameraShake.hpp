#pragma once
#include <mods/GameFunctions/GameFunc.hpp>
namespace GameFunctions
{
	/// <summary>
	/// Function for shaking screen
	/// </summary>
	class RequestPlCameraShake : GameFunc<void>
	{
	private:
		void *plCamera;

		typedef void(__cdecl *f_request_camera_shake)(uintptr_t context, void *plCamera, Vec3 pos, Quaternion rot, Vec3 hypocenter, int priority); //PlayerCameraController.RequestCameraShake
		f_request_camera_shake func;

		Vec3 pos;
		Quaternion rot;
		Vec3 hypocenter;
		int priority;

	public:
		RequestPlCameraShake()
		{
			//DevilMayCry5.app_PlayerCameraController__RequestCameraShake147317 
			//C3 CC CC 4C 8B DC 49 89 6B 18
			//RequestCameraShake(via.vec3, via.Quaternion, via.vec3, System.Int32)
			fAddr += 0xCBE390;
			func = (f_request_camera_shake)fAddr;
		}
		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="plCamera">player camera to shake</param>
		RequestPlCameraShake(uintptr_t plCamera) : RequestPlCameraShake() { this->plCamera = (void*)plCamera; }

		void set_pl_camera(uintptr_t plCamera) { this->plCamera = (void*)plCamera; }

		/// <summary>
		/// set parameters for screenshake
		/// </summary>
		/// <param name="pos"></param>
		/// <param name="rot"></param>
		/// <param name="hypocenter"></param>
		/// <param name="priority"></param>
		void set_params(Vec3 pos, Quaternion rot, Vec3 hypocenter, int priority)
		{
			this->pos = pos;
			this->rot = rot;
			this->hypocenter = hypocenter;
			this->priority = priority;
		}

		void invoke() override
		{
			auto context = get_thread_context();
			if (context == 0)
				return;
			sdk::call_object_func_easy<void*>((REManagedObject*)plCamera, "RequestCameraShake(via.vec3, via.Quaternion, via.vec3, System.Int32)", pos, rot, hypocenter, priority);
			//func(context, plCamera, pos, rot, hypocenter, priority);
		}

		void invoke(Vec3 pos, Quaternion rot, Vec3 hypocenter, int priority)
		{
			set_params(pos, rot, hypocenter, priority);
			invoke();
		}

		void operator()() override { invoke(); }

		void operator()(Vec3 pos, Quaternion rot, Vec3 hypocenter, int priority) { invoke(pos, rot, hypocenter, priority); }
	};
}