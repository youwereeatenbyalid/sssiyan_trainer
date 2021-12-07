#pragma once
#include "Mod.hpp"
#include <random>
#include "CheckpointPos.hpp"
#include "mods/GameFunctions/CreateShell.hpp"
#include "EnemySwapper.hpp"
#include <atomic>
#include "VergilSDTFormTracker.hpp"
#include "PlayerTracker.hpp"
#include "EnemyWaveEditor.hpp"

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
		func::Vec3 stratPosOffs;
		func::Vec3 moveTrackStepOffs;

		std::default_random_engine rndGen;

		std::shared_ptr<func::CreateShell> jcSpawn;

		const float zTrackOffs = 0.94f;
		const float rndEmMaxDist = 34.5f;

		const std::array<uintptr_t, 4> isPauseOffst { 0x100, 0x288, 0xC8, 0x5C4 };
		const std::array<uintptr_t, 7> jcPrefabOffsets {0x80, 0x88, 0x70, 0xC8, 0x300, 0x38, 0x10};//{0x140, 0x90, 0x10, 0xC8, 0x300, 0x38, 0x10};//{ 0x70, 0x170, 0x10, 0x88, 0x300, 0x38, 0x10 }; //{0x80, 0x90, 0x10, 0x108, 0x300, 0x38, 0x10};

		uintptr_t isPauseBase;
		uintptr_t jcPrefabBase;

		std::atomic_bool executing;

		bool isPtrBaseInit;
		const bool isRndJust = true;
		const bool isTrackJust = false;

		func::Vec3 get_lockon_pos(bool& isNullPtr)
		{
			uintptr_t lockOnObj = *(uintptr_t*)(PlayerTracker::vergilentity + 0x428);
			if (lockOnObj == 0)
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
			if (EnemySwapper::nowFlow == 0x16 && executing && !isBadPtr)
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
				executing.store(true);
				//*(int*)rayCastAddr = curRayCastSize;
			}
			else
			{
				isJceRunning = false;
				executing.store(false);
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
		const int defaultRndDelay = 100;
		const int defaultTrackDelay = 187;

		int rndDelayTime = defaultRndDelay;//ms
		int trackDelayTime = defaultTrackDelay;//ms

		float executionTime;
		const float rndExeTimeModDefault = 6.8f;
		const float trackExeTimeModDefault = 5.65f;
		const float rndAttackRate = 2.5f;

		JCEController()
		{
			maxOffset.x = 10.3f;
			maxOffset.y = 10.2;
			maxOffset.z = 4.21f;
			minOffset.x = -10.3f;
			minOffset.y = -10.2f;
			minOffset.z = 0.96f;

			moveTrackStepOffs.x = 1.18f;
			moveTrackStepOffs.y = 1.18F;
			moveTrackStepOffs.z = 0.98;

			executing = false;
			jcPrefabBase = 0x07E61B90;//0x07E60490;//0x07E53650;
			isPauseBase = 0x07E55910;
			executionTime = rndExeTimeModDefault;
			isPtrBaseInit = false;
		}

		void start_jce()
		{
			if(executing.load())
				return;
			if(!isPtrBaseInit)
				return;
			update_status(true);
			switch (jceType)
			{
				case JCEController::Random:
				{
					std::thread([&]
					{
						func::Vec3 pPos = CheckpointPos::get_player_coords();
						bool isBadPtr = false;
						bool isPause = false;
						uintptr_t jcPrefab = func::PtrController::get_ptr<uintptr_t>(jcPrefabBase, jcPrefabOffsets, isBadPtr);
						if (isBadPtr || jcPrefab == 0)
						{
							bad_ptr_stop();
							return;
						}
						auto rcx = jcSpawn->get_rcx_arg_ptr();
						if(!rcx.has_value())
						{
							bad_ptr_stop();
							return;
						}
						func::Quaternion defaultRot;
						func::Vec3 curPos;
						func::Vec3 prevPos;
						isBadPtr = false;
						uintptr_t jcShell;

						std::uniform_real_distribution<float> xDist(minOffset.x, maxOffset.x);
						std::uniform_real_distribution<float> yDist(minOffset.y, maxOffset.y);
						std::uniform_real_distribution<float> zDist(minOffset.z, maxOffset.z);

						bool nullLockOn = false;
						func::Vec3 lockOnPos = get_lockon_pos(nullLockOn);
						if(nullLockOn)
							lockOnPos = pPos;
						rndjc_pos_update(curPos, lockOnPos, xDist, yDist, zDist);

						jcSpawn->set_params(rcx.value(), jcPrefab, curPos, defaultRot, PlayerTracker::vergilentity, 0, 0);

						while (get_condition(isBadPtr))
						{
							isPause = func::PtrController::get_ptr<bool>(isPauseBase, isPauseOffst, isBadPtr);
							if (isPause)
								continue;
							if (EnemySwapper::nowFlow != 0x16 || executing == false)//check this again
								break;
							jcShell = jcSpawn->invoke(curPos, defaultRot);
							if (jcShell == 0)
								break;
							*(bool*)(jcShell + 0x440) = isRndJust; //isJust
							*(float*)(jcShell + 0x444) = rndAttackRate; //attackRate
							jcShell = 0;

							std::this_thread::sleep_for(std::chrono::milliseconds(rndDelayTime));

							prevPos = curPos;

							lockOnPos = get_lockon_pos(nullLockOn);
							if (nullLockOn || func::Vec3::vec_length(pPos, lockOnPos) > rndEmMaxDist)
								lockOnPos = pPos;

							do 
							{
								rndjc_pos_update(curPos, lockOnPos, xDist, yDist, zDist);
							} while(func::Vec3::vec_length(curPos, prevPos) < 5.1f && executing);

						}
						update_status(false);
					}).detach();

					break;
				}
				case JCEController::Track:
				{
					std::thread([&]
					{
						auto pPos = CheckpointPos::get_player_coords();
						func::Vec3 prevPos;
						func::Vec3 curPos = CheckpointPos::get_player_coords();
						curPos.z += zTrackOffs;
						func::Vec3 emPos;
						bool isBadPtr = false;
						bool isPause = false;
						uintptr_t jcPrefab = func::PtrController::get_ptr<uintptr_t>(jcPrefabBase, jcPrefabOffsets, isBadPtr);
						if (isBadPtr || jcPrefab == 0)
						{
							bad_ptr_stop();
							return;
						}
						auto rcx = jcSpawn->get_rcx_arg_ptr();
						if (!rcx.has_value())
						{
							bad_ptr_stop();
							return;
						}
						func::Quaternion defaultRot;
						jcSpawn->set_params(rcx.value(), jcPrefab, curPos, defaultRot, PlayerTracker::vergilentity, 0, 0);
						isBadPtr = false;
						uintptr_t jcShell;
						float distance = 0.0f;
						while (get_condition(isBadPtr))
						{
							isPause = func::PtrController::get_ptr<bool>(isPauseBase, isPauseOffst, isBadPtr);
							if (isPause)
								continue;
							if (EnemySwapper::nowFlow != 0x16)//check this again
								break;
							jcShell = jcSpawn->invoke(curPos, defaultRot);
							if (jcShell == 0)
								break;
							*(bool*)(jcShell + 0x440) = isTrackJust; //isJust
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

		void stop_jce()
		{
			update_status(false);
		}

		bool is_executing() const
		{
			return executing.load();
		}

		void init_ptrs_base(uintptr_t dmc5base)
		{
			jcPrefabBase += dmc5base;
			isPauseBase += dmc5base;
			isPtrBaseInit = true;
			jcSpawn = std::make_shared<func::CreateShell>();
		}

		bool is_ptr_init() const {return isPtrBaseInit; }

		void set_rndspawn_delay(int delay) {rndDelayTime = delay; }

		void set_trackspawn_delay(int delay) {trackDelayTime = delay; }

		void set_jce_type(Type type) 
		{
			jceType = type;
			switch (type)
			{
				case DMC3JCE::JCEController::Random:
				{
					executionTime = rndExeTimeModDefault;
					break;
				}
					
				case DMC3JCE::JCEController::Track:
				{
					executionTime = trackExeTimeModDefault;
					break;
				}
				default:
					break;
			}
		}

		Type get_jce_type() const {return jceType; }

	};

    static inline JCEController jceController{};
	static inline bool cheaton = true;
	static inline bool isJceRunning = false;
	static inline bool isCrashFixEnabled = true;
	static inline bool isUseDefaultJce = false;

	static inline uintptr_t canExeJceRet = 0;
	static inline uintptr_t canExeJceRet1 = 0;
	static inline uintptr_t subSdtRet = 0;
	static inline uintptr_t jcePfbRet = 0;
	static inline uintptr_t jcePfb1Ret = 0;
	static inline uintptr_t jcePfbJeJmp = 0;
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

	static inline float *pJceExeTime = nullptr;

	static inline int rndDelay = 0;
	static inline int trackDelay = 0;
	static inline int prevRayCastSize = 64;
	static inline int curRayCastSize = 64;

	static inline JCEController::Type jcTypeUi{};

	static void start_jce_asm();
	static void stop_jce_asm();

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
	void on_frame() override;
	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override;
	// on_draw_debug_ui() is called when debug window shows up
	void on_draw_debug_ui() override;

private:
	//std::unique_ptr<JCEController> jceController;
	void init_check_box_info() override;
	std::unique_ptr<FunctionHook> m_can_exe_jce_hook;
	std::unique_ptr<FunctionHook> m_can_exe_jce1_hook;
	std::unique_ptr<FunctionHook> m_sub_human_jce_hook;
	std::unique_ptr<FunctionHook> m_jce_prefab_hook;
	std::unique_ptr<FunctionHook> m_jce_timer_hook;
	std::unique_ptr<FunctionHook> m_jce_crashpoint_hook;
	std::unique_ptr<FunctionHook> m_jce_finishpfb_hook;
	std::unique_ptr<FunctionHook> m_jce_prefab1_hook;

};
//clang-format on

