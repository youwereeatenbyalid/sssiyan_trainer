#pragma once
#include <vector>
// clang-format off 
namespace GameListEditor
{
template <typename T>
	/// <summary>
	/// Inherit from it and override write_to_allocated_data() for your data struct
	/// </summary>
	/// <typeparam name="T">Custom list's item</typeparam>
	class AllocatedDataBase
	{
	protected:
		virtual bool write_to_allocated_data() = 0;

		T data;
		void* allocatedData;
		bool isAllocated;
		size_t dataSize;

	public:
		void const *get_allocated_data() const
		{
			if(!isAllocated)
				return NULL;
			return allocatedData;
		}

		AllocatedDataBase(const T *data, size_t gameDataSize)
		{
			this->data = *data;
			dataSize = gameDataSize;
			isAllocated = false;
		}

		AllocatedDataBase(size_t gameDataSize) : dataSize(gameDataSize) 
		{
			 isAllocated = false;
		}

		~AllocatedDataBase()
		{
			dealloc_data();
		}

		void set_data(const T *data)
		{
			this->data = *data;
		}

		template <typename T_data>
		bool set_to_allocated_data(T_data data, int offset)
		{
			if(!isAllocated)
				return false;
			uintptr_t addr = (uintptr_t)allocatedData;
			*(T_data*)(addr + offset) = data;
			return true;
		}

		bool alloc_data(size_t additional_size = 0x0)
		{
			if(isAllocated)
				return false;
			allocatedData = malloc(dataSize + additional_size);
			if(allocatedData == NULL)
				return false;
			if(!write_to_allocated_data())
				return false;
			isAllocated = true;
			return true;
		}

		bool dealloc_data()
		{
			if(!isAllocated)
				return false;
			free(allocatedData);
			isAllocated = false;
			return true;
		}

		bool is_game_data_allocated() const {return isAllocated; }

		T *get_data(){return &data; }
	};
}
// clang-format on