#pragma once
#include <stdint.h>
#include "Mod.hpp"
//clang-format off
namespace GameFunctions
{
	struct Vec3
	{
		float x = 0.0f;
		float z = 0.0f;
		float y = 0.0f;
		Vec3()
		{
		}
		Vec3(float x, float y, float z) : x(x), y(y), z(z)
		{
		}
	};

	struct Quaternion
	{
	public:
		float x = 0;
		float y = 0;
		float z = 0;
		float w = 0;

		Quaternion()
		{
			//GameFunctions::Quaternion::identity.w = 1.0f;
		}

		Quaternion(float x, float y, float z, float w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

	};

	/// <summary>
	/// Abstract class
	/// </summary>
	/// <typeparam name="T">Type of ret value.</typeparam>
	template <typename T>
	class GameFunc
	{
	protected:
		uintptr_t fAddr;

	public:
		//typedef func here

		GameFunc()//Add func offset to fAddr in child constructor and init typedef func
		{
			fAddr = g_framework->get_module().as<uintptr_t>();
		}

		uintptr_t get_address() const
		{
			return fAddr;
		}

		virtual T invoke() = 0;
		virtual T operator()()
		{
			return invoke();
		}
	};
}
//clang-format on
