#pragma once

//#pragma comment(lib, "../lib/jsonstthm.lib")

#include "Mod.hpp"
#include "EnemyData.hpp"
#include "ListEditor/MimicList.hpp"
#include <mutex>
#include <condition_variable>
#include "JsonStthm.h"
#include <filesystem>
#include <random>
#include <map>
#include <sstream>
#include "ImGuiExtensions/ImGuiExtensions.h"
#include "GameplayStateTracker.hpp"
#include "EndLvlHooks.hpp"
#include <algorithm>
#include "sdk/DMC5.hpp"

#define SELECTABLE_STYLE_ACT	ImVec4(0.26f, 0.39f, 0.58f, 0.41f)
#define SELECTABLE_STYLE_HVR	ImVec4(0.26f, 0.59f, 0.98f, 0.61f)
#define SELECTABLE_STYLE_ACT_HVR	ImVec4(0.26f, 0.59f, 0.98f, 0.91f) 

namespace json = JsonStthm;

namespace WaveEditorMod
{
	const ptrdiff_t COUNT_OFFS = 0x18;
	const ptrdiff_t ITEMS_OFFS = 0x10;
	const ptrdiff_t ARR_LENGTH_OFFS = 0x1C;
	const ptrdiff_t FIRST_ITEM_OFFS = 0x20;

	struct SetEmData
	{
		char mng[0x10];
		int emId;//0x10
		int idk = 0;// -
		uintptr_t strComment;//0x18
		int num = 1;//0x20
		float odds = 100.0f;//0x24
		bool isBoss = false;//0x28
		bool isNoDie = false;//0x29
		bool isNearPlayer = false;//0x2A
		bool isDontSetOrb = false;//0x2B
		float waitTimeMin = 0;//0x2C
		float waitTimeMax = 0;//0x30
		int greenOrbAddType;//0x34
		int whiteOrbAddType;//0x38
		int addParamIndex;//0x3C
		bool isAddParamSetPlayer = false;//0x40
		char idk2[0x7];
		uintptr_t AddParamIndexDataListPtr;//0x48
		bool isNetworkPlayerLimit;//0x50
		char idk3[0x2];
		int playerLimit;//0x54
		uintptr_t eventDataPtr;//0x58
		uintptr_t posDataListPtr;//0x60
		uintptr_t presetNoPtr;//0x68
		uintptr_t hpRatePtr;//0x70
		uintptr_t attackRatePtr;//0x78
		int selectedItem;//Non-class stuff

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

	struct MimicListData
	{
		int loadId = 0;
		NetListHook::MimicList <SetEmData*> mimicList{COUNT_OFFS, ITEMS_OFFS, ARR_LENGTH_OFFS, FIRST_ITEM_OFFS};
		GameEmList *bindedEmData = nullptr;
		bool isSwapped = false;

		void remove_em(int indx)
		{
			if(indx < 0 || indx > mimicList.get_mimic_count())
				return;
			delete mimicList[indx];
			mimicList.remove_at(indx);
		}

		void clear_em_list()
		{
			for (int i = 0; i < mimicList.get_mimic_count(); i++)
				delete mimicList[i];
			mimicList.clear();
			mimicList.set_capacity(4);
		}
	};

	class MimicListManager
	{
	private:
		std::string constLabelName;

		std::vector<std::shared_ptr<MimicListData>> mimicObjectsList;
		std::vector<std::string> objNames;
		std::vector<GameEmList> gameDataList;

		bool isAnyDataSwapped = false;
		bool isAllDataSwapped;
		bool isBindMode;

		std::map<int, int> emIds;// Key - emId, T - num of occurs;
		const size_t DEFAULT_ADD_OFFSET = 0x10;

		void bind_emdata()
		{
			if (gameDataList.empty())
			{
				remove_all_binds();
				return;
			}
			for (auto& mList : mimicObjectsList)
			{
				for (auto& emDt : gameDataList)
				{
					if (emDt.loadId == mList->loadId)
					{
						mList->bindedEmData = &emDt;
						isBindMode = true;
						break;
					}
				}
			}
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
		MimicListManager()
		{
			constLabelName = "Load Id = ";
			isAllDataSwapped = false;
			isBindMode = false;
		}

		//Call this once before swap all data
		void recheck_emids()
		{
			emIds.clear();
			for (const auto& lst : mimicObjectsList)
			{
				for (int i = 0; i < lst->mimicList.get_mimic_count(); i++)
					emIds[lst->mimicList[i]->emId]++;
			}
		}

		inline std::map<int, int> const* get_emids()
		{
			return &emIds;
		}

		void copy_to(int from, int to, bool byId = true)
		{
			auto lstFrom = get_mimic_list_data(from, byId);
			if (lstFrom == nullptr)
				return;
			auto lstTo = get_mimic_list_data(to, byId);
			if(from == to)//no mem leak allowed
				return;
			for (int i = 0; i < lstFrom->mimicList.get_mimic_count(); i++)
				lstTo->mimicList.emplace_back(new SetEmData(*(lstFrom->mimicList[i])));
		}

		void restore_all_data()
		{
			for (auto& mimic : mimicObjectsList)
			{
				mimic.get()->mimicList.restore();
				mimic->isSwapped = false;
			}
			isAllDataSwapped = false;
			isAnyDataSwapped = false;
		}

		void set_const_label(const char* name)
		{
			constLabelName = name;
		}

		bool add(int loadId)
		{
			for (const auto& item : mimicObjectsList)
			{
				if (item->loadId == loadId)
					return false;
			}
			mimicObjectsList.emplace_back(std::make_shared<MimicListData>());
			mimicObjectsList.back()->loadId = loadId;
			auto str = std::string(constLabelName + std::to_string(loadId));
			objNames.push_back(str);
			bind_emdata();
			return true;
		}

		void add_data(SetEmData* data, int listIndx)
		{
			mimicObjectsList[listIndx]->mimicList.add(data);
		}

		void add_game_em_data(const GameEmList &data)
		{
			gameDataList.push_back(data);
			bind_emdata();
		}

		void clear_game_em_data()
		{
			remove_all_binds();
			gameDataList.clear();
		}

		void remove_last()
		{
			if (mimicObjectsList.empty())
				return;
			mimicObjectsList[mimicObjectsList.size() - 1]->clear_em_list();
			mimicObjectsList.pop_back();
			objNames.pop_back();
			bind_emdata();
		}

		void remove_at(int indx)
		{
			if (mimicObjectsList.empty() || indx >= mimicObjectsList.size() || indx < 0)
				return;
			mimicObjectsList[indx]->clear_em_list();
			mimicObjectsList.erase(mimicObjectsList.begin() + indx);
			objNames.erase(objNames.begin() + indx);
			bind_emdata();
		}

		void remove_all()
		{
			for (auto& obj : mimicObjectsList)
			{
				obj->clear_em_list();
			}
			mimicObjectsList.clear();
			objNames.clear();
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

		std::shared_ptr<MimicListData> get_mimic_list_data(int indx, bool byLoadId = false)
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

		bool is_all_data_swapped(bool recheck = true)
		{
			if (!recheck)
				return isAllDataSwapped;
			for (auto& mimic : mimicObjectsList)
			{
				if (!mimic.get()->mimicList.is_swapped())
				{
					isAllDataSwapped = false;
					return false;
				}
			}
			isAllDataSwapped = true;
			return true;
		}

		bool is_any_swapped(bool recheck = true)
		{
			if(!recheck)
				return isAnyDataSwapped;
			for (const auto& mList : mimicObjectsList)
			{
				if (mList->mimicList.is_swapped())
				{
					isAnyDataSwapped = true;
					return true;
				}
			}
			isAnyDataSwapped = false;
			return false;
		}

		void change_loadid(int indx, int newId)
		{
			std::string newStr = std::string(constLabelName + std::to_string(newId));
			update_idstr(newStr, newId);
			objNames[indx] = newStr;
			mimicObjectsList[indx]->loadId = newId;
		}

		void bind_game_em_data(GameEmList *emData, int mimicIndx)
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

		std::vector<GameEmList> const &get_game_data_list() const { return gameDataList; }

		inline void set_is_any_swapped(bool val) noexcept {isAnyDataSwapped = val; }
	};

	class EnemyWaveEditor : public Mod, private EndLvlHooks::IEndLvl
	{
	public:
		enum Mode
		{
			Mod,
			ViewUserData,
			ReadGameData,
			Serialization
		};
		static Mode mode;

		const int SET_ENEMY_DATA_SIZE = 0x80;

		static bool cheaton;

		static uintptr_t retJmp;
		static uintptr_t retJl;
		static uintptr_t curListAddr;
		static uintptr_t prefabLoadJmp;
		static inline uintptr_t updateBpFadeInRet = 0; //fuck u capcom
		static inline uintptr_t updateBpFadeInJlRet = 0; //fuck u capcom
		static inline uintptr_t bossDanteCrashRet = 0;
		static inline uintptr_t bossDanteCrashSkip = 0;
		
		static inline std::unique_ptr<MimicListManager> mimListManager = nullptr;

		EnemyWaveEditor()
		{
			_txtHelperSize = ImGui::CalcTextSize(_txtHelper).x;
		}

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
		static std::mutex mtx;
		static inline std::mutex prefab_mtx{};

		static inline const std::array<const char*, 40> *_emNames = EnemyData::get_em_names();

		static std::atomic_int emReaderCounter;
		static std::atomic_int emSetterCounter;
		static int emListId;
		static int selectedMimicListItem;
		static int addListInRangeMin;
		static int addListInRangeMax;
		static inline int emChangeState = 0; //0 - add new; 1 - Edit emList

		static bool isBpWarmUpMode;

		inline static const std::array<const char*, 4> modeNames {
			"Add/change enemy data",
			"View all custom data",
			"Get game's enemy data",
			"Import profile"
		};

		enum SetupEmMode
		{
			First,
			Last,
		};

		enum ViewUserDataState
		{
			All,
			ByLoadId
		};

		static inline bool IsEmDataStartedLoading = false;

		static SetupEmMode setupEmMode;
		//static inline bool isDefaultGameData = true;
		SetEmData curCustomEmData;
		int selectedEmDataItem = 0;

		static inline constexpr char* _txtHelper = "You need";
		static inline float _txtHelperSize = 0;

		void reset(EndLvlHooks::EndType end) override
		{
			IsEmDataStartedLoading = false;
			restore_list_data_asm();
			if(cheaton && rndWaveGen->is_rnd_on_restart() && !prflManager->is_custom_gamemode())
				rndWaveGen->generate_random_waves();

		}
		void init_check_box_info() override;
		void print_emdata_input(SetEmData& data);
		void draw_mimic_list_ui();
		void draw_emlist_combo();
		void print_mimiclist_items(int i);
		void print_emlist_data();
		void emlist_btn();
		static void clear_emlist();
		static void edit_gamelists_asm(uintptr_t lstAddr);
		void to_clipboard(const std::string& s);
		void draw_copy_list_data();
		void print_spacing(const char* ch);
		static void read_em_data_asm(uintptr_t lstAddr);
		static void set_em_data_asm(const std::shared_ptr<MimicListData> &curHandleObj, uintptr_t lstAddr);
		void bpmode_data_setup();
		void print_em_colums(int indx);
		std::string add_line_em_info(SetEmData *data);
		static GameEmList get_game_emdata_asm(uintptr_t lstAddr, bool incReaderCounter, bool autoIdFix = true);
		int fromLst = 0;
		int toLst = 0;
		int prevGameDataCount = 0;
		int gameDataIndx = 0;
		ViewUserDataState viewUserDataState = ViewUserDataState::ByLoadId;
		std::unique_ptr<FunctionHook> m_emwave_hook;
		std::unique_ptr<FunctionHook> m_loadall_hook;
		std::unique_ptr<FunctionHook> m_bploadflow_hook;
		std::unique_ptr<FunctionHook> m_bossdante_crash_hook;
		std::unique_ptr<FunctionHook> m_bp_fadein_hook;

		class ProfileManager : EndLvlHooks::IEndLvl
		{
		private:
			std::vector<std::string> profileNames;
			json::JsonValue data;

			std::string _jsonEmDataPath = "";//std::filesystem::current_path().string() + "\\" + "emListsProfile.json";
			std::string _bpEmDataPath = "";
			std::string _gmDataPath = "";
			std::string _curGmPath = "";
			std::string _curGmFileName = "No file";

			int selectedProfile = 0;
			int selectedGmFile = 0;
			int lastMissionN = 0;

			bool isCustomGmSetted = false;
			bool isCreditWaves = false;

			enum ProfileType
			{
				SingleMission,
				FullGameMode
			};
			ProfileType loadType = SingleMission;

			struct Mission
			{
				const int mNum;
				int jsonIndex;
				Mission(int num, int stathamIndx) : mNum(num), jsonIndex(stathamIndx) { }
			};

			std::array<Mission, 24> indexList
			{
				Mission{0, -1},
				Mission{1, -1},
				Mission{2, -1},
				Mission{3, -1},
				Mission{4, -1},
				Mission{5, -1},
				Mission{6, -1},
				Mission{7, -1},
				Mission{8, -1},
				Mission{9, -1},
				Mission{10, -1},
				Mission{11, -1},
				Mission{12, -1},
				Mission{13, -1},
				Mission{14, -1},
				Mission{15, -1},
				Mission{16, -1},
				Mission{17, -1},
				Mission{18, -1},
				Mission{19, -1},
				Mission{20, -1},
				Mission{21, -1},//Nero ending
				Mission(22, -1),//Dante ending
				Mission{23, -1}//Vergil ending
			};

			void reset_indx_list()
			{
				for(auto &m : indexList)
					m.jsonIndex = -1;
			}

			bool update_mission_indxs()
			{
				if(data.GetMemberCount() > indexList.size())
					return false;
				reset_indx_list();
				int mNo = 0;
				for (int i = 0; i < data.GetMemberCount(); i++)
				{
					try
					{
						 mNo = (int)std::stoi(std::string(data[i]["SetupName"].ToString()).substr(1));
						 for (auto& m : indexList)
						 {
							 if (m.mNum == mNo)
							 {
								 m.jsonIndex = i;
								 break;
							 }
						 }
					}
					catch(std::exception &exc) { return false; }
				}
				return true;
			}

			void reset(EndLvlHooks::EndType type) override
			{
				isCreditWaves = false;
			}

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

			void import_data(int setupNameIndx, bool freeAfterLoad = true)
			{
				if (!mimListManager->is_any_swapped(false))
				{
					mimListManager->remove_all();
					if (data[setupNameIndx]["GameEmListHandler"]["GameEmList"].GetMemberCount() != 0)
					{
						int count = data[setupNameIndx]["GameEmListHandler"]["GameEmList"].GetMemberCount();
						mimListManager->clear_game_em_data();
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
							mimListManager->add_game_em_data(curGameList);
						}
					}

					for (int i = 0; i < data[setupNameIndx]["MimicObjectsList"].GetMemberCount(); i++)
					{
						int loadId = data[setupNameIndx]["MimicObjectsList"][i]["LoadId"].ToInteger();
						mimListManager->add(loadId);
						SetEmData *emData = nullptr;
						for (int j = 0; j < data[setupNameIndx]["MimicObjectsList"][i]["MimicList"].GetMemberCount(); j++)
						{
							emData = new SetEmData();
							emData->emId = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["EmId"].ToInteger();
							if (isCreditWaves)
							{
								if(emData->emId == 55)
									emData->emId++;
								if(emData->emId == 38)
									emData->emId = 42;
							}
							emData->selectedItem = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["SelectedItem"].ToInteger();
							emData->num = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["Num"].ToInteger();
							emData->odds = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["Odds"].ToFloat();
							emData->isBoss = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["IsBoss"].ToBoolean();
							emData->isNoDie = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["IsNoDie"].ToBoolean();
							emData->isDontSetOrb = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["IsDontSetOrbs"].ToBoolean();
							emData->isNearPlayer = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["IsNearPlayer"].ToBoolean();
							emData->waitTimeMin = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["WaitTimeMin"].ToFloat();
							emData->waitTimeMax = data[setupNameIndx]["MimicObjectsList"][i]["MimicList"][j]["WaitTimeMax"].ToFloat();
							mimListManager->add_data(emData, i);
						}
					}
					if (freeAfterLoad)
					{
						profileNames.clear();
						data.Reset();
					}
				}
			}

			bool load_custom_mission_data(int missionN)
			{
				if(!isCustomGmSetted)
					return false;
				if (missionN > 20)
					isCreditWaves = true;
				if (isCreditWaves)
				{
					if (lastMissionN >= missionN)
						return false;
					else
					{
						mimListManager->restore_all_data();
						mimListManager->clear_game_em_data();
					}
				}
				lastMissionN = missionN;
				if (missionN == 22)//Crash if swap Dante's credit scene. Idk wtf they decide to unload game data by some extraordinary way here and idk where
				{
					mimListManager->clear_game_em_data();
					mimListManager->remove_all();
					return false;
				}
				for (const auto &m : indexList)
				{
					if (m.mNum == missionN)
					{
						if (m.jsonIndex != -1)
						{
							import_data(m.jsonIndex, false);
							return true;
						}
						else
							return false;
					}
				}
			}

			void import_bp_default()
			{
				read_data(_bpEmDataPath.c_str());
				import_data(0);
			}

			int count() const
			{
				return profileNames.size();
			}

			std::string* const get_profile_name(int indx)
			{
				return &profileNames[indx];
			}

			inline void print_setup()
			{
				ImGui::TextWrapped("Select load data mode:");
				if (ImGui::RadioButton("Single mission/BP", (int*)&loadType, 0))
				{
					reset_indx_list();
					profileNames.clear();
					data.Reset();
					isCustomGmSetted = false;
				}
				ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
				ImGui::RadioButton("Custom game mode", (int*)&loadType, 1);
				ImGui::ShowHelpMarker("A .json with profiles for each mission in the game.");
				switch (loadType)
				{
					case WaveEditorMod::EnemyWaveEditor::ProfileManager::SingleMission:
					{
						ImGui::Separator();
						ImGui::TextWrapped(std::string("Place save file here: " + _jsonEmDataPath).c_str());
						ImGui::ShowHelpMarker("You can change path to save file (and it's name) in trainer's config file. Open \"DMC5_fw_config.txt\" and add line without quotes: \"EnemyWaveEditor._jsonEmDataPath = <YourPathToFileWithFileName.json>\"");
						if (ImGui::Button("Read enemy data settings"))
						{
							read_data(_jsonEmDataPath.c_str());
						}
						if (is_data_read())
						{
							bool isSelected = false;
							if (ImGui::BeginCombo("Seleceted profile", get_profile_name(selectedProfile)->c_str()))
							{
								for (int i = 0; i < count(); i++)
								{
									isSelected = (selectedProfile == i);
									if (ImGui::Selectable(get_profile_name(i)->c_str(), isSelected))
										selectedProfile = i;
									if (isSelected)
										ImGui::SetItemDefaultFocus();
								}
								ImGui::EndCombo();
							}
							ImGui::Spacing();
							if (ImGui::Button("Load profile##EmWaveEditor.import_data()"))
							{
								import_data(selectedProfile);
								mode = Mode::Mod;
							}
							ImGui::ShowHelpMarker("This will remove your current custom data and load data from selected profile. Memory shouldn't be allocated before this stage.");
						}
						ImGui::Separator();
						if (ImGui::Button("Load bp enemy info profile (it also will remove all current enemy lists setup)"))
							import_bp_default();
						break;
					}

					case WaveEditorMod::EnemyWaveEditor::ProfileManager::FullGameMode:
					{
						bool isSelected = false;
						if (ImGui::BeginCombo("##gmList", _curGmFileName.c_str()))
						{
							int i = -1;
							for (const auto& file : std::filesystem::directory_iterator(_gmDataPath))
							{
								i++;
								size_t strL = file.path().filename().string().length();
								if (file.path().filename().string().substr(strL - 5, 5) != ".json")
									continue;
								isSelected = selectedGmFile == i;
								if (ImGui::Selectable(file.path().filename().string().c_str(), isSelected))
								{
									_curGmPath = file.path().string();
									_curGmFileName = file.path().filename().string();
								}
							}
							i = -1;
							ImGui::EndCombo();
						}
						if (ImGui::Button("Select gamemode"))
						{
							if (read_data(_curGmPath.c_str()))
							{
								if(isCustomGmSetted = update_mission_indxs(); isCustomGmSetted)
									EnemyWaveEditor::mode = EnemyWaveEditor::Mode::Mod;
							}
						}
						break;
					}
					default:
						break;
				}
			}

			ProfileType get_cur_load_type() const noexcept { return loadType; }

			inline bool is_custom_gamemode() const noexcept { return isCustomGmSetted; }

			void reset_custom_gm()
			{
				isCustomGmSetted = false;
				data.Reset();
				profileNames.clear();
			}

			void on_config_load(const utility::Config& cfg)
			{
				_jsonEmDataPath = cfg.get("EnemyWaveEditor._jsonEmDataPath").value_or(std::filesystem::current_path().string() + "\\Collab Trainer\\EmListEditor\\" + "EmData.json");
				_bpEmDataPath = std::filesystem::current_path().string() + "\\Collab Trainer\\EmListEditor\\BpDefault\\" + "BP.json";
				_gmDataPath = std::filesystem::current_path().string() + "\\Collab Trainer\\EmListEditor\\GameModes\\";
				auto customGmTmp = cfg.get<bool>("EnemyWaveEditor.prflManager.isCustomGmSetted").value_or(false);
				if (customGmTmp)
				{
					loadType = ProfileManager::FullGameMode;
					_curGmPath = cfg.get("EnemyWaveEditor.prflManager._curGmPath").value_or("");
					_curGmFileName = cfg.get("EnemyWaveEditor.prflManager._curGmFileName").value_or("NoFile");
					try
					{
						if (read_data(_curGmPath.c_str()))
							isCustomGmSetted = update_mission_indxs();
						if(isCustomGmSetted)
							EnemyWaveEditor::cheaton = true;
					}
					catch (std::exception& exc)
					{
						isCustomGmSetted = false;
						loadType = SingleMission;
						_curGmPath = "";
						_curGmFileName = "NoFile";
					}

				}
			}

			void on_config_save(utility::Config& cfg)
			{
				cfg.set<bool>("EnemyWaveEditor.prflManager.isCustomGmSetted", isCustomGmSetted);
				if (isCustomGmSetted)
				{
					cfg.set("EnemyWaveEditor.prflManager._curGmPath", _curGmPath);
					cfg.set("EnemyWaveEditor.prflManager._curGmFileName", _curGmFileName);
				}
			}

			std::string const &get_gm_data_path() const { return _gmDataPath; }

			inline int get_last_misison_num() const noexcept {return lastMissionN; }
		};

		static inline std::unique_ptr<ProfileManager> prflManager = nullptr;

		class WaveRandomGenerator
		{
			using rndInt = std::uniform_int_distribution<int>;
			using rndFloat = std::uniform_real_distribution<float>;

		private:

			struct EmRestriction
			{
			private:
				int curEmId;
				int minNum = 1;
				int maxNum = 2;
				int maxGenInList = 1;
				int emCounter = 0;
				bool isEnabled = false;
				bool isFirstSpecEm = true;
				WaveRandomGenerator *wrg = nullptr;

			public:
				EmRestriction(int emId, WaveRandomGenerator *rg) : curEmId(emId), wrg(rg) { }

				void print_settings();

				//Returns true if out of limit bounds.
				bool check_special_limit(int selectedEnemy)
				{
					if (!isEnabled)
						return false;
					if (selectedEnemy == curEmId)
					{
						if (emCounter >= maxGenInList)
							return true;
						emCounter++;
					}
					return false;
				}

				void special_num_gen(int selectedEnemy, unsigned int &emNum, rndInt& intGen, std::default_random_engine& engine)
				{
					if (!isEnabled)
						return;
					if (selectedEnemy == curEmId)
					{
						intGen = rndInt(minNum, maxNum);
						emNum = intGen(engine);
					}
				}

				void reset_em_count()
				{
					emCounter = 0;
					isFirstSpecEm = true;
				}

				void on_config_load(const utility::Config& cfg);

				void on_config_save(utility::Config& cfg) const;

				bool get_is_enabled() const { return isEnabled; }

				void set_is_enabled(bool value) { isEnabled = value; }

				int em_id() const { return curEmId; }
			};

			EnemyWaveEditor *_pWaveEditor;

			std::vector<int> availableEnemies;//BySelectedIndex

			rndInt dist;
			rndFloat floatDist;
			std::mt19937 mt;
			std::default_random_engine rndEngine{mt};

			unsigned int seed;

			int minGen = 1;
			int maxGen = 4;
			int minNum = 1;
			int maxNum = 2;

			float minOdds = 100.0f;
			float maxOdds = 100.0f;
			float waitTimeMin = 0;
			float waitTimeMax = 0;

			bool isRandomNumGen = true;
			bool isRandomEmNumGen = true;
			bool isRndOdds = false;
			bool isNoOrbs = false;
			bool isRndNoOrbs = true;
			bool isNearPl = false;
			bool isRndNearPl = true;
			bool isAlwaysBossHUDForBosses = true;
			bool isAddEnemiesToBosses = false;
			bool isBossNumAlwaysOne = true;
			bool isAlwaysSingleGenBossInWave = true;
			bool isAlwaysSingleBossInWave = true;
			bool isNearPlIfBoss = false;
			bool isRndOnRestart = false;

			std::array<EmRestriction, 6> heavyEmList{
				EmRestriction{2, this},//Hell Judjecca
				EmRestriction{15, this},//ProtoAngelo
				EmRestriction{12, this},//Behemoth
				EmRestriction{11, this},//Lusachia
				EmRestriction{9, this},//Fury
				EmRestriction{6, this}//Empusa Queen
			};

			bool is_in_list(int em)
			{
				for (int n : availableEnemies)
				{
					if(n == em)
						return true;
				}
				return false;
			}

			void remove_item(int n)
			{
				for (int i = 0; i < availableEnemies.size(); i++)
				{
					if (availableEnemies[i] == n)
					{
						availableEnemies.erase(availableEnemies.begin() + i);
						return;
					}
				}
			}

			int rnd_int(int min, int max)
			{
				dist = rndInt(min, max);
				return dist(rndEngine);
			}

			bool is_boss(int emId)
			{
				if(emId >= 23)
					return true;
				return false;
			}

			bool is_m8()
			{
				if(mimListManager->get_game_data_list().size() >= 107)
					return false;
				int urizenCounter = 0;
				for (const auto& emList : mimListManager->get_game_data_list())
				{
					for (const auto& em : emList.emDataInfo)
					{
						if(em.emId == 35 || em.emId == 36)
							urizenCounter++;
						if(urizenCounter >= 2)
							return true;
					}
				}
				return false;
			}

			inline void no_negative(float& n)
			{
				if (n < 0)
					n = 0;
			}

			inline void limit_left(int& n, int limit)
			{
				if(n < limit)
					n = limit;
			}

			inline void input_int(const char* label, int &n, int limit = 1, int step = 1, int fastStep = 3, ImGuiInputTextFlags flags = 0)
			{
				ImGui::InputInt(label, (int*)&n, step, fastStep, flags);
				limit_left(n, limit);
			}

			inline void input_float(const char* label, float &n, const char* format = "%.1f", float step = 1.0f, float fastStep = 3.0f, ImGuiInputTextFlags flags = 0)
			{
				ImGui::InputFloat(label, &n, step, fastStep, format, flags);
				no_negative(n);
			}

			inline void print_bool_rnd_params(bool &isRnd, const char *label1, bool &param, const char *label2)
			{
				ImGui::Checkbox(label1, &isRnd);
				if (!isRnd)
					ImGui::Checkbox(label2, &param);
			}

			void sort()
			{
				if(availableEnemies.empty())
					return;
				for (int i = 0; i < availableEnemies.size() - 1; i++)
				{
					for (int j = 0; j < availableEnemies.size() - i - 1; j++)
					{
						if(availableEnemies[j] > availableEnemies[j + 1])
							std::swap(availableEnemies[j], availableEnemies[j+1]);
					}
				}
			}

			bool is_heavy_em_contains(int id)
			{
				for (const auto &em : heavyEmList)
				{
					if(id == em.em_id())
						return true;
				}
				return false;
			}

		public:
			WaveRandomGenerator(EnemyWaveEditor *waveEditor) : _pWaveEditor(waveEditor) 
			{
				seed = mt.default_seed;
			}
			
			void show_enemy_selection();

			void print_settings()
			{
				ImGui::Checkbox("Random enemy amount for each enemy list", &isRandomNumGen);
				if (isRandomNumGen)
				{
					ImGui::TextWrapped("Min enemy count:");
					input_int("##minGen", minGen);
					ImGui::TextWrapped("Max enemy count:");
				}
				else
					ImGui::TextWrapped("Enemy count for all lists:");
				input_int("##maxGen", maxGen);

				ImGui::Checkbox("Random enemy num for each enemy in list", &isRandomEmNumGen);
				if (isRandomEmNumGen)
				{
					ImGui::TextWrapped("Min enemy num:");
					input_int("##minNum", minNum);
					ImGui::TextWrapped("Max enemy num:");
				}
				else
					ImGui::TextWrapped("Enemy num for all lists:");
				input_int("##maxNum", maxNum);

				ImGui::Checkbox("Random appear odds for each enemy in list", &isRndOdds);
				if (isRndOdds)
				{
					ImGui::TextWrapped("Min odds:");
					UI::SliderFloat("##minOdds", &minOdds, 0, 100.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
					ImGui::TextWrapped("Max odds:");
				}
				else
					ImGui::TextWrapped("Odds for all enemies:");
				UI::SliderFloat("##maxnOdds", &maxOdds, 0, 100.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
				if(minOdds - maxOdds > 0.01f)
					minOdds = maxOdds - 0.1f;
				
				ImGui::TextWrapped("Wait time min for all enemies:");
				input_float("##waitTimeMin", waitTimeMin);
				ImGui::TextWrapped("Wait time max for all enemies:");
				input_float("##waitTimeMax", waitTimeMax);

				print_bool_rnd_params(isRndNoOrbs, "Random \"don't set orbs\" parameter for all enemies;", isNoOrbs, " \"Don't set orbs\" parameter for all enemies");
				print_bool_rnd_params(isRndNearPl, "Random \"is near player\" parameter for all enemies;", isNearPl, " \"Is near palyer\" parameter for all enemies");
				ImGui::Spacing();
				ImGui::Checkbox("Always add boss's HUD to bosses", &isAlwaysBossHUDForBosses);
				ImGui::Checkbox("Always generate 1 instance of boss enemy", &isBossNumAlwaysOne);
				ImGui::ShowHelpMarker("E.g. boss enemy will always have num = 1.");
				ImGui::Checkbox("Random only 1 boss in custom wave", &isAlwaysSingleGenBossInWave);
				ImGui::ShowHelpMarker("If custom enemy list already have a boss enemy, all next enemies in this list will be regular. If only bosses selected to random, this option will be disabled.");
				ImGui::Checkbox("Add bunch of random enemies to default game's bossfights", &isAddEnemiesToBosses);
				ImGui::ShowHelpMarker("If disabled, waves with bosses will be unchanged.");
				ImGui::Checkbox("Don't random bosses to game's default boss list", &isAlwaysSingleBossInWave);
				ImGui::ShowHelpMarker("E.g. there will be always 1 default boss in the default boss wave, even if some bosses selected to random. If only bosses selected to random, this option will be disabled.");
				ImGui::Checkbox("Always Set \"Is near player\" for bosses to true", &isNearPlIfBoss);
				ImGui::ShowHelpMarker("This can help if boss locks spawn of other enemies in list");
				ImGui::Separator();
				if (ImGui::CollapsingHeader("Special settings for \"powerful\" enemies"))
				{
					for (int i = 0; i < heavyEmList.size() - 1; i++)
					{
						heavyEmList[i].print_settings();
						ImGui::Spacing();
					}
					heavyEmList[heavyEmList.size() - 1].print_settings();
				}
				ImGui::Separator();
				ImGui::Spacing();
				ImGui::TextWrapped("Seed:");
				ImGui::ShowHelpMarker("For playing in coop mode seed must be the same for all players to generate same enemy sequences. Input num that <= 0 to random seed.");
				ImGui::InputInt("##rndWaveEditorSeed", (int*)&seed);
				if(seed <= 0)
					seed = std::random_device{}();
				if(ImGui::Button("Set seed"))
					set_seed(seed);
				ImGui::Checkbox("Auto random on restarts", &isRndOnRestart);
				ImGui::ShowHelpMarker("Always generate new waves with current seed when you reload/restart/exit from missions/secret missions/bp. Doesn't work when custom game mode loaded.");
			}

			void set_seed(unsigned int seed)
			{
				rndEngine.seed(seed);
				this->seed = seed;
			}

			void generate_random_waves()
			{
				if(availableEnemies.empty())
					return;
				mimListManager->restore_all_data();
				mimListManager->remove_all();
				int emCount = maxGen;
				bool isBossWave = false; //is boss wave by default game's data
				bool isBossWaveGenerated = false;
				bool isSpecialEm = false;
				bool isCurEmBoss = false;
				if (is_bosses_only())
				{
					isAlwaysSingleGenBossInWave = false;
					isAlwaysSingleBossInWave = false;
				}
				if (is_spec_em_only())
				{
					for (auto &em : heavyEmList)
						em.set_is_enabled(false);
				}
				bool isM8 = is_m8();
				for (int i = 0, k = 0; i < mimListManager->get_game_data_list().size(); i++, k++)
				{
					isBossWave = is_boss(mimListManager->get_game_data_list()[i].emDataInfo[0].emId);
					if ((isBossWave && !isAddEnemiesToBosses) || (isBossWave && isM8))
					{
						k--;
						continue;
					}
					std::vector<int> tmp = availableEnemies;
					if (isAlwaysSingleBossInWave && isBossWave && availableEnemies.size() > 1)
					{
						auto it = availableEnemies.begin();
						while (it != availableEnemies.end())
						{
							if(*it >= 20)
								availableEnemies.erase(it);
							else it++;
						}
					}
					
					mimListManager->add(i);
					if (isRandomNumGen)
					{
						if(minGen > maxGen)
							maxGen = minGen + 1;
						emCount = rnd_int(minGen, maxGen);
					}
					isBossWaveGenerated = false;
					for (int j = 0; j < emCount; j++)
					{
						isSpecialEm = false;
						mimListManager->add_data(new SetEmData(), k);//Need to do this by load id
						unsigned int enemy = 0;
						do
						{
							do
							{
								enemy = availableEnemies[rnd_int(0, availableEnemies.size() - 1)];
								for (int z = 0; z < heavyEmList.size(); z++)
								{
									if (heavyEmList[z].check_special_limit(enemy))
									{
										enemy = availableEnemies[rnd_int(0, availableEnemies.size() - 1)];
										z = 0;
									}
								}
							} while (isAlwaysSingleGenBossInWave && isBossWaveGenerated && enemy >= 20);

						} while(isAlwaysSingleBossInWave && isBossWave && enemy >= 20);

						mimListManager->get_mimic_list_data(k)->mimicList[j]->selectedItem = enemy;
						mimListManager->get_mimic_list_data(k)->mimicList[j]->emId = EnemyData::indx_to_id(enemy);
						isCurEmBoss = is_boss(mimListManager->get_mimic_list_data(k)->mimicList[j]->emId);
						isBossWaveGenerated = isBossWaveGenerated ? true : isCurEmBoss;
						if (isBossNumAlwaysOne && isCurEmBoss)
							mimListManager->get_mimic_list_data(k)->mimicList[j]->num = 1;
						else
						{
							if(maxNum < minNum)
								maxNum = minNum + 1;
							unsigned int emNum = maxNum;
							if (isRandomEmNumGen)
							{
								emNum = rnd_int(minNum, maxNum);
							}
							for (auto em : heavyEmList)
							{
								em.special_num_gen(enemy, emNum, dist, rndEngine);
							}
							mimListManager->get_mimic_list_data(k)->mimicList[j]->num = emNum;
						}

						float odds = maxOdds;
						if (isRndOdds && j != 0)
						{
							if(maxOdds - minOdds < -0.01f)
								maxOdds = minOdds + 0.1f;
							odds = (float)rnd_int(minOdds, maxOdds);
						}
						mimListManager->get_mimic_list_data(k)->mimicList[j]->odds = odds;
						bool _isNoOrbs = isNoOrbs;
						if(isRndNoOrbs)
							_isNoOrbs = (bool)rnd_int(0, 1);
						mimListManager->get_mimic_list_data(k)->mimicList[j]->isDontSetOrb = _isNoOrbs;
						bool _isNearPl = isNearPl;
						if(isNearPlIfBoss && isCurEmBoss)
							_isNearPl = true;
						else if (isRndNearPl)
							_isNearPl = (bool)rnd_int(0, 1);
						mimListManager->get_mimic_list_data(k)->mimicList[j]->isNearPlayer = _isNearPl;
						if (isAlwaysBossHUDForBosses)
						{
							if(is_boss(mimListManager->get_mimic_list_data(k)->mimicList[j]->emId))
								mimListManager->get_mimic_list_data(k)->mimicList[j]->isBoss = true;
						}
						else
							mimListManager->get_mimic_list_data(k)->mimicList[j]->isBoss = false;
						float wtMin = waitTimeMin, wtMax = waitTimeMax;
						if (j == 0)
							wtMin = wtMax = 0;

						mimListManager->get_mimic_list_data(k)->mimicList[j]->waitTimeMin = wtMin;
						mimListManager->get_mimic_list_data(k)->mimicList[j]->waitTimeMax = wtMax;

						if (isBossWave && isAddEnemiesToBosses && j == emCount - 1) // add Boss last to the list
						{
							int bossesCount = mimListManager->get_game_data_list()[i].emDataInfo.size();
							bool isM7Ldk = bossesCount == 2 && mimListManager->get_game_data_list()[i].emDataInfo[1].emId <= 20;
							for (int z = 0; z < bossesCount; z++)
							{
								mimListManager->add_data(new SetEmData(), i);
								j++;
								mimListManager->get_mimic_list_data(k)->mimicList[j]->isBoss = true;
								mimListManager->get_mimic_list_data(k)->mimicList[j]->emId = mimListManager->get_game_data_list()[i].emDataInfo[z].emId;
								mimListManager->get_mimic_list_data(k)->mimicList[j]->num = mimListManager->get_game_data_list()[i].emDataInfo[z].num;
								mimListManager->get_mimic_list_data(k)->mimicList[j]->selectedItem = EnemyData::id_to_indx(mimListManager->get_game_data_list()[i].emDataInfo[z].emId);
							}
							if(bossesCount == 1 && mimListManager->get_game_data_list()[i].emDataInfo[0].emId == 25)//Nidhogg is funny;
								std::swap(mimListManager->get_mimic_list_data(k)->mimicList[0], mimListManager->get_mimic_list_data(k)->mimicList[mimListManager->get_mimic_list_data(k)->mimicList.get_mimic_count() - 1]);
							else if (bossesCount == 2 && !isM7Ldk) // cat and parrot encounter before nightmire ty cumpcom for that funny trigger
							{
								if (mimListManager->get_mimic_list_data(k)->mimicList.get_mimic_count() > 2)
								{
									std::swap(mimListManager->get_mimic_list_data(k)->mimicList[mimListManager->get_mimic_list_data(k)->mimicList.get_mimic_count() - 2],//cat first
										mimListManager->get_mimic_list_data(k)->mimicList[0]);
									std::swap(mimListManager->get_mimic_list_data(k)->mimicList[mimListManager->get_mimic_list_data(k)->mimicList.get_mimic_count() - 1],
										mimListManager->get_mimic_list_data(k)->mimicList[1]);
								}
							}
							isBossWave = false;
						}
					}
					for(auto &em : heavyEmList)
						em.reset_em_count();
					availableEnemies = tmp;
				}
			}

			bool is_bosses_only()
			{
				for (int i : availableEnemies)
				{
					if(i < 20)
						return false;
				}
				return true;
			}

			bool is_spec_em_only()
			{
				for (int em : availableEnemies)
				{
					if(!is_heavy_em_contains(em))
						return false;
				}
				return true;
			}

			void on_config_load(const utility::Config &cfg);

			void on_config_save(utility::Config& cfg) const;

			inline bool is_rnd_on_restart() const noexcept { return isRndOnRestart; }
		};

		std::unique_ptr<WaveRandomGenerator> rndWaveGen = nullptr;
	};
}