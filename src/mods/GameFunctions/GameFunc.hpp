#pragma once
#include "Mod.hpp"
#include <optional>
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

		static float vec_length(const Vec3 &p1, const Vec3 &p2)
		{
			return sqrt(((p2.x - p1.x) * (p2.x - p1.x)) + ((p2.y - p1.y) * (p2.y - p1.y)) + ((p2.z - p1.z) * (p2.z - p1.z)));
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
			return (bool)IsBadReadPtr((void*)ptr, 0x8);
		}

	public:
		/// <summary>
		/// Return nullopt if win.h isBadReadPtr() happened.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="base">DevilMayCry5.exe + baseOffset.</param>
		/// <param name="offsets">Other offsets.</param>
		/// <param name="get_addr">Set true if u don't want to get ptr value.</param>
		/// <param name="isDerefedBase">Is dereference base passed.</param>
		/// <returns></returns>
		template <typename T, size_t offsCount>
		static std::optional<T> get_ptr(uintptr_t base, const std::array<uintptr_t, offsCount> &offsets, bool get_addr = false, bool isDerefedBase = false)
		{
			int count = offsets.size() - 1;
			if (!is_bad_ptr(base))
			{
				if(!isDerefedBase)
					base = *(uintptr_t*)base;
				for (int i = 0; i < count; i++)
				{
					if (!is_bad_ptr(base))
						base = *(uintptr_t*)(base + offsets[i]);
					else
					{
						return std::nullopt;
					}
				}
				if (!is_bad_ptr((base + offsets[count])))
				{
					if (!get_addr)
					{
						T res = *(T*)(base + offsets[count]);
						return std::optional<T>(res);
					}
					else
						return std::optional<T>(base + offsets[count]);
				}
			}
			return std::nullopt;
		}

		/// <summary>
		/// Try to volatile pointer to data via offsets.
		/// </summary>
		/// <typeparam name="T">Pointer type</typeparam>
		/// <param name="offsets">Other offsets</param>
		/// <param name="base">DevilMayCry5.exe + baseOffset.</param>
		/// <param name="isDerefedBase">Is dereference base passed</param>
		/// <returns>Returns volatile ptr if it's valid, otherwise std::nullopt.</returns>
		template <typename T, size_t offsCount>
		static std::optional<volatile T*> get_ptr(const std::array<uintptr_t, offsCount>& offsets, uintptr_t base, bool isDerefedBase = false)
		{
			auto res = PtrController::get_ptr<uintptr_t>(base, offsets, true, isDerefedBase);
			if(!res.has_value())
				return std::nullopt;
			return std::make_optional<volatile T*>((volatile T*)res.value());
		}

		/// <summary>
		/// Try to write to pointer
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="base">DevilMayCry5.exe + baseOffset.</param>
		/// <param name="offsets">Other offsets.</param>
		/// <param name="val">New *ptr value</param>
		/// <param name="isDerefedBase">Is dereference base passed</param>
		/// <returns>Return true if success, false if ptr isn't valid</returns>
		template <typename T, size_t offsCount>
		static bool try_to_write(uintptr_t base, const std::array<uintptr_t, offsCount>& offsets, T val, bool isDerefedBase = false)
		{
			auto ptr = PtrController::get_ptr<T>(offsets, base, isDerefedBase);
			if (ptr.has_value())
			{
				*(ptr.value()) = val;
				return true;
			}
			return false;
		}

		/// <summary>
		/// Convert .Net List<T> to std::vector<T>.
		/// </summary>
		/// <typeparam name="T">Use * for ref types</typeparam>
		/// <param name="listPtr"></param>
		/// <param name="listCapacity">Out param, return .net List<T>.Capacity.</param>
		/// <returns>Returns std::nullopt if IsBadReadPtr() happens.</returns>
		template <typename T>
		std::optional<std::vector<T>> get_dotnet_list(uintptr_t listPtr, size_t &listCapacity)
		{
			if(listPtr == 0)
				return std::nullopt;
			if(is_bad_ptr(listPtr + 0x18) || is_bad_ptr(listPtr + 0x10))
				return std::nullopt;
			size_t count = *(size_t*)(listPtr + 0x18);
			std::vector<T> res;
			uintptr_t items = *(uintptr_t*)(listPtr + 0x10);
			if(listCapacity = (*(size_t*)(items + 0x1C)); listCapacity == 0)
				return res;
			size_t itemSize = sizeof(T);
			size_t itemsSize = itemSize * count;
			for (size_t i = 0x20; i < itemsSize; i += itemSize)
			{
				res.emplace_back(*(T*)(items + i));
			}
			return std::make_optional<std::vector<T>>(res);
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
		uintptr_t threadContext = 0;

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

		virtual uintptr_t get_thread_context()
		{
			return (uintptr_t)((void*)sdk::VM::get()->get_thread_context());
		}

		virtual T __cdecl invoke() = 0;
		virtual T __cdecl operator()()
		{
			return invoke();
		}
	};
}
//clang-format on
