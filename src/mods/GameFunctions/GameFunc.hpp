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
			return (bool)IsBadReadPtr((void*)ptr, 0x8);// not thread safe?
		}

	public:
		/// <summary>
		/// Return nullopt if win.h isBadReadPtr() happened.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="base">DevilMayCry5.exe + baseOffset.</param>
		/// <param name="offsets">Other offsets.</param>
		/// <param name="get_addr">Set true if u want to get addres, not ptr value itself.</param>
		/// <returns></returns>
		template <typename T, size_t offsCount>
		static std::optional<T> get_ptr(uintptr_t base, const std::array<uintptr_t, offsCount> &offsets, bool get_addr = false)
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
						//isBadPtr = true;
						return std::nullopt;
					}
				}
				if (!is_bad_ptr((base + offsets[count])))
				{
					if (!get_addr)
					{
						T res = *(T*)(base + offsets[count]);
						//isBadPtr = false;
						return std::optional<T>(res);
					}
					else
						return std::optional<T>(base);
				}
			}
			//isBadPtr = true;
			return std::nullopt;
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
		//ptr to some managed stuff idk. Sometimes it can be be null, but some times not. In second case u need check what argument function use in-game and make ptr scan for it.
		uintptr_t rcx = 0;

	public:
		static const inline uintptr_t rcxBase = 0x07EC2F60;//0x07E53660;
		static const inline uintptr_t rcxBase2 = 0x07E53660;
		static inline const std::array<uintptr_t, 5> fancyRcx { 0x70, 0x50, 0x408, 0x80, 0x0 }; //{0x498, 0x60, 0xC0, 0x0};//Think this is some clr thread stuff actually //D380
		static inline const std::array<uintptr_t, 4> fancyRcx2 { 0x4D8, 0x50, 0x118, 0x0 }; //{0x498, 0x60, 0xC0, 0x0};//Think this is some clr thread stuff actually //C8C0


		//typedef func here

		GameFunc()//Add func offset to fAddr in child constructor and init typedef func
		{
			fAddr = g_framework->get_module().as<uintptr_t>();//it's not good relative to "static" behavior
		}

		uintptr_t get_address() const
		{
			return fAddr;
		}

		//virtual std::optional<uintptr_t> get_rcx_ptr() { return std::nullopt; }

		virtual uintptr_t get_cur_rcx() const { return rcx; }

		virtual std::optional<uintptr_t> get_rcx_ptr(int indx)
		{
			switch (indx)
			{
				case 0:
					return get_rcx_ptr();
				case 1:
				{
					auto base = g_framework->get_module().as<uintptr_t>() + rcxBase2;
					return GameFunctions::PtrController::get_ptr<uintptr_t>(base, fancyRcx2, true);
					break;
				}
				default:
				{
					return get_rcx_ptr();
				}
			}
		}

		virtual std::optional<uintptr_t> get_rcx_ptr()
		{
			auto base = g_framework->get_module().as<uintptr_t>() + rcxBase;
			auto ptrAddr = GameFunctions::PtrController::get_ptr<uintptr_t>(base, fancyRcx, true);
			return ptrAddr;
		}

		virtual void set_rcx(uintptr_t param) { rcx = param; }

		virtual T __cdecl invoke() = 0;
		virtual T __cdecl operator()()
		{
			return invoke();
		}
	};
}
//clang-format on
