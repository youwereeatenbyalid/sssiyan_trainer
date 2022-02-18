#pragma once
#include <mods/GameFunctions/GameFunc.hpp>
namespace GameFunctions
{
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
			fAddr += 0xCBE390;
			func = (f_request_camera_shake)fAddr;
		}

		RequestPlCameraShake(uintptr_t plCamera) : RequestPlCameraShake() { this->plCamera = (void*)plCamera; }

		void set_pl_camera(uintptr_t plCamera) { this->plCamera = (void*)plCamera; }

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
			func(context, plCamera, pos, rot, hypocenter, priority);
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