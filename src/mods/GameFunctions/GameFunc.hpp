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

		Vec3(const Vec3& vec)
		{
			x = vec.x;
			y = vec.y;
			z = vec.z;
		}

		Vec3(const volatile Vec3& vec)
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
	};

	class PtrController
	{
	private:

	public:

		static bool is_bad_ptr(uintptr_t ptr) noexcept
		{
			return !utility::isGoodReadPtr(ptr, sizeof(ptr));
		}

		static bool is_bad_ptr(volatile void *ptr) noexcept
		{
			return !utility::isGoodReadPtr((uintptr_t)ptr, sizeof(ptr));
		}

		static bool is_bad_ptr(void* ptr) noexcept
		{
			return !utility::isGoodReadPtr((uintptr_t)ptr, sizeof(ptr));
		}

		/// <summary>
		/// safely retrieves an address using a pointer chain.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="base">DevilMayCry5.exe + baseOffset.</param>
		/// <param name="offsets">Array of offsets in pointer chain.</param>
		/// <param name="isDerefedBase">Set true if a dereferenced base is passed.</param>
		/// <returns>the address at the end of a pointer chain, or nullopt if win.h isBadReadPtr() happened. </returns>
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

		/// <summary>
		/// safely retrieves a value using a pointer chain.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="base">DevilMayCry5.exe + baseOffset.</param>
		/// <param name="offsets">Array of offsets in pointer chain.</param>
		/// <param name="isDerefedBase">Set true if a dereferenced base is passed.</param>
		/// <returns>the value at the end of a pointer chain, or nullopt if win.h isBadReadPtr() happened. </returns>
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
		/// unsafe retrieval of an address using a pointer chain.
		/// </summary>
		/// <typeparam name="T">Pointer type</typeparam>
		/// <param name="base">DevilMayCry5.exe + baseOffset.</param>
		/// <param name="offsets">Array of offsets in pointer chain</param>
		/// <param name="isDerefedBase">Set true if a dereferenced base is passed</param>
		/// <returns>Returns pointer if it's valid, otherwise nullptr.</returns>
		template <typename T, size_t offsCount>
		static volatile T* get_ptr(const std::array<uintptr_t, offsCount>& offsets, uintptr_t base, bool isDerefedBase = false) noexcept
		{
			auto res = PtrController::get_ptr(base, offsets, isDerefedBase);
			if(!res.has_value())
				return nullptr;
			return (volatile T*)res.value();
		}

		/// <summary>
		/// safely writes a value to to an address using a pointer chain.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="base">DevilMayCry5.exe + baseOffset.</param>
		/// <param name="offsets">Array of offsets in pointer chain.</param>
		/// <param name="val">New *ptr value</param>
		/// <param name="isDerefedBase">Set true if a dereferenced base is passed</param>
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
	/// <summary>
	/// Helper class for reading RE strings as different string types.
	/// </summary>
	class StringController
	{
	public:

		/// <summary>
		/// converts from RE string to std::string.
		/// </summary>
		/// <param name="dotNetString">input string</param>
		/// <param name="isNoOffs"></param>
		/// <returns>dotNetString as std::string</returns>
		static std::string get_str(uintptr_t dotNetString, bool isNoOffs = false) noexcept
		{
			auto length = *(int32_t*)(dotNetString + 0x10);
			std::string res;
			res.reserve(length + 1);
			res.resize(length);
			int startOffs = isNoOffs ? 0 : 0x14;
			for (int i = 0, j = 0; i < length; i++, j += 2)
				res[i] = *(byte*)(dotNetString + startOffs + j);
			return std::move(res);
		}
		/// <summary>
		/// converts from RE string to std::wstring.
		/// </summary>
		/// <param name="dotNetString">input string</param>
		/// <param name="isNoOffs"></param>
		/// <returns>dotNetString as std::wstring</returns>
		static std::wstring get_str(uintptr_t dotNetString) noexcept
		{
			if (dotNetString == 0)
				return std::wstring();
			return std::move(std::wstring((const wchar_t*)(dotNetString + 0x14)));
		}
		/// <summary>
		/// converts from RE string to a raw wstring.
		/// </summary>
		/// <param name="dotNetString">input string</param>
		/// <param name="isNoOffs"></param>
		/// <returns>dotNetString as a wstr </returns>
		static const wchar_t const* get_raw_wstr(uintptr_t dotNetString) noexcept
		{
			if (dotNetString == 0)
				return nullptr;
			return (const wchar_t*)(dotNetString + 0x14);
		}
		/// <summary>
		/// Write .Net string to passed char array
		/// </summary>
		/// <param name="dotNetString">.Net string to read</param>
		/// <param name="strOut">output array to write to</param>
		/// <param name="dotNetStrLengthOut">size of .Net string</param>
		/// <param name="isNoOffs"></param>
		/// <returns></returns>
		template<size_t size>
		static void get_str(uintptr_t dotNetString, std::array<char, size>* strOut, unsigned int &dotNetStrLengthOut, bool isNoOffs = false) noexcept
		{
			if (strOut == nullptr /*|| PtrController::is_bad_ptr(dotNetString)*/)
				return;
			unsigned int endCounter = strOut->size();
			dotNetStrLengthOut = *(unsigned int*)(dotNetString + 0x10);
			if (endCounter > dotNetStrLengthOut)
				endCounter = dotNetStrLengthOut;
			int startOffs = isNoOffs ? 0 : 0x14;
			for (int i = 0, j = 0; i < endCounter; i++, j += 2)
				(*strOut)[i] = *(byte*)(dotNetString + startOffs + j);
			*(strOut->data() + dotNetStrLengthOut) = 0;
		}
		/// <summary>
		/// gets the length of an RE string.
		/// </summary>
		/// <param name="dotNetString">input string</param>
		/// <param name="isNoOffs"></param>
		/// <returns>dotNetString as a wstr </returns>
		static unsigned int get_str_length(uintptr_t dotNetString)
		{
			if (PtrController::is_bad_ptr(dotNetString))
				return 0;
			return *(unsigned int*)(dotNetString + 0x10);
		}

		/// <summary>
		/// String comparison between RE string and char sequence.
		/// </summary>
		/// <param name="dotNetString"></param>
		/// <param name="str"></param>
		/// <returns></returns>
		static bool str_cmp(uintptr_t dotNetString, const char* str) noexcept
		{
			/*if (PtrController::is_bad_ptr(dotNetString))
				return false;*/
			if (dotNetString == 0)
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
		/// <summary>
		/// String comparison between RE string and wchar sequence.
		/// </summary>
		/// <param name="dotNetString"></param>
		/// <param name="str"></param>
		/// <returns></returns>
		static bool str_cmp(uintptr_t dotNetString, const wchar_t* str) noexcept
		{
			/*if (PtrController::is_bad_ptr(dotNetString))
				return false;*/
			if (dotNetString == 0)
				return false;
			const wchar_t* newStrRaw = (const wchar_t*)(dotNetString + 0x14);
			int res = wcscmp(str, newStrRaw);
			return res == 0;
		}
	};

	/// <summary>
	/// Represents System.String with same static mng offsets for all strings
	/// </summary>
	class SysString
	{
	private:
		void* _data = nullptr;
		wchar_t* _str = nullptr;
		int _length = 0;

		static inline REManagedObject* _mngString = nullptr;
		/// <summary>
		/// reallocates the system string to hold the new value
		/// </summary>
		/// <param name="newStr"></param>
		void realloc(const wchar_t* newStr)
		{
			_length = wcslen(newStr);
			int size = _length * sizeof(wchar_t);
			if(_data != nullptr)
				_data = std::realloc(_data, 0x14 + size);
			else
				_data = std::malloc(0x14 + size);
			if (_mngString == nullptr)
			{
				_mngString = sdk::create_instance("System.String");
				typedef void(__cdecl* f_add_ref)(REManagedObject*);
				auto base = g_framework->get_module().as<uintptr_t>();
				//this needs to be changed to not be a static offset
				//calls a function that adds a reference to the RE string so garbage collection doesn't immediately eat it. 
				((f_add_ref)(base + 0x2526820))(_mngString);
			}
			memcpy(_data, (const void*)_mngString, 0x10);//just copy all remanagedObj stuff to fake string
			*(int*)((uintptr_t)_data + 0x10) = _length;
			_str = (wchar_t*)((uintptr_t)_data + 0x14);
			wcsncpy(_str, newStr, _length);
			_str[_length] = '\0';
		}

	public:
		SysString(const wchar_t* str)
		{
			if (str == nullptr)
				throw std::exception("str is nullptr");
			realloc(str);
		}

		SysString(const SysString& other)
		{
			_data = other._data;
			_str = other._str;
			_length = other._length;
		}

		~SysString()
		{
			std::free(_data);
			_str = nullptr;
			_data = nullptr;
			_length = 0;
		}

		void* get_net_str() const noexcept { return _data; }

		wchar_t* get_str() const noexcept { return _str; }

		int get_length() const noexcept { return _length; }

		wchar_t& operator[](int indx)
		{
			if (indx < 0 || indx >= _length)
				throw std::out_of_range("Index out of range.");
			return _str[indx];
		}
	};
	/// <summary>
	/// Helper class for reading from REFramework lists.
	/// </summary>
	class ListController
	{
	public:

		/// <summary>
		/// safely Convert .Net List<T> to std::vector<T>.
		/// </summary>
		/// <typeparam name="T">Use * for ref types</typeparam>
		/// <param name="listPtr">address of list</param>
		/// <param name="listCapacityOut">Out param, returns the .net List<T>.Capacity.</param>
		/// <returns>Returns std::nullopt if IsBadReadPtr() happens, or a vector representation of the RE list.</returns>
		template <typename T>
		static std::optional<std::vector<T>> get_dotnet_list(uintptr_t listPtr, size_t& listCapacityOut) noexcept
		{
			if (listPtr == 0)
				return std::nullopt;
			if (PtrController::is_bad_ptr(listPtr + 0x18) || PtrController::is_bad_ptr(listPtr + 0x10))
				return std::nullopt;
			size_t count = *(size_t*)(listPtr + 0x18);
			std::vector<T> res;
			uintptr_t items = *(uintptr_t*)(listPtr + 0x10);
			listCapacityOut = (*(size_t*)(items + 0x1C));
			if (listCapacityOut == 0)
				return std::make_optional<std::vector<T>>(res);
			size_t itemSize = sizeof(T);
			size_t itemsCount = 0x20 + itemSize * count;
			for (size_t i = 0x20; i < itemsCount; i += itemSize)
			{
				res.emplace_back(*(T*)(items + i));
			}
			return std::make_optional<std::vector<T>>(res);
		}
		/// <summary>
		/// Retrieves an item from an RE list
		/// </summary>
		/// <typeparam name="T">Type of item being retrieved</typeparam>
		/// <param name="listPtr">pointer to list</param>
		/// <param name="indx">index of item to retrieve</param>
		/// <returns>the item from the list at the specified index</returns>
		template<typename T>
		static T& get_item(uintptr_t listPtr, int indx)
		{
			if(indx < 0 || indx >= *(int*)(listPtr + 0x18))
				throw std::exception("Index out of range.");
			uintptr_t items = *(uintptr_t*)(listPtr + 0x10);
			return *(T*)(items + 0x20 + sizeof(T) * indx);
		}
		/// <summary>
		/// Retrieves an item from an RE array
		/// </summary>
		/// <typeparam name="T">Type of item being retrieved</typeparam>
		/// <param name="arrayPtr">pointer to array</param>
		/// <param name="indx">index of item to retrieve</param>
		/// <returns>the item from the array at the specified index</returns>
		template<typename T>
		static T& get_array_item(uintptr_t arrayPtr, int indx)
		{
			return *(T*)(arrayPtr + 0x20 + sizeof(T) * indx);
		}

		static int get_list_count(uintptr_t dotNetList) noexcept
		{ return *(int*)(dotNetList + 0x18);
		}
		/// <summary>
		/// return how many items can fit in the list passed.
		/// </summary>
		/// <param name="dotNetList">The .Net List<T> being queried</param>
		/// <returns>returns the .net List<T>.Capacity</returns>
		static int get_list_capacity(uintptr_t dotNetList) noexcept
		{
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
		/// <summary>
		/// 
		/// </summary>
		/// <returns>address of game function</returns>
		uintptr_t get_address() const
		{
			return fAddr;
		}
		/// <summary>
		/// SDK call to get_thread_context.
		/// </summary>
		/// <param name="unk"></param>
		/// <returns></returns>
		virtual inline uintptr_t get_thread_context(uint32_t unk = -1)
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
