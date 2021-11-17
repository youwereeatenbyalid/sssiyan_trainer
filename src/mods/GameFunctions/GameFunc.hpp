#pragma once
#include <stdint.h>
//clang-format off
namespace GameFunctions
{
	struct Vec3
	{
		float x = 0.0f;
		float z = 0.0f;
		float y = 0.0f;
		Vec3(){ }
		Vec3(float x, float y, float z) : x(x), y(y), z(z){ }
	};

	struct Quaternion
	{
		float x = 0;
		float y = 0;
		float z = 0;
		float w = 0;
		static Quaternion identity;

		Quaternion()
		{
			identity.w = 1.0f;
		}

		Quaternion(float x, float y, float z, float w) : Quaternion()
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}
	};

	class GameFunc
	{
	protected:
		void* fAddr;
		uintptr_t staticBase;
		uintptr_t fOffs;

	public:
		//typedef func here

		GameFunc(uintptr_t gameStaticBase)
		{
			staticBase = gameStaticBase;
			fAddr = (void*)(staticBase + fOffs);
		}

		uintptr_t get_address() const {return (uintptr_t)fAddr; }

		virtual void execute() = 0;
		virtual void operator()()
		{
			execute();
		}
	};
}
//clang-format on
