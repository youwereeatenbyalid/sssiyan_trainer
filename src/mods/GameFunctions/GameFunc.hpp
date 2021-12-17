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
		Vec3() {}
		Vec3(float x, float y, float z) : x(x), y(y), z(z){}

		Vec3(const Vector3f& vec)
		{
			x = vec.x;
			y = vec.y;
			z = vec.z;
		}

		Vec3(const Vector3f&& vec)
		{
			x = vec.x;
			y = vec.y;
			z = vec.z;
		}

		Vec3& operator = (const Vector3f& vec)
		{
			x = vec.x;
			y = vec.y;
			z = vec.z;
			return *this;
		}

		static float vec_length(const Vec3 &v1, const Vec3 &v2)
		{
			return sqrt(((v2.x - v1.x) * (v2.x - v1.x)) + ((v2.y - v1.y) * (v2.y - v1.y)) + ((v2.z - v1.z) * (v2.z - v1.z)));
		}

		Vector3f to_vector3f()
		{
			Vector3f res;
			res.x = x;
			res.y = y;
			res.z = z;
			return res;
		}
	};

	struct Quaternion
	{
	public:
		float x = 0;
		float y = 0;
		float z = 0;
		float w = 1.0f;

		Quaternion() { /*GameFunctions::Quaternion::identity.w = 1.0f;*/ }

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
			return (bool)IsBadReadPtr((void*)ptr, 0x8);// not thread safe?
		}

	public:
		template <typename T, size_t offsCount>
		static T get_ptr(uintptr_t base, const std::array<uintptr_t, offsCount> &offsets, bool& isBadPtr, bool get_addr = false)
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
					if (!get_addr)
					{
						T res = *(T*)(base + offsets[count]);
						isBadPtr = false;
						return res;
					}
					else
						return base;
				}
			}
			isBadPtr = true;
			return 0;
		}
	};

	/// <summary>
	/// Abstract class. Wrap to smart pointer if u want to make static object and init it in mod init function.
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

		virtual T __cdecl invoke() = 0;
		virtual T __cdecl operator()()
		{
			return invoke();
		}
	};
}
//clang-format on
