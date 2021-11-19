#pragma once
#include "Mod.hpp"
#include <random>
#include "CheckpointPos.hpp"
#include "mods/GameFunctions/PrefabInstantiate.hpp"
#include "EnemySwapper.hpp"
#include <atomic>
#include "VergilSDTFormTracker.hpp"

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
		float moveTrackOffs;
		std::unique_ptr<func::PrefabInstantiateScale> jcSpawn;
		func::Vec3 scale;
		const int JC_RND_COUNT = 900;
		const std::array<uintptr_t, 4> isPauseOffst { 0x100, 0x288, 0xC8, 0x5C4 };
		const std::array<uintptr_t, 7> jcPrefabOffsets { 0x70, 0x170, 0x10, 0x88, 0x300, 0x38, 0x10 }; //{0x80, 0x90, 0x10, 0x108, 0x300, 0x38, 0x10};
		uintptr_t isPauseBase;
		uintptr_t jcPrefabBase;
		//std::mutex mt;
		std::atomic_bool executing;
		bool isPtrBaseInit;

		std::vector<func::Vec3>* generate_pos(const Vector3f& center, int count)
		{
			std::vector<func::Vec3>* res = new std::vector<func::Vec3>();
			func::Vec3 tmp;
			std::default_random_engine xGen;
			std::default_random_engine yGen;
			std::default_random_engine zGen;
			std::default_random_engine generator;

			std::uniform_real_distribution<float> xDist(minOffset.x, maxOffset.x);
			std::uniform_real_distribution<float> yDist(minOffset.y, maxOffset.y);
			std::uniform_real_distribution<float> zDist(minOffset.z, maxOffset.z);

			for (int i = 0; i < count; i++)
			{
				/*tmp.x = center.x + xDist(xGen);
				tmp.y = center.y + yDist(yGen);
				tmp.z = center.z + zDist(zGen);*/
				tmp.x = center.x + xDist(generator);
				tmp.y = center.y + yDist(generator);
				tmp.z = center.z + zDist(generator);
				res->push_back(tmp);
			}
			return res;
		}

	public:
		//bool executing;
		enum Type
		{
			Random,
			Track
		};
		Type jceType;
		int rndTime = 69;//ms
		float executionTime;

		JCEController()
		{
			maxOffset.x = 14.3f;
			maxOffset.y = 14.3;
			maxOffset.z = 6.21f;
			minOffset.x = -14.3f;
			minOffset.y = -14.2f;
			minOffset.z = -0.73f;
			moveTrackOffs = 1.2f;
			executing = false;
			//auto base = g_framework->get_module().as<uintptr_t>();//crash if class instance is static
			//isPauseBase = base + 0x07E55910;
			//jcPrefabBase = base + 0x07E53650;//0x07E61B90;
			jcPrefabBase = 0x07E53650;
			isPauseBase = 0x07E55910;
			scale.x = scale.y = scale.z = 2.1f;
			//jcSpawn = std::make_unique<func::PrefabInstantiateScale>();//also crash
			executionTime = 5.0f;
			isPtrBaseInit = false;
		}

		void start_jce()
		{
			if(!isPtrBaseInit)
				return;
			switch (jceType)
			{
				case JCEController::Random:
				{
					if (executing)
						return;
					executing.store(true);
					std::thread([&]
					{
						//std::unique_lock<std::mutex> lock(mt);
						auto pPos = CheckpointPos::get_player_coords();
						auto jcPosList = generate_pos(pPos, JC_RND_COUNT);
						bool isBadPtr = false;
						bool isPause = false;
						uintptr_t jcPrefab = func::PtrController::get_ptr<uintptr_t>(jcPrefabBase, jcPrefabOffsets, isBadPtr);
						if (isBadPtr || jcPrefab == 0)
							return;
						jcSpawn->set_prefab(jcPrefab);
						//int rndTime = DeepTurbo::cheaton ? defaultJcRndTime / DeepTurbo::turbospeed : defaultJcRndTime;
						isBadPtr = false;
						int i = jcPosList->size();
						func::Quaternion defaultRot;
						do
						{
							isPause = func::PtrController::get_ptr<bool>(isPauseBase, isPauseOffst, isBadPtr);
							if (isPause)
								continue;
							std::this_thread::sleep_for(std::chrono::milliseconds(rndTime));
							if (i == jcPosList->size())
								i = 0;
							jcSpawn->invoke((*jcPosList)[i], defaultRot, scale);

							i++;
						} while (executing == true /* && EnemySwapper::nowFlow == 0x16*/ && !isBadPtr);
						delete jcPosList;
						executing.store(false);
					}).detach();

					break;
				}
				case JCEController::Track:
				{
					if (executing)
						return;
					break;
				}

				default:
					break;
			}
		}

		void stop_jce()
		{
			executing.store(false);
		}

		bool is_execute() const
		{
			return executing;
		}

		void init_ptrs_base(uintptr_t dmc5base)
		{
			jcPrefabBase += dmc5base;
			isPauseBase += dmc5base;
			isPtrBaseInit = true;
			jcSpawn = std::make_unique<func::PrefabInstantiateScale>();
		}

		bool is_ptr_init() const {return isPtrBaseInit; }

	};

    static inline JCEController jceController{};
	static inline bool cheaton = false;

	static inline uintptr_t canExeJceRet = 0;
	static inline uintptr_t canExeJceRet1 = 0;
	static inline uintptr_t subSdtRet = 0;
	static inline uintptr_t jcePfbRet = 0;
	static inline uintptr_t jceTimerRet = 0;
	static inline uintptr_t jceTimerStaticBase = 0;
	static inline uintptr_t stopJceTimerRet = 0;
	static inline uintptr_t jceTimerContinue = 0;


	static inline float humanJCECost = 3000.0f;
	static inline float minSdt = 3000.0f;

	static inline float *pJceExeTime = nullptr;

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
	std::unique_ptr<FunctionHook> m_jce_stop_hook;
};
//clang-format on

