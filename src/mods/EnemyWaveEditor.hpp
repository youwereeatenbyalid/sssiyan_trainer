#pragma once

#pragma comment(lib, "../lib/jsonstthm.lib")

#include "Mod.hpp"
#include "EnemySwapper.hpp"
#include <string>
#include "ListEditor/AllocatedData.hpp"
#include "ListEditor/MimicMngObj.hpp"
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "JsonStthm.h"
#include <filesystem>
#include <map>
#include "ImGuiExtensions/ImGuiExtensions.h"

namespace json = JsonStthm;

namespace WaveEditorMod
{
	struct SetEmData
	{
		int selectedItem;
		int emId;
		//void *commentPtr;
		int num = 1;
		float odds = 100.0f;
		bool isBoss = false;
		bool isNoDie = false;
		bool isNearPlayer = false;
		bool isDontSetOrb = false;
		float waitTimeMin = 0;
		float waitTimeMax = 0;
		int greenOrbAddType;
		int whiteOrbAddType;
		int addParamIndex;
		bool isAddParamSetPlayer = false;
		//void *AddParamIndexDataListPtr;
		bool isNetworkPlayerLimit;
		int playerLimit;
		//void *eventDataPtr;
		//void *posDataListPtr;
		//void *presetNoPtr;
		//void *hpRatePtr;
		//void *attackRatePtr;

		void set_default()
		{
			emId = 0;
			num = 1;
			odds = 100.0f;
			isBoss = false;
			isNoDie = false;;
			isNearPlayer = false;
			isDontSetOrb = false;
			waitTimeMin = 0;
			waitTimeMax = 0;
			isAddParamSetPlayer = false;
		}
	};

	class AllocatedEmData : public GameListEditor::AllocatedDataBase<SetEmData>
	{
		using AllocatedDataBase::AllocatedDataBase;
		bool AllocatedEmData::write_to_allocated_data() override
		{
			uintptr_t addr = (uintptr_t)AllocatedDataBase::allocatedData;
			*(int*)(addr + 0x10) = AllocatedDataBase::data.emId;
			*(int*)(addr + 0x20) = AllocatedDataBase::data.num;
			*(float*)(addr + 0x24) = AllocatedDataBase::data.odds;
			*(bool*)(addr + 0x28) = AllocatedDataBase::data.isBoss;
			*(bool*)(addr + 0x29) = AllocatedDataBase::data.isNoDie;
			*(bool*)(addr + 0x2A) = AllocatedDataBase::data.isNearPlayer;
			*(bool*)(addr + 0x2B) = AllocatedDataBase::data.isDontSetOrb;
			*(float*)(addr + 0x2C) = AllocatedDataBase::data.waitTimeMin;
			*(float*)(addr + 0x30) = AllocatedDataBase::data.waitTimeMax;
			//int greenOrbAddType - 0x34;
			//int whiteOrbAddType - 0x38;
			//int addParamindex - 0x3C;
			//  *(bool*)(addr + 0x40)   = customData[i].isAddParamSetPlayer;//Actually need to copy this from original data
	  //  //List<SetEnemyData.AddParamIndexData> - 0x48;
			*(bool*)(addr + 0x50) = false;//customData[i].isNetworkPlayerLimit;
			*(int*)(addr + 0x54) = 4;//customData[i].playerLimit;
			//EventSkipData - 0x58;
			//List<PositionData> - 0x60;
			//List<int> presetNo - 0x68;
			//List<float> HpRate - 0x70;
			//List<float> attackRate - 0x78;
			return true;
		}

	public:
		AllocatedEmData(const AllocatedEmData& obj) : AllocatedEmData(new SetEmData(obj.data), obj.dataSize)
		{
		}
	};

	struct EmData
	{
		int emId;
		int num;

		bool operator ==(const EmData& other)
		{
			if (emId == other.emId && num == other.num)
				return true;
			else return false;
		}
		bool operator !=(const EmData& other)
		{
			return !(this->operator==(other));
		}
	};

	struct GameEmList
	{
		std::vector<EmData> emDataInfo;
		int loadId = 0;
		int count;
		uintptr_t listAddr = 0;

		bool operator ==(const GameEmList& other)
		{
			if (count != other.count)
				return false;
			else
			{
				for (int i = 0; i < count; i++)
				{
					if (emDataInfo[i] != other.emDataInfo[i])
						return false;
				}
				return true;
			}
		}
	};

	struct HandleMimicObj
	{
		int loadId = 0;
		GameListEditor::MimicMngObj<SetEmData> mimicList { (size_t)0x8 };
		bool isUsedByRtEdit = false;
		GameEmList *bindedEmData = nullptr;
		bool isSwapped = false;
	};

	class MimicMngObjManager
	{
	private:
		std::string constLabelName;
		std::vector<std::shared_ptr<HandleMimicObj>> mimicObjectsList;
		std::vector<std::string> objNames;
		bool isAllAllocated;
		bool isAllDataSwapped;
		bool isBindMode;
		std::map<int, int> emIds;// Key - emId, T - num of occurs;
		const size_t DEFAULT_ADD_OFFSET = 0x10;

		inline bool check_all_allocated()
		{
			if (mimicObjectsList.empty())
			{
				isAllAllocated = false;
				return false;
			}
			for (auto& mimic : mimicObjectsList)
			{
				if (!mimic.get()->mimicList.is_all_allocated())
				{
					isAllAllocated = false;
					return false;
				}
			}
			isAllAllocated = true;
			return true;
		}

		inline void recheck_emids()
		{
			emIds.clear();
			for (const auto& lst : mimicObjectsList)
			{
				for (int i = 0; i < lst->mimicList.get_count(); i++)
					emIds[lst->mimicList.get_allocated_base_data(i)->get_data()->emId]++;
			}
		}

		void bind_emdata_to_mimic_obj(std::shared_ptr<HandleMimicObj> &obj, std::vector<GameEmList> *emList)
		{			
			if (emList == nullptr)
			{
				obj->bindedEmData = nullptr;
				isBindMode = false;
				return;
			}
			for (auto &emData : *emList)
			{
				if (obj->loadId == emData.loadId)
				{
					obj->bindedEmData = &emData;
					isBindMode = true;
					return;
				}
			}
			obj->bindedEmData = nullptr;
		}

		bool is_already_in_list(int loadId, int objIndx)
		{
			for (int i = 0; i < mimicObjectsList.size(); i++)
			{
				if (i != objIndx)
				{
					if (mimicObjectsList[i]->loadId == loadId)
						return true;
				}
			}
			return false;
		}

		void update_idstr(std::string &newStr, int newId, int uniq_indx = 0)
		{
			for (const auto& str : objNames)
			{
				if (str == newStr)
				{
					newStr = std::string(constLabelName + std::to_string(newId) + " ##" + std::to_string(uniq_indx));
					uniq_indx++;
					update_idstr(newStr, newId, uniq_indx);
				}
			}
		}

	public:
		MimicMngObjManager()
		{
			isAllAllocated = false;
			constLabelName = "Load Id = ";
			isAllDataSwapped = false;
			isBindMode = false;
		}

		inline std::map<int, int> const* get_emids()
		{
			return &emIds;
		}

		inline void copy_to(int from, int to, bool byId = true)
		{
			auto lstFrom = get_mimic_list_ptr(from, byId);
			if (lstFrom == nullptr)
				return;
			for (int i = 0; i < lstFrom->mimicList.get_count(); i++)
				get_mimic_list_ptr(to, byId)->mimicList.add_alloc_data(new AllocatedEmData(*(static_cast<AllocatedEmData*>(lstFrom->mimicList.get_allocated_base_data(i)))));
		}

		inline void alloc_all(std::vector<GameEmList> *emList = nullptr)
		{
			for (auto& mimic : mimicObjectsList)
			{
				mimic.get()->mimicList.alloc_all_data();
				mimic.get()->mimicList.alloc_mimic_arr(0);
				bind_emdata_to_mimic_obj(mimic, emList);
			}
			check_all_allocated();
			recheck_emids();
		}

		inline void dealloc_all()
		{
			for (auto& mimic : mimicObjectsList)
			{
				mimic.get()->mimicList.dealloc_all_data();
				mimic.get()->mimicList.dealloc_mimic_arr();
				mimic->isUsedByRtEdit = false;
				mimic->isSwapped = false;
			}
			check_all_allocated();
			emIds.clear();
			remove_all_binds();
		}

		inline void restore_all_data()
		{
			for (auto& mimic : mimicObjectsList)
			{
				mimic.get()->mimicList.restore_original_list_data();
				mimic->isUsedByRtEdit = false;
				mimic->isSwapped = false;
			}
			isAllDataSwapped = false;
		}

		inline void set_const_label(const char* name)
		{
			constLabelName = name;
		}

		bool add(int loadId)
		{
			for (const auto& item : mimicObjectsList)
			{
				if (item.get()->loadId == loadId)
					return false;
			}
			auto pObj = new HandleMimicObj();
			pObj->loadId = loadId;
			std::shared_ptr<HandleMimicObj> uPtr(pObj);
			mimicObjectsList.push_back(std::move(uPtr));
			auto str = std::string(constLabelName + std::to_string(loadId));
			objNames.push_back(str);
			return true;
		}

		void add_data(AllocatedEmData* data, int listIndx)
		{
			mimicObjectsList[listIndx].get()->mimicList.add_alloc_data(data);
		}

		inline void remove_last()
		{
			if (mimicObjectsList.empty())
				return;
			mimicObjectsList.pop_back();
			objNames.pop_back();
			recheck_emids();
		}

		inline void remove_at(int indx)
		{
			if (mimicObjectsList.empty() || indx >= mimicObjectsList.size() || indx < 0)
				return;
			mimicObjectsList.erase(mimicObjectsList.begin() + indx);
			objNames.erase(objNames.begin() + indx);
			recheck_emids();
		}

		inline void remove_all()
		{
			mimicObjectsList.clear();
			objNames.clear();
			isAllAllocated = false;
			emIds.clear();
			isBindMode = false;
		}

		inline auto get_names_data()
		{
			return objNames.data();
		}

		inline const std::string* get_list_name(int indx) const
		{
			return &objNames[indx];
		}

		std::shared_ptr<HandleMimicObj> get_mimic_list_ptr(int indx, bool byLoadId = false)
		{
			if (!byLoadId)
				return mimicObjectsList[indx];
			else
			{
				for (auto& mimic : mimicObjectsList)
				{
					if (mimic.get()->loadId == indx)
						return mimic;
				}
				return nullptr;
			}
		}

		inline int count() const
		{
			return mimicObjectsList.size();
		}

		inline bool is_all_allocated() const
		{
			return isAllAllocated;
		}

		inline bool is_all_data_swapped(bool recheck = true)
		{
			if (!recheck)
				return isAllDataSwapped;
			for (auto& mimic : mimicObjectsList)
			{
				if (!mimic.get()->mimicList.is_list_data_swapped())
				{
					isAllDataSwapped = false;
					return false;
				}
			}
			isAllDataSwapped = true;
			return true;
		}

		inline void change_loadid(int indx, int newId)
		{
			std::string newStr = std::string(constLabelName + std::to_string(newId));
			update_idstr(newStr, newId);
			objNames[indx] = newStr;
			mimicObjectsList[indx]->loadId = newId;
		}

		inline void bind_game_em_data(GameEmList *emData, int mimicIndx)
		{
			if(emData == nullptr)
				return;
			mimicObjectsList[mimicIndx]->bindedEmData = emData;
			isBindMode = true;
		}

		void remove_all_binds()
		{
			for(auto &obj: mimicObjectsList)
				obj->bindedEmData = nullptr;
			isBindMode = false;
		};

		bool is_all_id_uniq()
		{
			for (int i = 0; i < mimicObjectsList.size(); i++)
			{
				if(is_already_in_list(mimicObjectsList[i]->loadId, i))
					return false;
			}
			return true;
		}

		bool is_bind_mode() const {return isBindMode; }
	};

	class EnemyWaveEditor : public Mod
	{
	public:
		static std::mutex mt;
		static inline std::mutex mtx;
		enum Mode
		{
			Mod,
			ViewUserData,
			ReadGameData,
			//RunTimeEdit,
			Serialization
		};
		static Mode mode;

		const int SET_ENEMY_DATA_SIZE = 0x80;

		static std::condition_variable cv;

		static std::atomic_int emReaderCounter;
		static std::atomic_int emSetterCounter;
		static int emListId;
		static int selectedMimicListItem;
		static int addListInRangeMin;
		static int addListInRangeMax;
		static inline uint32_t bpFlowId = 0;

		static bool cheaton;
		static bool isAllAllocCorrectly;
		static bool forceLoadAll;
		static bool isPfbLoadRequested;
		static bool isBPFixRequested;
		static bool isBpAllocation;
		static bool isRequestEndBpStage; //30

		static uintptr_t retJmp;
		static uintptr_t retJl;
		static uintptr_t curListAddr;
		static uintptr_t prefabLoadJmp;
		static uintptr_t bpRetJmp;
		static uintptr_t fadeStaticBase;
		static uintptr_t missionExitProcRet;
		static uintptr_t missionExitProcRet1;

		
		static MimicMngObjManager mimObjManager;
		static SetEmData curEmData;
		static std::vector<GameEmList> gameDataList;

		EnemyWaveEditor() = default;

		std::string_view get_name() const override
		{
			return "EnemyWaveEditor";
		}
		std::string get_checkbox_name() override
		{
			return m_check_box_name;
		};
		std::string get_hotkey_name() override
		{
			return m_hot_key_name;
		};

		std::optional<std::string> on_initialize() override;

		// Override this things if you want to store values in the config file
		void on_config_load(const utility::Config& cfg) override;
		void on_config_save(utility::Config& cfg) override;

		// on_frame() is called every frame regardless whether the gui shows up.
		void on_frame() override;
		// on_draw_ui() is called only when the gui shows up
		// you are in the imgui window here.
		void on_draw_ui() override;
		// on_draw_debug_ui() is called when debug window shows up
		void on_draw_debug_ui() override;

		static void handle_emlist_asm(uintptr_t lstAddr);
		static void load_prefabs_asm(uintptr_t prefabManagerAddr);
		static void restore_list_data_asm();

	private:

		inline static const std::array<const char*, 4> modeNames {
			"Add/change enemy data",
			"View all custom data",
			"Read game enemy data",
			//"Run-time edit",
			"Import profile"
		};

		enum SetupEmMode
		{
			First,
			Last,
		};

		static SetupEmMode setupEmMode;
		static inline bool isDefaultGameData = true;
		static inline std::string jsonEmDataPath = "";//std::filesystem::current_path().string() + "\\" + "emListsProfile.json";
		SetEmData curCustomEmData;
		int selectedEmDataItem;
		void init_check_box_info() override;
		void print_emdata_input(SetEmData& data);
		void draw_mimic_list_ui();
		void draw_emlist_combo();
		void draw_mimiclist_items(int i);
		void print_emlist_data();
		void emlist_btn();
		static void clear_emlist();
		static void edit_gamelists_asm(uintptr_t lstAddr);
		int get_em_id(int selected);
		static int get_em_name_indx(int emId);
		void to_clipboard(const std::string& s);
		void draw_copy_list_data();
		void print_spacing(const char* ch);
		static void read_em_data_asm(uintptr_t lstAddr);
		static void set_em_data_asm(std::shared_ptr<HandleMimicObj>& curHandleObj, uintptr_t lstAddr);
		void bpmode_data_setup();
		static GameEmList get_game_emdata_asm(uintptr_t lstAddr, bool incReaderCounter);
		int fromLst = 0;
		int toLst = 0;
		int selectedProfile = 0;
		int prevGameDataCount = 0;
		int gameDataIndx = 0;
		std::unique_ptr<FunctionHook> m_emwave_hook;
		std::unique_ptr<FunctionHook> m_loadall_hook;
		std::unique_ptr<FunctionHook> m_bploadflow_hook;
		std::unique_ptr<FunctionHook> m_exitproc_hook;
		std::unique_ptr<FunctionHook> m_exitproc1_hook;


		class ProfileManager
		{
		private:
			std::vector<std::string> profileNames;
			json::JsonValue data;
			const char* jsonEmDataPath;
		public:

			bool is_data_read()
			{
				return !profileNames.empty();
			}

			bool read_data(const char* path)
			{
				profileNames.clear();
				try 
				{
					data.ReadFile(path);
					for (int i = 0; i < data.GetMemberCount(); i++)
						profileNames.push_back(data[i]["SetupName"].ToString());
				}
				catch(std::exception exc){return false; }
				return !profileNames.empty();
			}

			void import_data(int setupNameIndx)
			{
				if (!mimObjManager.is_all_allocated())
				{
					mimObjManager.remove_all();
					if (data[setupNameIndx]["GameEmListHandler"]["GameEmList"].GetMemberCount() != 0)
					{
						int count = data[setupNameIndx]["GameEmListHandler"]["GameEmList"].GetMemberCount();
						gameDataList.clear();
						EmData curEmData;

						for (int i = 0; i < count; i++)
						{
							GameEmList curGameList;
							curGameList.loadId = data[setupNameIndx]["GameEmListHandler"]["GameEmList"][i]["LoadId"].ToInteger();
							int emCount = data[setupNameIndx]["GameEmListHandler"]["GameEmList"][i]["EmDataList"].GetMemberCount();
							for (int j = 0; j < emCount; j++)
							{
								curEmData.emId = data[setupNameIndx]["GameEmListHandler"]["GameEmList"][i]["EmDataList"][j]["EmId"].ToInteger();
								curEmData.num = data[setupNameIndx]["GameEmListHandler"]["GameEmList"][i]["EmDataList"][j]["Num"].ToInteger();
								curGameList.emDataInfo.push_back(curEmData);
							}
							curGameList.listAddr = 0;
							curGameList.count = curGameList.emDataInfo.size();
							gameDataList.push_back(curGameList);
						}
					}

					for (int i = 0; i < data[setupNameIndx]["MimicObjectsList"].GetMemberCount(); i++)
					{
						int loadId = data[setupNameIndx]["MimicObjectsList"][i]["LoadId"].ToInteger();
						mimObjManager.add(loadId);
						SetEmData emData;
						for (int j = 0; j < data[setupNameIndx]["MimicObjectsList"][i]["MimicList"].GetMemberCount(); j++)
						{
							emData.emId = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["EmId"].ToInteger();
							emData.selectedItem = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["SelectedItem"].ToInteger();
							emData.num = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["Num"].ToInteger();
							emData.odds = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["Odds"].ToFloat();
							emData.isBoss = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["IsBoss"].ToBoolean();
							emData.isNoDie = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["IsNoDie"].ToBoolean();
							emData.isDontSetOrb = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["IsDontSetOrbs"].ToBoolean();
							emData.isNearPlayer = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["IsNearPlayer"].ToBoolean();
							emData.waitTimeMin = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["WaitTimeMin"].ToFloat();
							emData.waitTimeMax = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["WaitTimeMax"].ToFloat();
							mimObjManager.add_data(new AllocatedEmData(&emData, 0x80), i);
						}
					}
					profileNames.clear();
					data.Reset();
				}
			}

			int count() const
			{
				return profileNames.size();
			}

			std::string* const get_profile_name(int indx)
			{
				return &profileNames[indx];
			}
		};
		ProfileManager prflManager {};
	};
}



