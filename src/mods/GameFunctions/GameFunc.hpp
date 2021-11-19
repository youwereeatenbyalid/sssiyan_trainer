#pragma once
#include <stdint.h>
#include "Mod.hpp"
#include <type_traits>
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

	class PtrController
	{
	private:
		static bool is_bad_ptr(uintptr_t ptr)
		{
			return (bool)IsBadReadPtr((void*)ptr, 0x8);
		}

		template<typename T>
		struct isValidImageFormat
		{
			constexpr static bool value =
				std::is_same<T, uintptr_t>::value;
		};

	public:
		template <typename T, size_t offsCount>
		static T get_ptr(uintptr_t base, const std::array<uintptr_t, offsCount> &offsets, bool& isBadPtr)
		{
			int count = offsets.size() - 1;
			if (!is_bad_ptr(base))
			{
				base = *(uintptr_t*)base;
				for (int i = 0; i < count; i++)
				{
					if (!is_bad_ptr(base))
						base = *(uintptr_t*)(base + offsets[i]);
					else
					{
						isBadPtr = true;
						return 0;
					}
				}
				if (!is_bad_ptr((base + offsets[count])))
				{
					T res = *(T*)(base + offsets[count]);
					isBadPtr = false;
					return res;
				}
			}
			isBadPtr = true;
			return 0;
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
			fAddr = g_framework->get_module().as<uintptr_t>();//it's not good relative to "static" behavior
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
