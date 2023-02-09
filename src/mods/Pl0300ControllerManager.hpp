#pragma once
#include <random>

#include "Mod.hpp"
#include "Mods.hpp"
#include "GameplayStateTracker.hpp"
#include "events/Events.hpp"
#include "Pl0300Controller.hpp"
#include "EndLvlHooks.hpp"
#include "EnemySpawner.hpp"
#include "EnemyFixes.hpp"

//clang-format off

class EnemySpawner;

namespace PlCntr
{
	namespace Pl0300Cntr
	{
		class Pl0300ControllerManager : public Mod, private EndLvlHooks::IEndLvl
		{
		private:

			typedef void* (*_EmManager_RequestAddObj)(uintptr_t threadCtxt, uintptr_t emManager, uintptr_t emObj);

			std::unique_ptr<FunctionHook> _pl0300StartHook;
			std::unique_ptr<FunctionHook> _requestAddEmObjHook;
			std::unique_ptr<FunctionHook> _pl0300GetMissionHook;
			std::unique_ptr<FunctionHook> _pl0300CheckDtCancelHook;
			std::unique_ptr<FunctionHook> _checkEmThinkOffHook;
			std::unique_ptr<FunctionHook> _pl0300UpdateLockOnHook;
			std::unique_ptr<FunctionHook> _pl0300TeleportCalcDestHook;
			std::unique_ptr<FunctionHook> _pl0300CheckDamageHook;
			std::unique_ptr<FunctionHook> _pl0300OnChangePlayerActionFromThinkHook;
			std::unique_ptr<FunctionHook> _pl0300OnPreparePlayerActionFromThinkHook;

			static inline Pl0300ControllerManager* _mod = nullptr;

			std::vector<std::shared_ptr<Pl0300Controller>> _pl0300List;

			//Pl0300 spawned via Spawner mod. I need change this to PFbFactory later...
			EnemySpawner* emSpawnerMod;

			uintptr_t requestAddEmFuncAddr = 0;

			void init_check_box_info() override
			{
				m_check_box_name = m_prefix_check_box_name + std::string(get_name());
				m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
			}

			void after_all_inits() override;

			void reset(EndLvlHooks::EndType resetType);

			//---------------------------------------------Hooks for make pl0300 playable-----------------------------------------//

			static void pl0300_start_func_hook(uintptr_t threadCntx, uintptr_t pl0300);

			//Force m21 hit colliders
			static int pl0300_get_mission_n_hook(uintptr_t threadCntx, uintptr_t pl0300);

			//Allow to set DT if char group is enemy
			static bool pl0300_check_dt_cancel_hook(uintptr_t threadCtxt, uintptr_t pl0300);

			//Disable AI when char group is enemy
			static bool check_em_think_off_hook(uintptr_t threadCtxt, uintptr_t character);

			static void pl0300_update_lock_on_hook(uintptr_t threadCtxt, uintptr_t pl0300);

			//Hook for trick moves
			static void pl0300_teleport_calc_dest_hook(uintptr_t threadCtxt, uintptr_t fsmPl0300Teleport);

			//static void pl0300_on_change_pl_action_from_think(uintptr_t threadCtxt, uintptr_t pl0300);

			//static void pl0300_on_prepare_pl_action_from_think(uintptr_t threadCtxt, uintptr_t pl0300);

			//Do not add pl0300 controller to app.EnemyManager.emList
			static naked void em6000_request_add_em_detour();

			//Apply m21 colliders to pl0300 when charGroup = player
			static naked void em6000_damage_check_detour();

			void on_pl_pad_input_reset(uintptr_t pl, bool isAutoPad, bool* callOrig);

			//--------------------------------------------------------------------------------------------------------------------//

		public:
			Pl0300ControllerManager();

			~Pl0300ControllerManager()
			{
				PlayerTracker::before_reset_pad_input_unsub<Pl0300ControllerManager>(std::make_shared<Events::EventHandler<Pl0300ControllerManager, uintptr_t, bool, bool*>>(this, &Pl0300ControllerManager::on_pl_pad_input_reset));
			}

			//Calls automatically by Pl0300Controller.
			std::weak_ptr<Pl0300Controller> register_doppelganger(const Pl0300Controller* controller);

			//Calls automatically by Pl0300Controller.
			void remove_doppelganger(const Pl0300Controller* controller);

			//Change position of all controlled pl0300 (with colliders).
			void set_pos_to_all(gf::Vec3 pos, Pl0300Type type);

			bool destroy_game_obj(const std::weak_ptr<Pl0300Controller>& obj);

			//Try to create, setup and spawn boss Vergil with credit AI.
			//All pl0300 will be destroyed automatically when game will release level resources.
			//Load step is loading async state of enemy prefab manager. Prefab can be valid a little bit earlier then loadstep will be 0 if it wasn't preload before.
			//isKeepingOrigPadInput - do not set original player's pad input to pl0300 if Pl0300Type == PlHelper.
			//Returns empty weak ptr if em prefab isn't valid (but create update request to em prefab manager if possible).
			std::weak_ptr<Pl0300Controller> create_em6000(Pl0300Type controllerType, gf::Vec3 pos, volatile int*& loadStepOut, bool isKeepingOrigPadInput = false);

			//Destroy all pl0300 wich spawned as Pl0300Controller::Pl0300Type::Em6000Friendly
			void kill_all_friendly_em6000();

			Pl0300ControllerManager(const Pl0300ControllerManager& other) = delete;

			static inline uintptr_t requestAddEmRet = 0;
			static inline uintptr_t damageCheckRet = 0;


			std::string_view get_name() const override
			{
				return "Pl0300ControllerManager";
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
			/*void on_config_load(const utility::Config& cfg) override {}
			void on_config_save(utility::Config& cfg) override {}*/

			// on_draw_ui() is called only when the gui shows up
			// you are in the imgui window here.
			void on_draw_ui() override;

			static void is_pl0300_controller_asm(uintptr_t threadCtxt, uintptr_t emManager, uintptr_t pl0300);

			static bool check_pl0300_asm(uintptr_t pl0300);
		};
	}
}

//clang-format on