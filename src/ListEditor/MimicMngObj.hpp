#pragma once
#include <vector>
#include <stdexcept>
#include "AllocatedData.hpp"
// clang-format off
namespace GameListEditor
{
	//look to mngObj + 0x8 and mngObj + 0x18;
	template <typename T>
	class MimicMngObj
	{
	private:
		static constexpr uintptr_t COUNT_OFFS = 0x18;
		static constexpr uintptr_t MNG_OBJ_OFFS = 0x10;
		static constexpr uintptr_t ARR_LENGTH_OFFS = 0x1C;
		static constexpr uintptr_t BASE_DATA_OFFS = 0x20;

		std::vector<std::unique_ptr<AllocatedDataBase<T>>> *pDataList;
		uintptr_t listAddr = 0;
		uintptr_t origMngObjAddr = 0;
		uintptr_t origMngObjPtr = 0;
		size_t itemSize;
		int count = 0;
		int addCustomCount = 0;
		int origCount = 0;
		int origArrayLength = 0;
		uintptr_t listCountAddr = 0;
		uintptr_t arrLengthAddr = 0;
		size_t objSize = 0;
		void *mimicMngObj;
		bool isAllocated = false;
		bool isDataSwapped = false;

	protected:
		//Override this if u have list of val types like int and u are using it instead of pDataList
		virtual bool write_to_mimic_arr(size_t additionalOffset = 0)
		{
			uintptr_t addr = (uintptr_t)mimicMngObj; //for void* arithmetic
			//*(bool*)(addr+0x8) = 1;//wtf is this?
			//*(bool*)(addr + 0x18) = 1; //wtf is this?
			for (int i = 0; i < pDataList->size(); i++)
			{
				*(uintptr_t*)(addr + itemSize * i + BASE_DATA_OFFS + additionalOffset) = (uintptr_t)((*pDataList)[i].get()->get_allocated_data());
				count++;
			}
			return true;
		}

	public:
		MimicMngObj(size_t itemSize) : itemSize(itemSize) 
		{
			pDataList = new std::vector<std::unique_ptr<AllocatedDataBase<T>>>();
		}

		MimicMngObj(size_t itemSize, size_t vecReserve)
		{
			pDataList = new std::vector<std::unique_ptr<AllocatedDataBase<T>>>();
			pDataList->reserve(vecReserve);
		}

		~MimicMngObj()
		{
			restore_original_list_data();
			dealloc_mimic_arr();
			pDataList->clear();
			if (pDataList)
			{
				delete pDataList;
				pDataList = nullptr;
			}
		}

		inline uintptr_t get_list_addr() const { return listAddr; }
		inline uintptr_t get_game_mng_obj_addr() const { return origMngObjAddr; }
		inline uintptr_t get_game_mng_obj_ptr() const { return origMngObjPtr; }
		inline int get_orig_list_count() const { return origCount; }
		inline uintptr_t get_list_count_addr() const { return listCountAddr; }
		inline uintptr_t get_arr_length_addr() const { return arrLengthAddr; }
		inline void get_array_length(){return origArrayLength; }
		inline bool is_allocated() const {return isAllocated; }
		inline bool is_list_data_swapped() const {return isDataSwapped;}
		inline int get_allocated_count() const {return count; }
		inline int get_count() const
		{
			if (pDataList == nullptr)
				return 0;
			return pDataList->size();
		}
		inline int get_cutom_count() const {return addCustomCount; }
		inline size_t get_mimic_obj_size() const {return objSize; }

		//Set all necessary original list data to custom list. Return false withiout setting data if object already swapped. 
		bool set_game_list_data(uintptr_t gameListAddr)
		{
			if (!isDataSwapped)
			{
				listAddr = gameListAddr;
				listCountAddr = listAddr + COUNT_OFFS;
				origCount = *(int*)listCountAddr;
				origMngObjAddr = listAddr + MNG_OBJ_OFFS;
				origMngObjPtr = *(uintptr_t*)origMngObjAddr; //mngObj
				arrLengthAddr = origMngObjPtr + ARR_LENGTH_OFFS;
				origArrayLength = *(int*)(origMngObjPtr + ARR_LENGTH_OFFS);
				return true;
			}
			return false;
		}

		void add_alloc_data(AllocatedDataBase<T> *data)
		{
			if(data == NULL)
				throw std::runtime_error("data is NULL");
			auto uPtr = std::unique_ptr<AllocatedDataBase<T>>(data);
			pDataList->emplace_back(std::move(uPtr));
		}

		void remove(int indx)
		{
			pDataList->erase(pDataList->begin() + indx);
		}

		template <typename Td>
		bool set_to_mimic_obj(Td data, size_t offs)
		{
			if(!isAllocated)
				return false;
			*(Td*)((uintptr_t)mimicMngObj+offs) = data;
			return true;
		}

		AllocatedDataBase<T> *get_allocated_base_data(int indx)
		{
			return (*pDataList)[indx].get();
		}

		/// <summary>
		/// Allocate memory for mimic list
		/// </summary>
		/// <param name="additionalSize"></param>
		/// <param name="additionalStartOffset">Items will be placed with this offset</param>
		/// <param name="addCustomCount">Additional count for possible additional items</param>
		/// <returns></returns>
		bool alloc_mimic_arr(size_t additionalSize = 0, size_t additionalStartOffset = 0, int addCustomCount = 0)
		{
			if(pDataList == NULL)
				throw std::runtime_error("pDataList = NULL");
			size_t size = pDataList->size() * itemSize + BASE_DATA_OFFS + additionalSize + additionalStartOffset;
			mimicMngObj = malloc(size);
			if (mimicMngObj == NULL)
				return false;
			//count = pDataList->size();
			isAllocated = true;
			if (write_to_mimic_arr(additionalStartOffset))
			{
				this->addCustomCount = addCustomCount;
				objSize = size;
				*(int*)((uintptr_t)mimicMngObj + ARR_LENGTH_OFFS) = count + addCustomCount;
				return true;
			}
			else
				return false;
		}

		bool dealloc_mimic_arr()
		{
			if(!isAllocated)
				return false;
			if(mimicMngObj = NULL)
				return false;
			free(mimicMngObj);
			isAllocated = false;
			count = 0;
			return true;
		}

		void alloc_all_data(size_t additionalSize = 0x0)
		{
			for (int i = 0; i < pDataList->size(); i++)
			{
				if ((*pDataList)[i].get()->is_game_data_allocated())
					continue;
				(*pDataList)[i].get()->alloc_data(additionalSize);
			}

		}

		void dealloc_all_data()
		{
			restore_original_list_data();
			for (int i = 0; i < pDataList->size(); i++)
			{
				if(!(*pDataList)[i].get()->is_game_data_allocated())
					continue;
				(*pDataList)[i].get()->dealloc_data();
			}
		}

		bool swap_original_list_data()
		{
			if(!isAllocated)
				return false;
			*(int*)arrLengthAddr = count + addCustomCount;
			*(int*)listCountAddr = count + addCustomCount;
			*(uintptr_t*)origMngObjAddr = (uintptr_t)mimicMngObj;
			isDataSwapped = true;
			return true;
		}

		bool restore_original_list_data()
		{
			if(!isDataSwapped)
				return false;
			*(int*)listCountAddr = origCount;
			*(int*)arrLengthAddr = origArrayLength;
			*(uintptr_t*)origMngObjAddr = origMngObjPtr;
			isDataSwapped = false;
			return true;
		}
		//void* of allocated mimic list
		inline void *get_mimic_obj() {return mimicMngObj;}

		/// <summary>
		/// Include mimicObj itself
		/// </summary>
		/// <returns></returns>
		bool is_all_allocated()
		{
			if(!isAllocated)
				return false;
			for (int i = 0; i < pDataList->size(); i++)
			{
				if(!(*pDataList)[i].get()->is_game_data_allocated())
					return false;
			}
			return true;
		}
	};
}
// clang-format on