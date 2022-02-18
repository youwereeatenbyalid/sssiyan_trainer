#pragma once
#include "Mod.hpp"
#include <random>
#include "CheckpointPos.hpp"
#include "mods/GameFunctions/CreateShell.hpp"
#include "EnemySwapper.hpp"
#include <atomic>
#include <condition_variable>
#include "VergilSDTFormTracker.hpp"
#include "PlayerTracker.hpp"
#include "EnemyWaveEditor.hpp"
#include "DeepTurbo.hpp"

//clang-format off
namespace func = GameFunctions;


class DMC3JCE : public Mod
{
public:
	class JCEController
	{
	private:
		func::Vec3 maxOffset;
		func::Vec3 minOffset;
		func::Vec3 moveTrackStepOffs;

		std::mt19937 rndGen;

		func::CreateShell jcSpawn;

		int rndDelayTime = defaultRndDelay;//ms
		int trackDelayTime = defaultTrackDelay;//ms

		const float zTrackOffs = 0.94f;
		const float rndEmMaxDist = 34.5f;
		const float rndAttackRate = 2.5f;
		const float trackAttackRate = 1.2f;

		const std::array<uintptr_t, 4> isPauseOffst { 0x100, 0x288, 0xC8, 0x5C4 };
		//const std::array<uintptr_t, 7> jcPrefabOffsets {0x80, 0x88, 0x70, 0xC8, 0x300, 0x38, 0x10};//{0x140, 0x90, 0x10, 0xC8, 0x300, 0x38, 0x10};//{ 0x70, 0x170, 0x10, 0x88, 0x300, 0x38, 0x10 }; //{0x80, 0x90, 0x10, 0x108, 0x300, 0x38, 0x10};
		const std::array<uintptr_t, 2> jcPfbYamatoParam {0x38, 0x10}; //Via PlTraker::yamatocommonparam

		uintptr_t isPauseBase;
		//uintptr_t jcPrefabBase;

		std::atomic_bool isExecuting;

		bool isPtrBaseInit;
		const bool isRndJust = true;
		const bool isTrackJust = false;

		func::Vec3 get_lockon_pos(bool& isNullPtr)
		{
			uintptr_t lockOnObj = *(uintptr_t*)(PlayerTracker::vergilentity + 0x428);
			if (IsBadReadPtr((void*)lockOnObj, 0x8))
			{
				isNullPtr = true;
				return func::Vec3(0, 0, 0);
			}
			func::Vec3 lockOnPos;
			lockOnObj = *(uintptr_t*)(lockOnObj + 0x10);//LockOnTargetWork target
			auto transform = *(uintptr_t*)(lockOnObj + 0x50);//cachedTransform
			//auto lockOnOffset = *(uintptr_t*)(lockOnObj + 0x20);
			lockOnPos.x = *(float*)(transform + 0x30) + (*(float*)(lockOnObj + 0x20));
			lockOnPos.z = *(float*)(transform + 0x34) + (*(float*)(lockOnObj + 0x24));
			lockOnPos.y = *(float*)(transform + 0x38) + (*(float*)(lockOnObj + 0x28));
			isNullPtr = false;
			return lockOnPos;
		}

		void track_pos_update(func::Vec3 &currentJcPos)
		{
			func::Vec3 lockOnPos;
			bool isNoLockOn = false;
			lockOnPos = get_lockon_pos(isNoLockOn);
			if(isNoLockOn)
				return;
			lockOnPos.z += zTrackOffs;
			float length = func::Vec3::vec_length(currentJcPos, lockOnPos);
			float acc = 0.0f;
			if(length >= 11.4f)
				acc = 2.3f;
			if(abs(lockOnPos.x - currentJcPos.x) >= 2.3f)
				currentJcPos.x = lockOnPos.x >= currentJcPos.x ? currentJcPos.x + moveTrackStepOffs.x + acc : currentJcPos.x - moveTrackStepOffs.x - acc;
			else
				currentJcPos.x = lockOnPos.x;
			if (abs(lockOnPos.y - currentJcPos.y) >= 2.3f)
				currentJcPos.y = lockOnPos.y >= currentJcPos.y ? currentJcPos.y + moveTrackStepOffs.y + acc : currentJcPos.y - moveTrackStepOffs.y - acc;
			else
				currentJcPos.y = lockOnPos.y;
			if (abs(lockOnPos.z - currentJcPos.z) >= 2.1f)
				currentJcPos.z = lockOnPos.z >= currentJcPos.z ? currentJcPos.z + moveTrackStepOffs.z : currentJcPos.z - moveTrackStepOffs.z;
			else
				currentJcPos.z = lockOnPos.z;
		}

		void rndjc_pos_update(func::Vec3 &pos, const func::Vec3 &pPos, std::uniform_real_distribution<float> &xDist, std::uniform_real_distribution<float> &yDist, std::uniform_real_distribution<float> &zDist)
		{
			pos.x = pPos.x + xDist(rndGen);
			pos.y = pPos.y + yDist(rndGen);
			pos.z = pPos.z + zDist(rndGen);
		}

		void rndjc_pos_update(func::Vec3& pos, const func::Vec3 &pPos, std::uniform_int_distribution<int> &xDist, std::uniform_int_distribution<int>& yDist, std::uniform_real_distribution<float>& zDist)
		{
			pos.x = pPos.x + xDist(rndGen);
			pos.y = pPos.y + yDist(rndGen);
			pos.z = pPos.z + zDist(rndGen);
		}

		bool get_condition(bool isBadPtr)
		{
			if (EnemySwapper::nowFlow == 0x16 && isExecuting.load() && !isBadPtr && !DeepTurbo::isCutscene && PlayerTracker::vergilentity != 0)
			{
				if (EnemySwapper::gameMode == 3)
				{
					if(WaveEditorMod::EnemyWaveEditor::bpFlowId != 0x16)
						return false;
				}
				return true;
			}
			else return false;
		}

		void update_status(bool run)
		{
			if (run)
			{
				isJceRunning = true;
				isExecuting.store(true);
				//*(int*)rayCastAddr = curRayCastSize;
			}
			else
			{
				isExecuting.store(false);
				isJceRunning = false;
				//*(int*)rayCastAddr = prevRayCastSize;
			}
		}

		void bad_ptr_stop()
		{
			stop_jce();
			cheaton = false;
		}

	public:
		enum Type
		{
			Random,
			Track
		};

	private:
		Type jceType;

	public:
		const int defaultRndDelay = 120;
		const int defaultTrackDelay = 187;

		static inline float executionTimeAsm;
		int rndEmTrackInterval = 22;

		const float rndExeTimeModDefault = 6.8f;
		const float trackExeTimeModDefault = 5.95f;

		JCEController()
		{
			maxOffset.x = 10.7f;
			maxOffset.y = 10.6;
			maxOffset.z = 4.21f;
			minOffset.x = -10.7f;
			minOffset.y = -10.6f;
			minOffset.z = 0.96f;

			moveTrackStepOffs.x = 1.18f;
			moveTrackStepOffs.y = 1.18F;
			moveTrackStepOffs.z = 0.98;

			isExecuting = false;
			//jcPrefabBase = 0x07E61B90;//0x07E60490;//0x07E53650;
			isPauseBase = 0x07E55910;
			JCEController::executionTimeAsm = rndExeTimeModDefault;
			auto base = g_framework->get_module().as<uintptr_t>();
			//jcPrefabBase += base;
			isPauseBase += base;
			isPtrBaseInit = true;
		}

		bool set_capacity(int newCap){ return jcSpawn.set_list_shell_capacity(newCap); }

		volatile void start_jce()
		{
			if(isExecuting.load())
				return;
			if(!isPtrBaseInit)
				return;
			update_status(true);
			int id = 105;
			int lvl = 1;
			bool isBadPtr = false;
			std::optional<bool> isPause;
			auto jcPrefab = func::PtrController::get_ptr_val<uintptr_t>(PlayerTracker::yamatocommonparameter, jcPfbYamatoParam, true); 
			switch (jceType)
			{
				case JCEController::Random:
				{
					std::thread([this, id, lvl, isBadPtr, isPause, jcPrefab]() mutable
					{
						func::Vec3 pPos = CheckpointPos::get_player_coords();
						if (!jcPrefab.has_value() || jcPrefab.value() == 0)
						{
							bad_ptr_stop();
							return;
						}
						if(jcSpawn.get_thread_context() == 0)
						{
							bad_ptr_stop();
							return;
						}
						func::Quaternion defaultRot;
						func::Vec3 curPos;
						func::Vec3 prevPos;
						isBadPtr = false;
						volatile void* jcShell = 0;

						std::uniform_real_distribution<float> xDist(minOffset.x, maxOffset.x);
						std::uniform_real_distribution<float> yDist(minOffset.y, maxOffset.y);
						std::uniform_real_distribution<float> zDist(minOffset.z, maxOffset.z);

						bool nullLockOn = false;
						func::Vec3 lockOnPos = get_lockon_pos(nullLockOn);
						if(nullLockOn)
							lockOnPos = pPos;
						rndjc_pos_update(curPos, lockOnPos, xDist, yDist, zDist);
						int jcNum = 0;
						int tryIter = 0;
						jcSpawn.set_params(jcPrefab.value_or(0), curPos, defaultRot, PlayerTracker::vergilentity, lvl, id);
						/*if (isSetCustomCapacity)
						{
							if (jcSpawn.get_list_shell_capacity() < newCapacity)
							{
								__try
								{
									jcSpawn.set_list_shell_capacity(newCapacity);
								}
								__except (EXCEPTION_EXECUTE_HANDLER)
								{
									isBadPtr = true;
									bad_ptr_stop();
									return;
								}
							}
						}*/
						while (get_condition(isBadPtr))
						{
							isPause = func::PtrController::get_ptr_val<bool>(isPauseBase, isPauseOffst);
							if (!isPause.has_value())
							{
								isBadPtr = true;
								break;
							}
							if (isPause.value())
								continue;
							if (EnemySwapper::nowFlow != 0x16 || isExecuting.load() == false)//check this again
								break;
							__try
							{
								jcShell = (volatile void*)jcSpawn(curPos, defaultRot);
							}
							__except (EXCEPTION_EXECUTE_HANDLER)
							{
								isBadPtr = true;
								bad_ptr_stop();
								break;
							}
							if (jcShell == 0)
								continue;
							if(!IsBadReadPtr((void*)((uintptr_t)jcShell+0x440), 8))
								*(bool*)((uintptr_t)jcShell + 0x440) = isRndJust; //isJust
							if(!IsBadReadPtr((void*)((uintptr_t)jcShell + 0x444), 8))
								*(float*)((uintptr_t)jcShell + 0x444) = rndAttackRate; //attackRate
							jcNum++;
							std::this_thread::sleep_for(std::chrono::milliseconds(rndDelayTime));//Try to use winH Sleep()
							prevPos = curPos;

							lockOnPos = get_lockon_pos(nullLockOn);
							if (nullLockOn || func::Vec3::vec_length(pPos, lockOnPos) > rndEmMaxDist)
								lockOnPos = pPos;
							if (jcNum >= rndEmTrackInterval)
							{
								jcNum = 0;
								lockOnPos.z += zTrackOffs;
								curPos = lockOnPos;
							}
							else
							{
								tryIter = 0;
								do
								{
									tryIter++;
									rndjc_pos_update(curPos, lockOnPos, xDist, yDist, zDist);
								} while (func::Vec3::vec_length(curPos, prevPos) < 4.65f && tryIter <= 60);
							}
						}
						update_status(false);
					}).detach();

					break;
				}
				case JCEController::Track:
				{
					std::thread([&, id, lvl, isBadPtr, isPause, jcPrefab]() mutable
					{
						auto pPos = CheckpointPos::get_player_coords();
						func::Vec3 prevPos;
						func::Vec3 curPos = CheckpointPos::get_player_coords();
						curPos.z += zTrackOffs;
						func::Vec3 emPos;
						if (!jcPrefab.has_value())
						{
							isBadPtr = true;
							bad_ptr_stop();
							return;
						}
						if (jcSpawn.get_thread_context() == 0)
						{
							isBadPtr = true;
							bad_ptr_stop();
							return;
						}
						func::Quaternion defaultRot;
						jcSpawn.set_params(jcPrefab.value(), curPos, defaultRot, PlayerTracker::vergilentity, lvl, id);
						isBadPtr = false;
						uintptr_t jcShell;
						while (get_condition(isBadPtr))
						{
							isPause = func::PtrController::get_ptr_val<bool>(isPauseBase, isPauseOffst);
							if (!isPause.has_value())
							{
								isBadPtr = true;
								break;
							}
							if (isPause.value())
								continue;
							if (EnemySwapper::nowFlow != 0x16)//check this again
								break;
							__try
							{
								jcShell = jcSpawn(curPos, defaultRot);
							}
							__except (EXCEPTION_EXECUTE_HANDLER)
							{
								isBadPtr = true;
								bad_ptr_stop();
								break;
							}
							if (jcShell == 0)
								break;
							*(bool*)(jcShell + 0x440) = isTrackJust; //isJust
							*(float*)(jcShell + 0x444) = trackAttackRate; //attackRate
							jcShell = 0;
							std::this_thread::sleep_for(std::chrono::milliseconds(trackDelayTime));
							track_pos_update(curPos);
						}
						update_status(false);
					}).detach();
					
					break;
				}

				default:
					break;
			}
		}

		void __cdecl stop_jce()
		{
			update_status(false);
		}

		bool is_executing() const
		{
			return isExecuting.load();
		}

		void init_ptrs_base(uintptr_t dmc5base)
		{
			//jcPrefabBase += dmc5base;
			isPauseBase += dmc5base;
			isPtrBaseInit = true;
		}

		bool is_ptr_init() const {return isPtrBaseInit; }

		void set_rndspawn_delay(int delay) {rndDelayTime = delay; }

		void set_trackspawn_delay(int delay) {trackDelayTime = delay; }

		int get_rndspawn_delay() const noexcept {return rndDelayTime; }

		int get_trackspawn_delay() const noexcept {return trackDelayTime; }

		void set_jce_type(Type type, bool isDefaultTime = true, float time = 0) 
		{
			jceType = type;
			if (isDefaultTime)
			{
				switch (type)
				{
					case DMC3JCE::JCEController::Random:
					{
						JCEController::executionTimeAsm = rndExeTimeModDefault;
						break;
					}

					case DMC3JCE::JCEController::Track:
					{
						JCEController::executionTimeAsm = trackExeTimeModDefault;
						break;
					}
					default:
						break;
				}
			}
			else
				JCEController::executionTimeAsm = time;
		}

		Type get_jce_type() const {return jceType; }

	};

    static inline std::unique_ptr<JCEController> jceController{nullptr};
	static inline bool cheaton = true;
	static inline bool isJceRunning = false;
	static inline bool isCrashFixEnabled = true;
	static inline bool isUsingDefaultJce = false;
	static inline bool isSetCustomCapacity = false;

	static inline uintptr_t canExeJceRet = 0;
	static inline uintptr_t canExeJceRet1 = 0;
	static inline uintptr_t subSdtRet = 0;
	static inline uintptr_t jcePfbRet = 0;
	static inline uintptr_t jcePfb1Ret = 0;
	static inline uintptr_t jcePfbJeJmp = 0;
	static inline uintptr_t jcePfbJneJmp = 0;
	static inline uintptr_t jcePfb2Ret = 0;

	static inline uintptr_t jceTimerRet = 0;
	static inline uintptr_t jceTimerStaticBase = 0;
	static inline uintptr_t stopJceTimerRet = 0;
	static inline uintptr_t jceTimerContinue = 0;
	static inline uintptr_t crashPointRet = 0;
	static inline uintptr_t crashPointJeJmp = 0;
	static inline uintptr_t jceFinishPfbRet = 0;
	static inline uintptr_t rayCastAddr = 0;

	static inline float humanJCECost = 3000.0f;
	static inline float minSdt = 3000.0f;

	static inline int rndDelay = 0;
	static inline int trackDelay = 0;
	static inline int prevRayCastSize = 64;
	static inline int curRayCastSize = 64;
	static inline int newCapacity = 256;

	static inline JCEController::Type jcTypeUi{};

	static void __cdecl start_jce_asm();
	static void __cdecl stop_jce_asm();

	//PlVergil +1978 - curWeapon;

	DMC3JCE() = default;

	std::string_view get_name() const override
	{
		return "DMC3 JCE";
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
	// void on_frame() override;
	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override;
	// on_draw_debug_ui() is called when debug window shows up
	// void on_draw_debug_ui() override;

private:
	//std::unique_ptr<JCEController> jceController;
	float rndExeDuration = 0;
	float trackExeDuration = 0;
	void init_check_box_info() override;
	std::unique_ptr<FunctionHook> m_can_exe_jce_hook;
	std::unique_ptr<FunctionHook> m_can_exe_jce1_hook;
	std::unique_ptr<FunctionHook> m_sub_human_jce_hook;
	std::unique_ptr<FunctionHook> m_jce_timer_hook;
	std::unique_ptr<FunctionHook> m_jce_crashpoint_hook;
	std::unique_ptr<FunctionHook> m_jce_finishpfb_hook;
	std::unique_ptr<FunctionHook> m_jce_prefab2_hook;
};
//clang-format on

