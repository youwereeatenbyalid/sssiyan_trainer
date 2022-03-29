#pragma once
#include "Mod.hpp"
#include "utility/Memory.hpp"
#include <optional>
#include <type_traits>
#define M_PI 3.14159265358979323846

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

		Vec3 operator + (const Vec3 &vAdd) { return Vec3(x + vAdd.x, y + vAdd.y, z + vAdd.z ); }

		friend Vec3 operator +(float num, const Vec3 &vec) {return Vec3(vec.x + num, vec.y + num, vec.z + num); }

		Vec3& operator +=(const Vec3& vAdd)
		{
			x += vAdd.x;
			y += vAdd.y;
			z += vAdd.z;
			return *this;
		}

		Vec3& operator *=(const float num)
		{
			x*=num;
			y*=num;
			z*=num;
			return *this;
		}

		Vec3 operator - (const Vec3 &vSub) { return Vec3(x - vSub.x, y - vSub.y, z - vSub.z ); }

		Vec3& operator -=(const Vec3& vSub)
		{
			x -= vSub.x;
			y -= vSub.y;
			z -= vSub.z;
			return *this;
		}

		friend Vec3 operator *(float num, const Vec3 &vec) {return Vec3(vec.x * num, vec.y * num, vec.z * num); }

		friend Vec3 operator /(float num, const Vec3& vec) {return Vec3(vec.x / num, vec.y / num, vec.z / num); }

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
	private:
		

	public:
		float x = 0;
		float z = 0;
		float y = 0;
		float w = 1.0f;

		Quaternion() { /*GameFunctions::Quaternion::identity.w = 1.0f;*/ }

		Quaternion(float x, float y, float z, float w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

		enum Axis : short
		{
			X,
			Y,
			Z
		};

		enum RotType : short
		{
			Active,
			Passive
		};		

		Quaternion inline conjugate() const noexcept { return Quaternion(-x, -y, -z, w); }

		float inline norm() const noexcept { return x * x + y * y + z * z + w * w; }

		Quaternion inverse() const
		{
			auto norm = this->norm();
			auto conj = this->conjugate();
			return Quaternion(conj.x / norm, conj.y / norm, conj.z / norm, conj.w / norm);
		}

		friend Quaternion operator *(const Quaternion& r1, const Quaternion &r2)
		{
			float w = r1.w*r2.w - r1.x*r2.x - r1.y*r2.y - r1.z*r2.z;
			float x = r1.w*r2.x + r1.x*r2.w - r1.y*r2.z + r1.z*r2.y;
			float y = r1.w*r2.y + r1.x*r2.z + r1.y*r2.w - r1.z*r2.x;
			float z = r1.w*r2.z - r1.x*r2.y + r1.y*r2.x + r1.z*r2.w;
			return Quaternion(x, y, z, w);
		}

		Quaternion& operator *=(const Quaternion& r2)
		{
			*this = *this * r2;
			return *this;
		}

		static Vec3 q_rot(const Quaternion& r1, const Vec3& v1, RotType type = Passive)
		{
			auto inverseQ = r1.conjugate();
			Quaternion res;
			Quaternion r2(v1.x, v1.y, v1.z, 0);
			if (type == Passive)
				res = r1 * r2 * inverseQ;
			else
				res = inverseQ * r2 * r1;
			return Vec3(res.x, res.y, res.z);
		}

		/*friend Vec3 operator *(const Quaternion& r1, const Vec3& v1)
		{
			auto inverseQ = Quaternion::conjugate(r1);
			Quaternion r2(v1.x, v1.y, v1.z, 0);
			auto res = r1 * r2;
			res = res * inverseQ;
			return Vec3(res.x, res.y, res.z);

		}*/

	};

	class PtrController
	{
	private:

	public:

		static bool is_bad_ptr(uintptr_t ptr) noexcept
		{
			return !utility::isGoodReadPtr(ptr, sizeof(ptr));
		}

		/// <summary>
		/// Return nullopt if win.h isBadReadPtr() happened.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="base">DevilMayCry5.exe + baseOffset.</param>
		/// <param name="offsets">Other offsets.</param>
		/// <param name="get_addr">Set true if u don't want to get ptr value.</param>
		/// <param name="isDerefedBase">Is dereference base passed.</param>
		/// <returns></returns>
		template <size_t offsCount>
		static std::optional<uintptr_t> get_ptr(uintptr_t base, const std::array<uintptr_t, offsCount> &offsets, bool isDerefedBase = false) noexcept
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
					return std::optional<uintptr_t>(base + offsets[count]);
				}
			}
			return std::nullopt;
		}

		template<typename T, size_t offsCount>
		static std::optional<T> get_ptr_val(uintptr_t base, const std::array<uintptr_t, offsCount>& offsets, bool isDerefedBase = false) noexcept
		{
			auto addr = PtrController::get_ptr(base, offsets, isDerefedBase);
			if(!addr)
				return std::nullopt;
			if(is_bad_ptr(addr.value()))
				return std::nullopt;
			return std::make_optional<T>(*(T*)(addr.value()));
		}

		/// <summary>
		/// Try to get volatile pointer to data via offsets.
		/// </summary>
		/// <typeparam name="T">Pointer type</typeparam>
		/// <param name="offsets">Other offsets</param>
		/// <param name="base">DevilMayCry5.exe + baseOffset.</param>
		/// <param name="isDerefedBase">Is dereference base passed</param>
		/// <returns>Returns pointer if it's valid, otherwise nullptr.</returns>
		template <typename T, size_t offsCount>
		volatile T* get_ptr(const std::array<uintptr_t, offsCount>& offsets, uintptr_t base, bool isDerefedBase = false) noexcept
		{
			auto res = PtrController::get_ptr<uintptr_t>(base, offsets, isDerefedBase);
			if(!res.has_value())
				return nullptr;
			return (volatile T*)res.value();
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
		static bool try_to_write(uintptr_t base, const std::array<uintptr_t, offsCount>& offsets, T val, bool isDerefedBase = false) noexcept
		{
			auto ptr = PtrController::get_ptr(base, offsets, isDerefedBase);
			if (ptr.has_value())
			{
				*(T*)(ptr.value()) = val;
				return true;
			}
			return false;
		}
	};

	class StringController
	{
	public:

		static std::string get_str(uintptr_t dotNetString) noexcept
		{
			if (PtrController::is_bad_ptr(dotNetString))
				return nullptr;
			auto length = *(int32_t*)(dotNetString + 0x10);
			std::string res;
			res.reserve(length);
			res.resize(length);
			for (int i = 0, j = 0; i < length; i++, j += 2)
				res[i] = *(byte*)(dotNetString + 0x14 + j);
			return std::move(res);
		}

		template<size_t size>
		static void get_str(uintptr_t dotNetString, std::array<char, size>* strOut, unsigned int &dotNetStrLengthOut) noexcept
		{
			if (strOut == nullptr /*|| PtrController::is_bad_ptr(dotNetString)*/)
				return;
			unsigned int endCounter = strOut->size();
			dotNetStrLengthOut = *(unsigned int*)(dotNetString + 0x10);
			if (endCounter > dotNetStrLengthOut)
				endCounter = dotNetStrLengthOut;
			for (int i = 0, j = 0; i < endCounter; i++, j += 2)
				(*strOut)[i] = *(byte*)(dotNetString + 0x14 + j);
			*(strOut->data() + dotNetStrLengthOut) = 0;
		}

		static unsigned int get_str_length(uintptr_t dotNetString)
		{
			if (PtrController::is_bad_ptr(dotNetString))
				return 0;
			return *(unsigned int*)(dotNetString + 0x10);
		}

		static bool str_cmp(uintptr_t dotNetString, const char* str) noexcept
		{
			if (PtrController::is_bad_ptr(dotNetString))
				return false;
			auto strLength = *(unsigned int*)(dotNetString + 0x10);
			if (strLength != strlen(str))
				return false;
			for (int i = 0, j = 0; i < strLength; i++, j += 2)
			{
				if (str[i] != *(char*)(dotNetString + 0x14 + j))
					return false;
			}
			return true;
		}
	};

	class ListController
	{
	public:

		/// <summary>
		/// Convert .Net List<T> to std::vector<T>.
		/// </summary>
		/// <typeparam name="T">Use * for ref types</typeparam>
		/// <param name="listPtr"></param>
		/// <param name="listCapacity">Out param, return .net List<T>.Capacity.</param>
		/// <returns>Returns std::nullopt if IsBadReadPtr() happens.</returns>
		template <typename T>
		std::optional<std::vector<T>> get_dotnet_list(uintptr_t listPtr, size_t& listCapacity) noexcept
		{
			if (listPtr == 0)
				return std::nullopt;
			if (PtrController::is_bad_ptr(listPtr + 0x18) || PtrController::is_bad_ptr(listPtr + 0x10))
				return std::nullopt;
			size_t count = *(size_t*)(listPtr + 0x18);
			std::vector<T> res;
			uintptr_t items = *(uintptr_t*)(listPtr + 0x10);
			if (listCapacity = (*(size_t*)(items + 0x1C)); listCapacity == 0)
				return res;
			size_t itemSize = sizeof(T);
			size_t itemsSize = itemSize * count;
			for (size_t i = 0x20; i < itemsSize; i += itemSize)
			{
				res.emplace_back(*(T*)(items + i));
			}
			return std::make_optional<std::vector<T>>(res);
		}

		int get_list_count(uintptr_t dotNetList) noexcept
		{
			if(PtrController::is_bad_ptr(dotNetList))
				return 0;
			else return *(int*)(dotNetList + 0x18);
		}

		int get_list_capacity(uintptr_t dotNetList) noexcept
		{
			if (PtrController::is_bad_ptr(dotNetList) || PtrController::is_bad_ptr(dotNetList+0x10))
				return 0;
			auto items = *(uintptr_t*)(dotNetList + 0x10);
			return *(int*)(items + 0x1C);
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

		virtual uintptr_t get_thread_context(uint32_t unk = -1)
		{
			return (uintptr_t)((void*)sdk::get_thread_context(unk));
		}

		virtual T __cdecl invoke() = 0;
		virtual T __cdecl operator()()
		{
			return invoke();
		}
	};
}
//clang-format on
