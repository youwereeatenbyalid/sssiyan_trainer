#include <vector>
#include <memory>
#include <cstdlib>
#include <stdexcept>

/*
* Usually IL2Cpp builded apps uses next offsets for collections:
* 0x18 - Count offset;
* 0x10 - items offset;
* ----In "items" (System.Array or smth)---/
* 0x1C or 0x18 - Capacity offset;
* 0x20 - first item offset;
*/

namespace NetListHook
{
	template <typename T>
	class MimicList
	{
	private:
		const uintptr_t MNG_OFFS = 0x10;

		uintptr_t lstAddr = 0;
		uintptr_t itemsOffs = 0;
		uintptr_t countOffs = 0;
		uintptr_t capacityOffs = 0;
		uintptr_t firstItemOffs = 0;
		uintptr_t itemsPtr = 0;
		uintptr_t itemsAddr = 0;

		size_t allocSize = 0;
		size_t itemSize = 0;

		unsigned int netListCount = 0;
		unsigned int netListCapacity = 0;
		unsigned int allocCount = 0;

		volatile int* pNetListCount = nullptr;
		volatile int* pNetListCapacity = nullptr;

		unsigned int* pMimicCapacity = nullptr;

		bool isSwapped = false;

		void* netArray = nullptr;

		T* firstItem = nullptr;

		void arr_alloc(int capacity)
		{
			size_t size = itemSize * capacity + firstItemOffs;
			netArray = std::malloc(size);
			if (netArray == nullptr)
				throw std::bad_alloc();
			allocSize = size;
			pMimicCapacity = (unsigned int*)((uintptr_t)netArray + capacityOffs);
			*pMimicCapacity = capacity;
			firstItem = (T*)((uintptr_t)netArray + firstItemOffs);
		}

		MimicList(int itemSize, int count, int capacity)
		{
			arr_alloc(itemSize, count, capacity);
		}

		void free_data()
		{
			std::free(netArray);
			allocSize = 0;
			allocCount = 0;
			netArray = nullptr;
			pMimicCapacity = nullptr;
			firstItem = nullptr;
		}

		void update_ptrs()
		{
			if (netArray == nullptr)
				return;
			pMimicCapacity = (unsigned int*)((uintptr_t)netArray + capacityOffs);
			firstItem = (T*)((uintptr_t)netArray + firstItemOffs);
			if (isSwapped)
			{
				*pNetListCount = allocCount;
				*(uintptr_t*)itemsAddr = (uintptr_t)netArray;
			}
		}

	public:

		MimicList(uintptr_t countOffset, uintptr_t itemsOffset, uintptr_t capacityOffset, uintptr_t firstItemOffset, uintptr_t mng_offs = 0x10) : countOffs(countOffset), itemsOffs(itemsOffset), capacityOffs(capacityOffset), firstItemOffs(firstItemOffset),
			itemSize(sizeof(T))
		{
			arr_alloc(4);
		}

		MimicList(uintptr_t countOffset, uintptr_t itemsOffset, uintptr_t capacityOffset, uintptr_t firstItemOffset, std::initializer_list<T> list, uintptr_t mng_offs = 0x10) : MimicList(countOffset, itemsOffset, capacityOffset,
			firstItemOffset, mng_offs)
		{
			for (const auto& data : list)
				emplace_back(data);
		}

		void add(const T& data)
		{
			if (netArray == nullptr)
			{
				arr_alloc(4);
			}
			else if (*pMimicCapacity == allocCount)
			{
				auto newCap = *pMimicCapacity + (*pMimicCapacity / 2);
				set_capacity(newCap);
			}
			firstItem[allocCount++] = data;
			update_ptrs();
		}

		template <typename ...Args>
		T& emplace_back(Args&&... args)
		{
			if (netArray == nullptr)
			{
				arr_alloc(4);
			}
			else if (*pMimicCapacity == allocCount)
			{
				auto newCap = *pMimicCapacity + (*pMimicCapacity / 2);
				set_capacity(newCap);
			}
			firstItem[allocCount] = T(std::forward<Args>(args)...);
			return firstItem[allocCount++];
		}

		//Remove last item from the list
		void remove()
		{
			if (allocCount == 0 || netArray == nullptr)
				return;
			firstItem[--allocCount].~T();// Actually make no sence 'cause non-num stuff must be always pass as a pointer
			update_ptrs();
		}

		void set_capacity(unsigned int capacity)
		{
			if (capacity < 0)
				return;
			allocSize = firstItemOffs + itemSize * capacity;
			auto newMem = malloc(allocSize);
			if (newMem == nullptr)
				throw std::bad_alloc();
			if (capacity < allocCount)
				allocCount = capacity;
			for (int i = 0; i < allocCount; i++)
				*(T*)((uintptr_t)newMem + firstItemOffs + itemSize * i) = firstItem[i];
			std::free(netArray);
			netArray = newMem;
			update_ptrs();
			*pMimicCapacity = capacity;
		}

		unsigned int get_capacity() const
		{
			if (pMimicCapacity != nullptr)
				return pMimicCapacity;
			return 0;
		}

		void remove_at(int indx)
		{
			if (indx >= allocCount || indx < 0 || allocCount < 0)
				throw std::out_of_range("Out of range");
			if (indx == allocCount - 1)
				remove();
			else
			{
				//memcpy(&(firstItem[indx]), &(firstItem[indx + 1]), (allocCount - indx + 1) * itemSize);
				firstItem[indx].~T();
				for (int i = indx; i < allocCount - 1; i++)
					firstItem[i] = std::move(firstItem[i + 1]);
				allocCount--;
				update_ptrs();
			}

		}

		~MimicList()
		{
			restore();
			free_data();
			pNetListCapacity = nullptr;
			pNetListCount = nullptr;
			pMimicCapacity = nullptr;
		}

		T& operator [](int indx)
		{
			if (netArray == nullptr || indx >= allocCount || indx < 0)
				throw std::out_of_range("Out of range");
			return firstItem[indx];
		}

		const T& operator [](int indx) const
		{
			if (netArray == nullptr || indx >= allocCount || indx < 0)
				throw std::out_of_range("Out of range");
			return firstItem[indx];
		}

		//Swap original list's data.
		void swap()
		{
			if (lstAddr == 0)
				return;
			set_list_data(lstAddr);//Update original data;
			*(int*)(lstAddr + countOffs) = allocCount;
			*(uintptr_t*)itemsAddr = (uintptr_t)netArray;
			memcpy(netArray, (const void*)itemsPtr, MNG_OFFS);
			isSwapped = true;
		}

		//Restore original list's data;
		void restore()
		{
			if (!isSwapped)
				return;
			*(int*)(lstAddr + countOffs) = netListCount;
			*(int*)(itemsPtr + capacityOffs) = netListCapacity;
			*(uintptr_t*)itemsAddr = itemsPtr;
			isSwapped = false;
		}

		//Get all necessary info about original list (call this before swap());
		void set_list_data(uintptr_t listAddr)
		{
			if (netArray == nullptr || isSwapped)
				return;
			/*if (isSwapped)
				throw std::exception("Pointers was already swapped. Restore data before load info from another list.");*/
			lstAddr = listAddr;
			netListCount = *(int*)(lstAddr + countOffs);
			pNetListCount = (int*)(lstAddr + countOffs);
			itemsPtr = *(uintptr_t*)(lstAddr + itemsOffs);
			itemsAddr = lstAddr + itemsOffs;
			netListCapacity = *(int*)(itemsPtr + capacityOffs);
			pNetListCapacity = (int*)(itemsPtr + capacityOffs);
		}

		void clear()
		{
			for (int i = 0; i < allocCount; i++)
				firstItem[i].~T();
			allocCount = 0;
			update_ptrs();
		}

		//Was data in original list swapped;
		bool is_swapped() const
		{
			return isSwapped;
		}

		////Returns List.Capacity, that list had after set_list_data() or swap() was called;
		unsigned int get_net_list_capacity() const noexcept
		{
			return netListCapacity;
		}

		//Returns List.Count, that list had after set_list_data() or swap() was called;
		unsigned int get_net_list_count() const noexcept
		{
			return netListCount;
		}

		//Returns deferenced pointer to List.Count;
		int get_list_count() const
		{
			if (pNetListCount == nullptr)
				throw std::exception("nullptr to List.Count");
			return *pNetListCount;
		}

		//Returns num of elements in hooked list;
		int get_mimic_count() const
		{
			return allocCount;
		}

		//Address of original list;
		uintptr_t get_list_addr() const noexcept
		{
			return lstAddr;
		}

		//List<T>.items
		uintptr_t get_items_ptr() const noexcept
		{
			return itemsPtr;
		}

		//List.items address
		uintptr_t get_items_addr() const noexcept
		{
			return itemsAddr;
		}

		//Returns size of allocated memory for hooked list;
		size_t get_itemsmem_size() const noexcept
		{
			return allocSize;
		}

		//Get pointer to mimic dotnet array
		void* get_mimic_items()
		{
			return netArray;
		}
	};
}