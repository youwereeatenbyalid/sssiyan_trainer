#pragma once
#include "Mod.hpp"
#include "Mods.hpp"
#include "PlayerTracker.hpp"
#include "GameplayStateTracker.hpp"
#include "EndLvlHooks.hpp"
#include "EnemySpawner.hpp"
#include "InputSystem.hpp"
#include "Pl0300ControllerManager.hpp"
#include "VergilDoppelInitSetup.hpp"
#include "PlSetActionData.hpp"
#include "GameFunctions/PlayerCheckNormalJump.hpp"
#include "GameFunctions/PlayerSetDT.hpp"
#include "VergilQuickSilver.hpp"
#include "WitchTime.hpp"
#include "EnemyData.hpp"

//clang-format off
namespace gf = GameFunctions;

class BossVergilMoves : public Mod, private EndLvlHooks::IEndLvl
{
public:
	enum class Pl0300TrickType
	{
		ToEnemy,
		InPlace
	};

	enum class StabReaction
	{
		None = 0, 
		Stun = 3200,
		Dying = 3230,
		DamageStandL = 3001,//0xBB9
		SlamDamage = 3040,
		DamageFlyL = 3092,
		DamageDownFaceUp = 3102,
		DamageWallLand = 3220,
		DamageDiagonalBlown = 3060
	};

private:

	Pl0300TrickType _pl0300TrickType = Pl0300TrickType::ToEnemy;

	class PlPair
	{

	public:

		void destroy_doppel() noexcept
		{
			_doppelDestroyCoroutine.stop();
			_doppelUpdateCoroutine.stop();

			if(auto pl0300 = _pl0300.lock(); pl0300 != nullptr && pl0300->get_doppel() != 0)
			{
				auto doppel = pl0300->get_doppel_ctrl().lock();
				if (doppel != nullptr)
				{
					doppel->set_action(L"Wait");
					doppel->pl_reset_status();
				}
				pl0300->destroy_doppel();
			}
		}

	private:

		enum class Pl0300Actions
		{
			AirRaid,
			Stab,
			DodgeLeft,
			DodgeRight
		};

		enum class LastPlSwap
		{
			ViaPlId,
			ViaCamSwap
		};

		LastPlSwap _lastPlSwap = LastPlSwap::ViaPlId;

		static inline const std::array<const wchar_t*, 12> _airRaidNames
		{
			L"Concentrate_Start",//0
			L"Concentrate_Loop",//1
			L"Concentrate_PreEnd",//2
			L"Em6000AirRaidStart",//3
			L"Em6000Demon_Air_Raid_Start_Loop",//4
			L"Em6000Demon_Air_Raid_Start_End",//5
			L"Em6000Demon_Air_Raid_Assault_Start",//6
			L"new state 415263B9",//7
			L"TeleportOnAirRaid",//8
			L"AppearOnAirRaid",//9
			L"LoopAfterAppearOnAirRaid",//10
			L"Em6000Demon_Air_Raid_Assault_End",//11
			//"Wait"
		};

		static inline const std::array<const wchar_t*, 7> _trickStabNames
		{
			L"Production_Throw_Attack",//0
			L"ProductionThrowAttackMain",//1
			L"Stab",//2
			L"new state 950FBA69",//Hit pl object 3
			L"new state F82A490A",//after hit pl object 4 
			L"Stab_Hit" //5
			L"Stab_Miss",//6
			L"StabMiss"//7
			//"Wait"
		};

		std::vector<uintptr_t> _stabHitInfoList;

		static inline std::unique_ptr<gf::SysString> _stabStr = nullptr;

		static inline std::mt19937 _rndGen{};

		static inline std::uniform_real_distribution<float> _rndTeleportOffsXY{ -3.25f, 4.15f};

		static inline sdk::REMethodDefinition* _transformSetLocalScaleMethod;
		static inline sdk::REMethodDefinition* _plDoCharUpdateMethod;
		static inline sdk::REMethodDefinition* _pl0800ResetStatusMethod;
		static inline sdk::REMethodDefinition* _pl0800EndCutSceneMethod;
		static inline sdk::REMethodDefinition* _plCamCntrlSetPlAccMethod;
		static inline sdk::REMethodDefinition* _emSetActionMethod;
		static inline sdk::REMethodDefinition* _charSetLockOnTargetMethod;
		static inline sdk::REMethodDefinition* _targetCntrSetTargetMethod;
		static inline sdk::REMethodDefinition* _targetCntrUpdateMethod;
		static inline sdk::REMethodDefinition* _workRateSetHitStopMethod;

		static inline sdk::RETypeDefinition* _lockOnObjTD;
		static inline sdk::RETypeDefinition* _plAccessorTD;

		sdk::REMethodDefinition* _gameModelSetDrawSelfMethod = nullptr;
		sdk::REMethodDefinition* _gameModelSetEnableMethod = nullptr;
		sdk::REMethodDefinition* _networkBBUpdateMethod = nullptr;
		sdk::REMethodDefinition* _networkBBUpdateNetworkTypeMethod = nullptr;

		REManagedObject* _pl0300Accessor = nullptr;

		static inline std::unique_ptr<FunctionHook> _plCamCntrSetPlHook = nullptr;
		static inline uintptr_t _plCamCntrlSetPlAddr = 0;

		static inline bool _isStaticInitRequested = true;
		static inline bool _isBossMovePerforming = false;
		static inline bool _isCameraSetSkipRequested = false;

		static inline int _pairCount = 0;

		bool _isAfterFirstAirRaidState = false;
		bool _isAfterFirstTrickStabState = false;

		bool _isTrickStabPerforming = false;

		bool _isPl0300Active = false;

		gf::Vec3 _airRaidStartPos;
		gf::Vec3 _moveStartPos;

		const InputSystem* _inputSys = nullptr;

		uintptr_t _pl0800LastLockOnTargetWork = 0;
		uintptr_t _pl0800LastPlAccessor = 0;

		static inline void pl_cam_cntrl_set_pl_hook(uintptr_t threadCntx, uintptr_t obj, uintptr_t plAcessor)
		{
			//Disallow cam set while pl0300 is active
			if (_isCameraSetSkipRequested)
				return;
			_plCamCntrSetPlHook->get_original<decltype(pl_cam_cntrl_set_pl_hook)>()(threadCntx, obj, plAcessor);
		}

		void on_photo_mode_open(uintptr_t threadCntx, uintptr_t ui3500GUI)
		{
			if (!_isPl0300Active)
				return;
			auto pl0300 = _pl0300.lock();
			if (pl0300 == nullptr)
				return;
			gf::Transform_SetPosition::set_character_pos(_pl0800, pl0300->get_pl_pos());
			pl0300->get_network_base_bhvr_update_method()->call(threadCntx, _pl0800);
		}

		void on_photo_mode_closed(uintptr_t threadCntx, uintptr_t ui3500GUI)
		{
			if (!_isPl0300Active)
				return;
			auto pl0300 = _pl0300.lock();
			if (pl0300 == nullptr)
				return;
			gf::Transform_SetPosition::set_character_pos(_pl0800, _moveStartPos);
			pl0300->get_network_base_bhvr_update_method()->call(threadCntx, _pl0800);
		}

		void on_pl0300_trick_update(uintptr_t threadCntxt, uintptr_t fsmPl0300Teleport, std::shared_ptr<PlCntr::Pl0300Cntr::Pl0300Controller> pl0300, bool* skipOrig)
		{
			auto upl0300 = pl0300->get_pl();
			if (upl0300 != _pl0300.lock()->get_pl())
				return;
			auto ccc = *(uintptr_t*)(upl0300 + 0x1818);
			if (ccc == 0)
				return;
			*skipOrig = true;
			if (*(int*)(upl0300 + 0x1AE0) == 6)
			{
				auto characterTarget = *(uintptr_t*)(ccc + 0x58);
				if (characterTarget == 0)
					return;
				gf::Vec3 targetPos = *(gf::Vec3*)(ccc + 0x60);
				gf::Vec3 newPos(targetPos.x + _rndTeleportOffsXY(_rndGen), targetPos.y + _rndTeleportOffsXY(_rndGen), targetPos.z);
				*(gf::Vec3*)(upl0300 + 0x1f40) = *(gf::Vec3*)(fsmPl0300Teleport + 0x80) = *(gf::Vec3*)(fsmPl0300Teleport + 0x80) = newPos;
				return;
			}
			else
			{
				auto characterTarget = *(uintptr_t*)(ccc + 0x58);
				gf::Vec3 targetPos = *(gf::Vec3*)(ccc + 0x60);
				if (_stabTrickUpdateType == Pl0300TrickType::ToEnemy && characterTarget != 0)
				{
					gf::Vec3 newPos(targetPos.x + _rndTeleportOffsXY(_rndGen), targetPos.y + _rndTeleportOffsXY(_rndGen), (*(gf::Vec3*)(characterTarget + 0x3E0)).z);
					*(gf::Vec3*)(pl0300->get_pl() + 0x1f40) = *(gf::Vec3*)(fsmPl0300Teleport + 0x80) = newPos;
				}
				else
					*(gf::Vec3*)(pl0300->get_pl() + 0x1f40) = *(gf::Vec3*)(fsmPl0300Teleport + 0x80) = pl0300->get_ground_pos();
				*(int*)(pl0300->get_pl() + 0x2008) = 24; //24 - Stab //25 - StabFromWalk;
			}
		}

		void after_pl0300_lockon_update(uintptr_t threadCntxt, std::shared_ptr<PlCntr::Pl0300Cntr::Pl0300Controller> pl0300)
		{
			if (pl0300->get_pl() != _pl0300.lock()->get_pl() || !check_target_work_valid(_pl0800LastLockOnTargetWork))
				return;
			else
			{
				auto uPl0300 = _pl0300.lock()->get_pl();
				auto pl0300TargetCntrl = *(uintptr_t*)(uPl0300 + 0x300);
				auto character = *(uintptr_t*)(_pl0800LastLockOnTargetWork + 0x58);
				_targetCntrSetTargetMethod->call(threadCntxt, pl0300TargetCntrl, 5, character);
				_targetCntrUpdateMethod->call(threadCntxt, pl0300TargetCntrl);
			}
		}

		void on_pl0300_lockon_target_update(uintptr_t threadCntxt, std::shared_ptr<PlCntr::Pl0300Cntr::Pl0300Controller> pl0300, bool* skipOrigCall)
		{
			if (pl0300->get_pl() != _pl0300.lock()->get_pl() || !check_target_work_valid(_pl0800LastLockOnTargetWork))
				return;
			else
			{
				*skipOrigCall = true;
				auto uPl0300 = _pl0300.lock()->get_pl();
				auto pl0300TargetCntrl = *(uintptr_t*)(uPl0300 + 0x300);
				auto character = *(uintptr_t*)(_pl0800LastLockOnTargetWork + 0x58);
				_targetCntrSetTargetMethod->call(threadCntxt, pl0300TargetCntrl, 5, character);
				_targetCntrUpdateMethod->call(threadCntxt, pl0300TargetCntrl);
			}
		}

		void update_doppel()
		{
			if (GameplayStateTracker::isCutscene)
				destroy_doppel();
		}

		inline void stop_air_raid_coroutine()
		{
			_isAfterFirstAirRaidState = false;
			if (!_pl0300ActionUpdateCoroutine.is_started())
				return;
			_pl0300ActionUpdateCoroutine.stop();
			if (auto pl0300 = _pl0300.lock(); pl0300 != nullptr)
			{
				pl0300->destroy_all_related_shells();
				set_pl0300_scale(pl0300->get_pl(), gf::Vec3(1.0f, 1.0f, 1.0f));
			}
		}

		inline void stop_trick_stab_coroutine()
		{
			_isAfterFirstTrickStabState = _isTrickStabPerforming = false;
			_pl0300ActionUpdateCoroutine.stop();
			if (auto pl0300 = _pl0300.lock(); pl0300 != nullptr && pl0300->get_cur_dt() == PlCntr::DT::SDT)
				pl0300->set_dt(PlCntr::DT::Human);
		}

		void set_pl0300_scale(uintptr_t pl0300, gf::Vec3 scale)
		{
			if (_transformSetLocalScaleMethod == nullptr)
				return;
			auto transform = *(uintptr_t*)((*(uintptr_t*)(pl0300 + 0x10)) + 0x18);
			_transformSetLocalScaleMethod->call(sdk::get_thread_context(), transform, scale);
		}

		inline void pl0300_action_end_char_swap()
		{
			if (_lastPlSwap == LastPlSwap::ViaPlId)
				change_manual_pl(4);
			else
				change_pl_via_cam((uintptr_t)sdk::get_thread_context(), 4);
		}

		inline void update_stab_reaction(const std::shared_ptr<PlCntr::Pl0300Cntr::Pl0300Controller> &pl0300)
		{
			if (_stabEmReaction == StabReaction::None)
				return;
			auto pl0300HC = *(uintptr_t*)(pl0300->get_pl() + 0x1F8);
			auto pl0300AttackList = *(uintptr_t*)(pl0300HC + 0x268);
			if (pl0300AttackList == 0)
				return;
			int attackListCount = gf::ListController::get_list_count(pl0300AttackList);
			for (int i = 0; i < attackListCount; i++)
			{
				auto hitInfo = gf::ListController::get_item<uintptr_t>(pl0300AttackList, i);
				if (hitInfo == 0 || std::find_if(_stabHitInfoList.begin(), _stabHitInfoList.end(), [&](uintptr_t arg)
						{
							return hitInfo == arg;
						}) != _stabHitInfoList.end()
					)
					continue;
				_stabHitInfoList.push_back(hitInfo);
				if (hitInfo == 0)
					continue;
				auto damageHC = *(uintptr_t*)(hitInfo + 0xC0);
				auto damagedChar = *(uintptr_t*)(damageHC + 0xA0);
				if (damagedChar == 0)
					continue;
				if (auto emId = EnemyData::get_em_id(damagedChar); emId != EnemyData::Dante && emId != EnemyData::Vergil && emId != EnemyData::None)
				{
					_emSetActionMethod->call(sdk::get_thread_context(), damagedChar, (int32_t)_stabEmReaction, 0, 10.0f, 0.0f, 1, 0, true);
					if (_isSetHitStopAfterStab)
					{
						auto workRate = *(uintptr_t*)(damagedChar + 0x200);
						if (workRate != 0)
							_workRateSetHitStopMethod->call(sdk::get_thread_context(), workRate, _hitStopRate, _hitStopTime, false);
					}
				}
			}
		}

		void update_pl0300_behavior(Pl0300Actions action)
		{
			auto pl0300 = _pl0300.lock();
			if (pl0300 == nullptr || GameplayStateTracker::isCutscene)
			{
				force_end_moves();
				return;
			}
			if (_inputSys->is_action_button_pressed(InputSystem::PadInputGameAction::CameraReset) && 
				_inputSys->is_action_button_pressed(InputSystem::PadInputGameAction::ChangeLockOn))
				force_end_moves();
			auto actionStr = gf::StringController::get_raw_wstr(*(uintptr_t*)(pl0300->get_pl() + 0x190));
			switch (action)
			{
				case BossVergilMoves::PlPair::Pl0300Actions::AirRaid:
				{
					//pl0300->set_hp(20000.0f);
					if (!_isAfterFirstAirRaidState)
					{
						if (wcscmp(actionStr, _airRaidNames[0]) == 0)
						{
							_isAfterFirstAirRaidState = true;
							pl0300->set_is_control(true);
						}

					}
					else 
					{
						auto it = std::find_if(_airRaidNames.begin(), _airRaidNames.end(), [&](const wchar_t* name) { return wcscmp(name, actionStr) == 0; });
						if (it == _airRaidNames.end())
						{
							pl0300->set_dt(PlCntr::DT::Human);
							stop_air_raid_coroutine();
							auto airRaidCtrl = *(uintptr_t*)(pl0300->get_pl() + 0x1C90);
							if (airRaidCtrl != 0)
							{
								if (*(bool*)(airRaidCtrl + 0x30))
									pl0300->set_pos_full(_moveStartPos);
							}
							pl0300_action_end_char_swap();
							return;
						}
					}
					
					if (_isAfterFirstAirRaidState && pl0300->get_cur_dt() == PlCntr::DT::SDT && wcscmp(actionStr, _airRaidNames[_airRaidNames.size() - 1]) == 0)
					{
							pl0300->set_dt(PlCntr::DT::Human);
							set_pl0300_scale(pl0300->get_pl(), gf::Vec3(1.0f, 1.0f, 1.0f));
					}
					break;
				}
				case BossVergilMoves::PlPair::Pl0300Actions::Stab:
				{
					if (!_isAfterFirstTrickStabState)
					{
						if (wcscmp(actionStr, _trickStabNames[0]) == 0)
						{
							_isAfterFirstTrickStabState = true;
							pl0300->set_is_control(true);
						}
					}
					else
					{
						auto it = std::find_if(_trickStabNames.begin(), _trickStabNames.end(), [&](const wchar_t* name) { return wcscmp(name, actionStr) == 0; });
						if (it == _trickStabNames.end())
						{
							stop_trick_stab_coroutine();
							pl0300_action_end_char_swap();
							return;
						}
						update_stab_reaction(pl0300);
					}
					break;
				}
				case BossVergilMoves::PlPair::Pl0300Actions::DodgeLeft:
				{

					break;
				}
				case BossVergilMoves::PlPair::Pl0300Actions::DodgeRight:
				{

					break;
				}
				default:
				{
					stop_air_raid_coroutine();
					break;
				}
			}
		}

		bool check_target_work_valid(uintptr_t lockOnTargetWork)
		{
			if (lockOnTargetWork == 0)
				return false;
			auto lockOnTargetManager = (uintptr_t)sdk::get_managed_singleton<REManagedObject*>("app.LockOnTargetManager");
			if (lockOnTargetManager == 0)
				return false;
			auto accessableList = *(uintptr_t*)(lockOnTargetManager + 0x78);
			for (int i = 0; i < gf::ListController::get_list_count(accessableList); i++)
			{
				auto item = gf::ListController::get_item<uintptr_t>(accessableList, i);
				if (lockOnTargetWork== item)
					return true;
			}
			return false;
		}

		void set_pl0800_lock_on_to_pl0300(const std::shared_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>& pl0300, uintptr_t threadCntx)
		{
			auto uPl0300 = pl0300->get_pl();
			auto pl0300PadInput = *(uintptr_t*)(uPl0300 + 0xEF0);
			auto pl0300EnemyPadInput = *(uintptr_t*)(uPl0300 + 0xEF8);
			auto pl0300TracePadInput = *(uintptr_t*)(uPl0300 + 0xF00);

			if (pl0300PadInput != 0)
			{
				*(bool*)(pl0300PadInput + 0x24) = false;
				*(bool*)(pl0300PadInput + 0x28) = true;
			}

			auto pl0800LockOnObj = *(uintptr_t*)(_pl0800 + 0x428);
			if (pl0800LockOnObj == 0)
				return;
			auto pl0800LockOnTargetWork = *(uintptr_t*)(pl0800LockOnObj + 0x10);
			if (pl0800LockOnTargetWork == 0)
				return;

			_pl0800LastLockOnTargetWork = pl0800LockOnTargetWork;

			auto ccc = *(uintptr_t*)(uPl0300 + 0x1818);
			auto character = *(uintptr_t*)(pl0800LockOnTargetWork + 0x58);
			if (ccc != 0)
			{
				//*(uintptr_t*)(ccc + 0x58) = character;
				sdk::call_object_func_easy<void*>((REManagedObject*)ccc, "set_target(app.character.Character)", character);
				*(gf::Vec3*)(ccc + 0x60) = *get_char_pos(character);
			}

			auto pl0300TargetCntrl = *(uintptr_t*)(uPl0300 + 0x300);
			_targetCntrSetTargetMethod->call(threadCntx, pl0300TargetCntrl, 5, character);
			_targetCntrUpdateMethod->call(threadCntx, pl0300TargetCntrl);
		}

		Coroutines::Coroutine<decltype(&PlPair::destroy_doppel), PlPair*> _doppelDestroyCoroutine{ &PlPair::destroy_doppel, false };
		Coroutines::Coroutine<decltype(&PlPair::update_doppel), PlPair*> _doppelUpdateCoroutine{ &PlPair::update_doppel, false, true };
		Coroutines::Coroutine<decltype(&PlPair::update_pl0300_behavior), PlPair*, Pl0300Actions> _pl0300ActionUpdateCoroutine{ &PlPair::update_pl0300_behavior, true, true };

	public:

		const uintptr_t _pl0800;
		const std::weak_ptr<PlCntr::Pl0300Cntr::Pl0300Controller> _pl0300;

		static inline Pl0300TrickType _stabTrickUpdateType = Pl0300TrickType::ToEnemy;

		static inline StabReaction _stabEmReaction = StabReaction::DamageStandL;

		PlPair() = delete;
		PlPair(uintptr_t pl0800, std::weak_ptr<PlCntr::Pl0300Cntr::Pl0300Controller> pl0300, const InputSystem* inputSysMod, uintptr_t threadCntx) : _pl0800(pl0800), _pl0300(pl0300), _inputSys(inputSysMod)
		{
			_doppelDestroyCoroutine.ignoring_update_on_pause(true);
			_gameModelSetDrawSelfMethod = pl0300.lock()->get_game_model_set_draw_self_method();
			_gameModelSetEnableMethod = pl0300.lock()->get_game_model_set_enable_method();
			_networkBBUpdateMethod = pl0300.lock()->get_network_base_bhvr_update_method();
			_networkBBUpdateNetworkTypeMethod = pl0300.lock()->get_network_base_bhvr_update_network_type_method();

			_pl0300ActionUpdateCoroutine.set_delay(0);
			_pl0300ActionUpdateCoroutine.ignoring_update_on_pause(true);

			_doppelUpdateCoroutine.set_delay(0.2f);
			_doppelUpdateCoroutine.ignoring_update_on_pause(true);

			_pl0300Manager->on_pl0300_teleport_calc_destination_sub(std::make_shared<Events::EventHandler<PlPair, uintptr_t, uintptr_t, std::shared_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>, bool*>>
				(this, &PlPair::on_pl0300_trick_update));
			_pl0300Manager->after_pl0300_update_lock_on_sub(std::make_shared<Events::EventHandler<PlPair, uintptr_t, std::shared_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>>>
				(this, &PlPair::after_pl0300_lockon_update));
			_pl0300Manager->on_pl0300_update_lock_on_target_sub(std::make_shared<Events::EventHandler<PlPair, uintptr_t, std::shared_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>, bool*>>
				(this, &PlPair::on_pl0300_lockon_target_update));

			GameplayStateTracker::on_ui3500_gui_open_sub(std::make_shared<Events::EventHandler<PlPair, uintptr_t, uintptr_t>>(this, &PlPair::on_photo_mode_open));
			GameplayStateTracker::on_ui3500_gui_closed_sub(std::make_shared<Events::EventHandler<PlPair, uintptr_t, uintptr_t>>(this, &PlPair::on_photo_mode_closed));

			if (_isStaticInitRequested)
			{
				_transformSetLocalScaleMethod = sdk::find_method_definition("via.Transform", "set_LocalScale(via.vec3)");
				_plDoCharUpdateMethod = sdk::find_method_definition("app.Player", "doCharacterUpdate()");
				_pl0800ResetStatusMethod = sdk::find_method_definition("app.PlayerVergilPL", "resetStatus(app.GameModel.ResetType)");
				_pl0800EndCutSceneMethod = sdk::find_method_definition("app.PlayerVergilPL", "endCutScene(System.Int32, System.Single, app.character.Character.WetType, System.Single)");
				_plCamCntrlSetPlAccMethod = sdk::find_method_definition("app.PlayerCameraController", "set_player(app.IPlayerAccessor)");
				_emSetActionMethod = sdk::find_method_definition("app.Enemy",
					"setAction(app.Enemy.ActionEnum, System.UInt32, System.Single, System.Single, via.motion.InterpolationMode, via.motion.InterpolationCurve, System.Boolean)");
				_charSetLockOnTargetMethod = sdk::find_method_definition("app.character.Character", "set_lockOnTarget(app.LockOnObject)");
				_targetCntrSetTargetMethod = sdk::find_method_definition("app.TargetController", "setTarget(app.TargetController.TargetTypeEnum, app.character.Character)");//enum 5
				_targetCntrUpdateMethod = sdk::find_method_definition("app.TargetController", "update()");
				_workRateSetHitStopMethod = sdk::find_method_definition("app.WorkRate", "setHitStop(System.Single, System.Single, System.Boolean)");

				_lockOnObjTD = sdk::find_type_definition("app.LockOnObject");
				_plAccessorTD = sdk::find_type_definition("app.PlayerAccessor");
				
				_stabStr = std::make_unique<gf::SysString>(L"Stab");

				_plCamCntrlSetPlAddr = g_framework->get_module().as<uintptr_t>() + 0xCD1920;

				_isStaticInitRequested = false;
			}

			if(_plCamCntrSetPlHook == nullptr)
			{
				_plCamCntrSetPlHook = std::make_unique<FunctionHook>(_plCamCntrlSetPlAddr, &pl_cam_cntrl_set_pl_hook, false);
				_plCamCntrSetPlHook->create();
			}

			auto shared = pl0300.lock();
			if (shared == nullptr)
				return;
			_pl0300Accessor = _plAccessorTD->create_instance_full();
			PfbFactory::PrefabFactory::add_ref(_pl0300Accessor);
			auto uPlAcc = (uintptr_t)_pl0300Accessor;
			*(uintptr_t*)(uPlAcc + 0x10) = shared->get_transform();
			//*(uintptr_t*)(uPlAcc + 0x18) = pl0300->get_pl();
			sdk::get_object_method(_pl0300Accessor, "set_player(app.Player)")->call(threadCntx, _pl0300Accessor, shared->get_pl());
			_pairCount++;
		}

		~PlPair()
		{
			destroy_doppel();
			stop_air_raid_coroutine();
			_pl0300ActionUpdateCoroutine.stop();
			_plCamCntrSetPlHook = nullptr;
			_pl0300Manager->on_pl0300_teleport_calc_destination_unsub(std::make_shared<Events::EventHandler<PlPair, uintptr_t, uintptr_t, std::shared_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>, bool*>>
				(this, &PlPair::on_pl0300_trick_update));
			_pl0300Manager->after_pl0300_update_lock_on_unsub(std::make_shared<Events::EventHandler<PlPair, uintptr_t, std::shared_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>>>
				(this, &PlPair::after_pl0300_lockon_update));
			_pl0300Manager->on_pl0300_update_lock_on_target_unsub(std::make_shared<Events::EventHandler<PlPair, uintptr_t, std::shared_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>, bool*>>
				(this, &PlPair::on_pl0300_lockon_target_update));
			GameplayStateTracker::on_ui3500_gui_open_unsub(std::make_shared<Events::EventHandler<PlPair, uintptr_t, uintptr_t>>(this, &PlPair::on_photo_mode_open));
			GameplayStateTracker::on_ui3500_gui_closed_unsub(std::make_shared<Events::EventHandler<PlPair, uintptr_t, uintptr_t>>(this, &PlPair::on_photo_mode_closed));
			if (_pl0300Accessor != nullptr)
				PfbFactory::PrefabFactory::release(_pl0300Accessor);
			_pl0300Accessor = nullptr;
			_pl0300Manager->destroy_game_obj(_pl0300);
			_isCameraSetSkipRequested = false;
			_pairCount--;
			if (_pairCount == 0)
				_plCamCntrSetPlHook = nullptr;
		}

		void on_pl_reload_reset()
		{
			force_end_moves();
			_isTrickStabPerforming = false;
			_isAfterFirstAirRaidState = _isAfterFirstTrickStabState = false;
			_pl0300ActionUpdateCoroutine.stop();
		}

		static inline sdk::REMethodDefinition* get_pl_do_char_update_method() noexcept { return _plDoCharUpdateMethod; }

		inline bool is_doppel_mode() const noexcept { return _pl0300.lock()->get_doppel() != 0; }

		inline bool is_trick_stab_performing() const noexcept { return _isTrickStabPerforming; }

		std::weak_ptr<PlCntr::Pl0300Cntr::Pl0300Controller> createDoppel(bool useLifeTime, float lifeTime, gf::Vec3 doppelScale = gf::Vec3{1.0f, 1.0f, 1.0f}, bool isFirst = false, 
			float hp = 250.0f, float AttackRate = 0.35f) noexcept
		{
			auto pl0300 = _pl0300.lock();
			if (pl0300 == nullptr && _pl0800 == 0)
				return std::weak_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>();
			if (is_doppel_mode())
				return _pl0300.lock()->get_doppel_ctrl();
			
			pl0300->set_pos_full(*get_char_pos(_pl0800));
			pl0300->generate_doppel(isFirst, hp, AttackRate);
			auto pl0300Doppel = pl0300->get_doppel_ctrl().lock();
			if ( pl0300Doppel != nullptr)
			{
				set_pl0300_scale(pl0300Doppel->get_pl(), gf::Vec3(1.15f, 1.15f, 1.15f));
				if (useLifeTime)
				{
					_doppelDestroyCoroutine.set_delay(lifeTime);
					_doppelDestroyCoroutine.start(this);
				}
				_doppelUpdateCoroutine.start(this);
			}
			pl0300->set_draw_self(false);
			pl0300->set_enable(false);
			pl0300->enable_physics_char_controller(false);
			return pl0300Doppel;
		}

		void force_end_moves()
		{
			auto pl0300 = _pl0300.lock();
			if (pl0300 == nullptr || !_pl0300ActionUpdateCoroutine.is_started())
				return;
			if(_lastPlSwap == LastPlSwap::ViaPlId)
			{
				if (int* curPlId = get_cur_pl_id((uintptr_t)(pl0300->get_mission_setting_manager())); curPlId == nullptr || *curPlId == 4)
					return;
			}
			else if(gf::StringController::str_cmp(*(uintptr_t*)(pl0300->get_pl() + 0x190), "Wait"))
				return;
			if(pl0300->get_cur_dt() == PlCntr::DT::SDT)
				pl0300->set_dt(PlCntr::DT::Human);
			set_pl0300_scale(pl0300->get_pl(), gf::Vec3(1.0f, 1.0f, 1.0f));
			_pl0300ActionUpdateCoroutine.stop();
			_isAfterFirstAirRaidState = _isAfterFirstTrickStabState = _isTrickStabPerforming = false;
			pl0300->set_pos_full(_moveStartPos);
			gf::Transform_SetPosition::set_character_pos(_pl0800, _moveStartPos);
			if (_lastPlSwap == LastPlSwap::ViaPlId)
				change_manual_pl(4);
			else
				change_pl_via_cam((uintptr_t)sdk::get_thread_context(), 4);
		}

		void change_pl0300_enable_state(bool enable)
		{
			auto pl0300 = _pl0300.lock();
			if (pl0300 == nullptr)
				return;

			pl0300->set_enable(enable);
			pl0300->set_draw_self(enable);
			pl0300->enable_physics_char_controller(enable);
			pl0300->set_is_control(enable);
		}

		void change_pl0800_enable_state(bool enable)
		{
			*(uintptr_t*)(_pl0800 + 0x4C6) = enable;
			change_pl0800_physics_char_controller_state(enable);
			_gameModelSetDrawSelfMethod->call(sdk::get_thread_context(), _pl0800, enable);
			_gameModelSetEnableMethod->call(sdk::get_thread_context(), _pl0800, enable);
		}

		void change_pl0800_physics_char_controller_state(bool enable)
		{
			auto charController = *(uintptr_t*)(_pl0800 + 0x2F0);
			auto subCharController = *(uintptr_t*)(_pl0800 + 0x2F8);
			if(charController != 0 && subCharController != 0)
				*(bool*)(charController + 0x30) = *(bool*)(subCharController + 0x30) = enable;
		}

		void start_air_raid(gf::Vec3 centerFloor, const PlCntr::Pl0300Cntr::AirRaidSettings &settings, const PlCntr::HitControllerSettings &hcSettings, bool useCoroutine = true)
		{
			auto pl0300 = _pl0300.lock();
			if (pl0300 == nullptr)
				return;
			if (pl0300->get_doppel() != 0)
				destroy_doppel();
			_airRaidStartPos = *get_char_pos(_pl0800);
			auto emParam = pl0300->get_pl();
			emParam = *(uintptr_t*)(emParam + 0x1768);
			if (emParam == 0)
				return;
			auto devilParam = *(uintptr_t*)(emParam + 0x118);
			if (devilParam != 0)
				*(float*)(devilParam + 0x1C) = *(float*)(devilParam + 0x20) = 0;
			_moveStartPos = *get_char_pos(_pl0800);
			pl0300->set_center_floor(centerFloor);

			pl0300->set_air_raid_settings(settings, *(gf::Vec3*)(_pl0800 + 0x3E0));
			pl0300->set_hitcontroller_settings(hcSettings);
			*get_char_rot(pl0300->get_pl()) = *get_char_rot(_pl0800);
			pl0300->set_is_control(true);
			pl0300->set_dt(PlCntr::DT::SDT);
			pl0300->set_action(_airRaidNames[0]);
			set_pl0300_scale(pl0300->get_pl(), gf::Vec3(1.15f, 1.15f, 1.15f));
			auto ccc = *(uintptr_t*)(pl0300->get_pl() + 0x1818);
			if (ccc != 0 && *(uintptr_t*)(ccc + 0x58) != 0)
			{
				auto characterTarget = *(uintptr_t*)(ccc + 0x58);
				gf::Vec3 targetPos = *(gf::Vec3*)(ccc + 0x60);
				gf::Vec3 newPos(targetPos.x + _rndTeleportOffsXY(_rndGen), targetPos.y + _rndTeleportOffsXY(_rndGen), targetPos.z);
				*(gf::Vec3*)(pl0300->get_pl() + 0x1f40) = newPos;
				*(int*)(pl0300->get_pl() + 0x2008) = 37; //24 - Stab //25 - StabFromWalk;
				*(int*)(pl0300->get_pl() + 0x1AE0) = 6;
			}
			pl0300->update_em_teleport();
			if(useCoroutine)
				_pl0300ActionUpdateCoroutine.start(this, Pl0300Actions::AirRaid);
		}

		void start_trick_stab(gf::Vec3 centerOfFloor, const PlCntr::HitControllerSettings& hcSettings, Pl0300TrickType warpType)
		{
			auto pl0300 = _pl0300.lock();
			if (pl0300 == nullptr)
				return;
			auto emParam = pl0300->get_pl();
			emParam = *(uintptr_t*)(emParam + 0x1768);
			if (emParam == 0)
				return;
			pl0300->set_center_floor(centerOfFloor);

			pl0300->set_hitcontroller_settings(hcSettings);
			pl0300->set_hp(20000.0f);
			_moveStartPos = *get_char_pos(_pl0800);

			pl0300->set_is_control(true);

			auto ccc = *(uintptr_t*)(pl0300->get_pl() + 0x1818);
			//pl0300->update_em_teleport();
			if (ccc != 0 && *(uintptr_t*)(ccc + 0x58) != 0)
			{
				auto characterTarget = *(uintptr_t*)(ccc + 0x58);
				gf::Vec3 targetPos = *(gf::Vec3*)(ccc + 0x60);
				if (warpType == Pl0300TrickType::ToEnemy)
				{
					gf::Vec3 newPos(targetPos.x + _rndTeleportOffsXY(_rndGen), targetPos.y + _rndTeleportOffsXY(_rndGen), (*(gf::Vec3*)(characterTarget + 0x3E0)).z);
					*(gf::Vec3*)(pl0300->get_pl() + 0x1f40) = newPos;
				}
				else
					*(gf::Vec3*)(pl0300->get_pl() + 0x1f40) = pl0300->get_ground_pos();
				*(int*)(pl0300->get_pl() + 0x2008) = 24; //24 - Stab //25 - StabFromWalk;
				*(int*)(pl0300->get_pl() + 0x1AE0) = 5;
			}
			pl0300->get_network_base_bhvr_update_method()->call(sdk::get_thread_context(), pl0300->get_pl());
			_stabHitInfoList.clear();
			pl0300->set_action(_stabStr.get());
			
			//pl0300->set_action_from_think(_stabStr.get(), 0x94A9A36A);
			_pl0300ActionUpdateCoroutine.start(this, Pl0300Actions::Stab);
			_isTrickStabPerforming = true;
		}

		int *get_cur_pl_id(uintptr_t missionSettingMngr) const noexcept
		{
			if (missionSettingMngr == 0)
				return nullptr;
			auto settings = *(uintptr_t*)(missionSettingMngr + 0xA0);
			if (settings == 0)
				return nullptr;
			auto plInfo = *(uintptr_t*)(settings + 0x10);
			if (plInfo == 0)
				return nullptr;
			return (int*)(plInfo + 0x10);
		}

		void change_pl_via_cam(uintptr_t threadCntxt, int plId, bool callPl0800EndCutscene = true)
		{
			auto pl0300 = _pl0300.lock();
			if (pl0300 == nullptr)
				return;
			if (pl0300->get_cur_mediation_type() != 2)
			{
				pl0300->set_network_base_active(true);
				pl0300->get_network_base_bhvr_update_method()->call(threadCntxt, pl0300->get_pl());
			}
			if (*get_cur_pl_id((uintptr_t)(pl0300->get_mission_setting_manager())) == 3 && plId == 3)
				return;
			auto camCntr = get_pl_camera_controller();
			if (camCntr == 0)
				return;
			if (plId == 3)
			{
				set_pl0800_lock_on_to_pl0300(pl0300, threadCntxt);
				pl0300->set_pos_full(*get_char_pos(_pl0800));
				*get_char_rot(pl0300->get_pl()) = *get_char_rot(_pl0800);
				*(bool*)(_pl0800 + 0x4C6) = false;
				pl0300->set_hp(20000.0f);
				if (callPl0800EndCutscene && _pl0800EndCutSceneMethod != nullptr)
					_pl0800EndCutSceneMethod->call(sdk::get_thread_context(), _pl0800, 0, 0, 0, 0);
				if (_pl0800ResetStatusMethod != nullptr)
					_pl0800ResetStatusMethod->call(sdk::get_thread_context(), _pl0800, 0);
				change_pl0800_enable_state(false);
				change_pl0300_enable_state(true);
				
				//Create camera hook only if pl0800 is current manual player
				if(*(uintptr_t*)(((uintptr_t)pl0300->get_pl_manager()) + 0x60) == _pl0800)
				{
					_pl0800LastPlAccessor = *(uintptr_t*)(camCntr + 0x4D0);
					_plCamCntrlSetPlAccMethod->call(threadCntxt, camCntr, _pl0300Accessor);
					_isCameraSetSkipRequested = true;
				}
				_isPl0300Active = true;
			}
			else
			{
				*(uintptr_t*)(pl0300->get_pl() + 0x428) = 0;
				*(bool*)(_pl0800 + 0x4C6) = true;
				auto pl0300PadInput = *(uintptr_t*)(pl0300->get_pl() + 0xEF0);
				if (pl0300PadInput != 0)
				{
					*(bool*)(pl0300PadInput + 0x24) = false;
					*(bool*)(pl0300PadInput + 0x28) = false;
				}

				if (callPl0800EndCutscene && _pl0800EndCutSceneMethod != nullptr)
					_pl0800EndCutSceneMethod->call(sdk::get_thread_context(), _pl0800, 0, 0, 0, 0);

				gf::Transform_SetPosition::set_character_pos(_pl0800, pl0300->get_pl_pos());
				*get_char_rot(_pl0800) = *get_char_rot(pl0300->get_pl());
				if (pl0300->get_cur_dt() == PlCntr::DT::SDT)
					pl0300->set_dt(PlCntr::DT::Human);
				pl0300->set_action(L"Wait");
				pl0300->get_pl_reset_status_method()->call(threadCntxt, pl0300->get_pl());
				pl0300->end_cutscene();
				change_pl0300_enable_state(false);
				change_pl0800_enable_state(true);
				_isCameraSetSkipRequested = false;
				_plCamCntrlSetPlAccMethod->call(threadCntxt, camCntr, _pl0800LastPlAccessor);
				_pl0800LastLockOnTargetWork = 0;
				_pl0800LastPlAccessor = 0;
				_isPl0300Active = false;
			}
			_lastPlSwap = LastPlSwap::ViaCamSwap;
			_networkBBUpdateMethod->call(threadCntxt, pl0300->get_pl());
			_networkBBUpdateMethod->call(threadCntxt, _pl0800);
		}

		uintptr_t change_manual_pl(int plId, bool callPl0800EndCutscene = true)
		{
			auto plManager = (uintptr_t)sdk::get_managed_singleton<REManagedObject>("app.PlayerManager");
			const auto pl0300 = _pl0300.lock();
			if (pl0300 == nullptr || plManager == 0)
				return 0;
			if (pl0300->get_cur_mediation_type() != 2)
			{
				pl0300->set_network_base_active(true);
				pl0300->get_network_base_bhvr_update_method()->call(sdk::get_thread_context(), pl0300->get_pl());
			}
			int* curPl = get_cur_pl_id((uintptr_t)(pl0300->get_mission_setting_manager()));
			if (curPl == nullptr)
				return 0;
			auto pl = plId == 4 ? _pl0800 : pl0300->get_pl();
			*curPl = plId;

			if (plId == 4)
			{
				*(bool*)(_pl0800 + 0x4C6) = true;
				auto pl0300PadInput = *(uintptr_t*)(pl0300->get_pl() + 0xEF0);
				if (pl0300PadInput != 0)
				{
					*(bool*)(pl0300PadInput + 0x24) = false;
					*(bool*)(pl0300PadInput + 0x28) = false;
				}
				auto pl0800PadInput = *(uintptr_t*)(_pl0800 + 0xEF0);
				if (pl0800PadInput != 0)
					*(bool*)(pl0800PadInput + 0x24) = true;
				
				if (callPl0800EndCutscene && _pl0800EndCutSceneMethod != nullptr)
					_pl0800EndCutSceneMethod->call(sdk::get_thread_context(), _pl0800, 0, 0, 0, 0);

				gf::Transform_SetPosition::set_character_pos(pl, pl0300->get_pl_pos());
				*get_char_rot(pl) = *get_char_rot(pl0300->get_pl());
				if (pl0300->get_cur_dt() == PlCntr::DT::SDT)
					pl0300->set_dt(PlCntr::DT::Human);
				pl0300->end_cutscene();
				pl0300->get_pl_reset_status_method()->call(sdk::get_thread_context(), pl0300->get_pl());
				change_pl0300_enable_state(false);
				change_pl0800_enable_state(true);
				_pl0800LastLockOnTargetWork = 0;
				_isPl0300Active = false;
				_isCameraSetSkipRequested = false;
			}
			else
			{
				*(uintptr_t*)(pl0300->get_pl() + 0x428) = 0;
				pl0300->set_pos_full(*get_char_pos(_pl0800));
				*get_char_rot(pl0300->get_pl()) = *get_char_rot(pl);
				*(bool*)(_pl0800 + 0x4C6) = false;

				set_pl0800_lock_on_to_pl0300(pl0300, (uintptr_t)sdk::get_thread_context());
				pl0300->set_hp(20000.0f);
				change_pl0800_enable_state(false);
				change_pl0300_enable_state(true);
				*get_char_rot(pl0300->get_pl()) = *get_char_rot(pl);
				
				if (_pl0800ResetStatusMethod != nullptr)
				{
					_pl0800ResetStatusMethod->call(sdk::get_thread_context(), _pl0800, 0);
					_networkBBUpdateMethod->call(sdk::get_thread_context(), _pl0800);
				}
				_isPl0300Active = true;
			}
			*(uintptr_t*)(plManager + 0x60) = pl;
			_networkBBUpdateMethod->call(sdk::get_thread_context(), pl0300->get_pl());
			_networkBBUpdateMethod->call(sdk::get_thread_context(), _pl0800);
			_lastPlSwap = LastPlSwap::ViaPlId;
			return pl;
		}

		inline bool is_pl0300_active() const noexcept { return _isPl0300Active; }
	};

	std::vector<std::unique_ptr<PlPair>> _vergilsList;

	static inline std::unique_ptr<gf::SysString> _waitStr = nullptr;

	std::mutex _onPfbInitMtx;

	static inline const float _doppelHp = 20000.0F;
	float _doppelAttackRate = 0.35f;
	float _doppelLifeTime = 850.0f;
	float _airRaidHeightOfArenaSideAutoOffs = 1.5f;
	float _airRaidHeightOnOutsideAutoOffs = 8.0f;
	float _airRaidDistanceCheckGroundAutoOffs = 5.0f;
	static inline float _hitStopRate;
	static inline float _hitStopTime;

	bool _isFirstDoppelUpdate = true;

	bool _isPl0300Loaded = false;
	bool _isPl0800CheckCommHookInstalled = false;

	bool _isBossDoppelEnabled = true;
	bool _isDoppelsActive = false;
	bool _isDoppelsAutoDestroy = false;
	bool _useEnhancedDoppelsHCSettings = true;
	bool _isNeedToScaleDoppel = true;
	bool _useInstantDoppelsTeleports = false;

	bool _isAirRaidEnabled = true;
	bool _isAirRaidAutoSetup = true;

	bool _isTrickStabEnabled = true;
	static inline bool _isSetHitStopAfterStab = false;

	bool _isExCostume = false;

	PlCntr::DT _doppelsDtState = PlCntr::DT::Human;

	PlCntr::HitControllerSettings _pl0300doppelsHCS;
	PlCntr::HitControllerSettings _pl0300AirRaidHCS;
	PlCntr::HitControllerSettings _pl0300TrickStabHCS;

	PlCntr::Pl0300Cntr::TeleportTimingParams _defaultGameParams;
	PlCntr::Pl0300Cntr::TeleportTimingParams _instantTeleportParams;

	static inline BossVergilMoves *_mod = nullptr;

	InputSystem* _inputSystem = nullptr;

	VergilQuickSilver* _vergilQSMod = nullptr;

	WitchTime* _wtMod = nullptr;

	static inline PlCntr::Pl0300Cntr::Pl0300ControllerManager* _pl0300Manager = nullptr;

	PlCntr::Pl0300Cntr::AirRaidSettings _airRaidSettings{};

	sdk::REMethodDefinition* _pl0800IsActionExecutableMethod = nullptr;
	sdk::REMethodDefinition* _commandCreateLeverCommandMethod = nullptr;
	sdk::REMethodDefinition* _plMngrRequestRemoveMethod;

	uintptr_t _pl0800CheckCommandAddr = 0;

	const std::array<char*, 3> _delayNames{ "Fast mode", "Default mode", "Slow mode" };
	int _curPlId = 4;

	std::unique_ptr<FunctionHook> _FE_doCommandSpecHook = nullptr;
	std::unique_ptr<FunctionHook> _fuckCumpcomSyncHook = nullptr;
	std::unique_ptr<FunctionHook> _pl0800CheckCommandHook = nullptr;
	std::unique_ptr<FunctionHook> _plSetOrbEfxHook = nullptr;

	VergilDoppelInitSetup::DoppelDelayState _doppelSpeed = VergilDoppelInitSetup::DoppelDelayState::Slow;

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	void reset(EndLvlHooks::EndType end) override
	{
		_isFirstDoppelUpdate = true;
		_isPl0300Loaded = _isDoppelsActive = _isPl0800CheckCommHookInstalled = false;
		_pl0800CheckCommandHook = nullptr;
		_vergilsList.clear();
		VergilDoppelInitSetup::on_doppel_summon_unsub(std::make_shared<Events::EventHandler<BossVergilMoves, uintptr_t, VergilDoppelInitSetup::DoppelDelayState, bool*>>
			(this, &BossVergilMoves::on_doppel_summon));
		if (end == EndLvlHooks::EndType::Bp)
			PlayerTracker::on_pl_die_unsub(std::make_shared<Events::EventHandler<BossVergilMoves, uintptr_t, uintptr_t>>(this, &BossVergilMoves::on_pl_set_die));
	};

	void after_all_inits() override
	{
		_inputSystem = static_cast<InputSystem*>(g_framework->get_mods()->get_mod("InputSystem"));
		_pl0300Manager = static_cast<PlCntr::Pl0300Cntr::Pl0300ControllerManager*>(g_framework->get_mods()->get_mod("Pl0300ControllerManager"));
		_vergilQSMod = static_cast<VergilQuickSilver*>(g_framework->get_mods()->get_mod("VergilQuickSilver"));
		_wtMod = static_cast<WitchTime*>(g_framework->get_mods()->get_mod("WitchTime"));
	}

	static uintptr_t get_pl_camera_controller()
	{
		auto camMngr = (uintptr_t)sdk::get_managed_singleton<REManagedObject*>("app.CameraManager");
		if (camMngr == 0)
			return 0;
		return *(uintptr_t*)(camMngr + 0x98);
	}

	uintptr_t get_em_params(const std::weak_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>& pl0300) const noexcept { return *(uintptr_t*)(pl0300.lock()->get_pl() + 0x1768); }

	//Fuuuuuuuuck this shit got called after any trainig reset
	void on_pfb_manager_inited()
	{
		std::unique_lock<std::mutex> lck(_onPfbInitMtx);
		if (!cheaton || _isPl0300Loaded)
			return;
		VergilDoppelInitSetup::on_doppel_summon_sub(std::make_shared<Events::EventHandler<BossVergilMoves, uintptr_t, VergilDoppelInitSetup::DoppelDelayState, bool*>>
			(this, &BossVergilMoves::on_doppel_summon));
		if (_pl0300Manager != nullptr)
		{				
			if (GameplayStateTracker::gameMode == 3)
				PlayerTracker::on_pl_die_sub(std::make_shared<Events::EventHandler<BossVergilMoves, uintptr_t, uintptr_t>>(this, &BossVergilMoves::on_pl_set_die));
			if (_pl0800CheckCommandHook == nullptr)
			{
				_pl0800CheckCommandHook = std::make_unique<FunctionHook>(_pl0800CheckCommandAddr, &BossVergilMoves::pl0800_check_command_hook, false);
				_isPl0800CheckCommHookInstalled = _pl0800CheckCommandHook->create();
			}
		}
		_pl0300Manager->load_pfb(_isExCostume);
	}

	void on_pl_added(uintptr_t threadCtxt, uintptr_t plManager, uintptr_t player)
	{
		if (!cheaton || player == 0 || *(int*)(player + 0xE64) != 4)
			return;
		auto missionSettingMngr = (uintptr_t)sdk::get_managed_singleton<REManagedObject*>("app.MissionSettingManager");
		auto startPos = *(gf::Vec3*)(missionSettingMngr + 0xB0);
		auto pl0300Weak = _pl0300Manager->create_em6000(PlCntr::Pl0300Cntr::Pl0300Type::PlHelper, startPos, true, _isExCostume);
		auto pl0300 = pl0300Weak.lock();
		if (pl0300 == nullptr)
			return;
		_isPl0300Loaded = true;
		pl0300->set_network_base_active(false);
		pl0300->get_network_base_bhvr_update_method()->call(threadCtxt, pl0300->get_pl());
		_vergilsList.emplace_back(std::make_unique<PlPair>(player, pl0300Weak, _inputSystem, threadCtxt));
		pl0300->update_pl_manager();
		sdk::get_object_method((REManagedObject*)plManager, "addPlayer(app.Player)")->call(threadCtxt, plManager, pl0300->get_pl());
		pl0300->set_is_no_die(true);
		pl0300->set_is_control(false);
	}

	void on_pl_remove(uintptr_t threadCtxt, uintptr_t plManager, uintptr_t pl, bool isUnload)
	{
		if (pl == 0 || *(int*)(pl + 0xE64) != 4 || _vergilsList.empty())
			return;
		for (int i = 0; i < _vergilsList.size(); i++)
		{
			if (_vergilsList[i]->_pl0800 == pl)
			{
				_vergilsList.erase(_vergilsList.begin() + i);
				break;
			}
		}		
	}

	void on_pl_set_die(uintptr_t threadCtxt, uintptr_t pl)
	{
		if (cheaton && _isPl0300Loaded && *(int*)(pl + 0xE64) == 4 && GameplayStateTracker::gameMode == 3)
		{
			auto bpManager = (uintptr_t)sdk::get_managed_singleton<REManagedObject>("app.BloodyPalaceManager");
			if (bpManager != 0)
			{
				*(int*)(bpManager + 0x60) = 42;//curFlowState
				auto padInput = *(uintptr_t*)(pl + 0xEF0);
				if (padInput != 0)
					*(bool*)(padInput + 0x24) = false;//isEnabled
				auto hc = *(uintptr_t*)(pl + 0x1F8);
				auto damageInfo = *(uintptr_t*)(hc + 0x270);
				sdk::find_method_definition("app.Player", "setPlayerDieReaction(app.HitController.DamageInfo)")->call(threadCtxt, (REManagedObject*)pl, (REManagedObject*)damageInfo);
			}
		}
	}

	void on_doppel_summon(uintptr_t pl0800, VergilDoppelInitSetup::DoppelDelayState doppelSpeed, bool* skipSummon)
	{
		if (!cheaton || !_isBossDoppelEnabled || !_isPl0300Loaded)
			return;
		if (doppelSpeed == _doppelSpeed)
		{
			for (auto& i : _vergilsList)
			{
				if (pl0800 == i->_pl0800)
				{
					if (!i->is_doppel_mode() )
					{
						if (*(float*)(pl0800 + 0x1110) >= 3000.0f)
						{
							auto doppelScale = _isNeedToScaleDoppel ? gf::Vec3(1.15f, 1.15f, 1.15f) : gf::Vec3(1.0f, 1.0f, 1.0f);
							auto doppel = i->createDoppel(_isDoppelsAutoDestroy, _doppelLifeTime * 100.0F, doppelScale, false, _doppelHp, _doppelAttackRate).lock();
							if (_isFirstDoppelUpdate)
							{
								_isFirstDoppelUpdate = false;
								_defaultGameParams = doppel->get_teleport_timing_params().value_or(PlCntr::Pl0300Cntr::TeleportTimingParams{});
							}
							if (_useInstantDoppelsTeleports)//this params shared with main Vergil :(
								doppel->set_teleport_timing_params(_instantTeleportParams);
							else
								doppel->set_teleport_timing_params(_defaultGameParams);

							if (_useEnhancedDoppelsHCSettings)
								doppel->set_hitcontroller_settings(_pl0300doppelsHCS);
							*(float*)(pl0800 + 0x1110) -= 3000.0f;
							doppel->set_dt(_doppelsDtState);
						}
					}
					else
					{
						i->destroy_doppel();
					}
				}
			}
			*skipSummon = true;
		}
	}

	static gf::Vec3 *get_char_pos(uintptr_t character) noexcept
	{
		auto gameObj = *(uintptr_t*)(character + 0x10);
		auto transform = *(uintptr_t*)(gameObj + 0x18);
		return (gf::Vec3*)(transform + 0x30);
	}

	static gf::Vec3 get_ground_char_pos(uintptr_t character) { return *(gf::Vec3*)(character + 0x3E0); }

	static gf::Quaternion *get_char_rot(uintptr_t character) noexcept
	{
		auto gameObj = *(uintptr_t*)(character + 0x10);
		auto transform = *(uintptr_t*)(gameObj + 0x18);
		return (gf::Quaternion*)(transform + 0x40);
	}

	static bool force_edge_do_command_spec_ds_hook(uintptr_t threadCtxt, uintptr_t weaponForceEdge, int action)
	{
		const auto vergil = *(uintptr_t*)(weaponForceEdge + 0x310);
		if (!cheaton || !_mod->_isAirRaidEnabled || !_mod->_isPl0300Loaded || *(int*)(vergil + 0x9B0) == 2 || *(float*)(vergil + 0x1B20) < 5000.0f)
			return _mod->_FE_doCommandSpecHook->get_original<decltype(force_edge_do_command_spec_ds_hook)>()(threadCtxt, weaponForceEdge, action);

		gf::PlayerCheckNormalJump checkJump(vergil);
		for (auto& i : _mod->_vergilsList)
		{
			if (i->_pl0800 == vergil)
			{
				if (auto pl0300 = i->_pl0300.lock(); pl0300 != nullptr && checkJump() && _mod->_pl0800IsActionExecutableMethod != nullptr && _mod->_pl0800IsActionExecutableMethod->call(threadCtxt, vergil, 25))
				{
					if (pl0300->get_game_model_get_hp_method() != nullptr && pl0300->get_game_model_set_hp_method() != nullptr)
					{
						*(float*)(vergil + 0x1B50) = 300.0f;//conc gauge 
						*(float*)(vergil + 0x1B20) -= 5000.0f;//SDT gauge
						float pl0800CurHp = pl0300->get_game_model_get_hp_method()->call<float>(threadCtxt, i->_pl0800);
						pl0800CurHp = pl0800CurHp >= 17000.0f ? 20000.0f : pl0800CurHp + 3000.0f;
						pl0300->get_game_model_set_hp_method()->call(threadCtxt, i->_pl0800, pl0800CurHp);
						i->change_pl_via_cam(threadCtxt, 3);
						if (_mod->_isAirRaidAutoSetup)
						{
							auto pl0800GroundPos = get_ground_char_pos(i->_pl0800);
							_mod->_airRaidSettings.heightOfArenaSide = _mod->_airRaidHeightOfArenaSideAutoOffs + pl0800GroundPos.z;
							_mod->_airRaidSettings.heightOnOutside = _mod->_airRaidHeightOnOutsideAutoOffs + pl0800GroundPos.z;
							_mod->_airRaidSettings.distanceCheckGround = _mod->_airRaidDistanceCheckGroundAutoOffs + pl0800GroundPos.z;
						}
						_mod->_vergilQSMod->request_end_quicksilver();
						_mod->_wtMod->request_stop_witchtime();
						i->start_air_raid(*get_char_pos(i->_pl0800), _mod->_airRaidSettings, _mod->_pl0300AirRaidHCS);
						return true;
					}
				}
			}
		}
		return false;
	}

	static bool pl0800_check_command_hook(uintptr_t threadCtxt, uintptr_t pl0800)
	{
		if (!cheaton || !_mod->_isPl0300Loaded || !_mod->_isTrickStabEnabled)
			return _mod->_pl0800CheckCommandHook->get_original<decltype(pl0800_check_command_hook)>()(threadCtxt, pl0800);
		if( *(int*)(pl0800 + 0x1B5C) == 2 && *(int*)(pl0800 + 0x1978) == 0 &&//concentrationLvl & weaponYamato
			_mod->_inputSystem->is_action_button_pressed(*(uintptr_t*)(pl0800 + 0xEF0), InputSystem::PadInputGameAction::AttackS) &&
			*(bool*)(pl0800 + 0xED0) && //isManualLockOn
			_mod->_inputSystem->is_action_button_pressed(*(uintptr_t*)(pl0800 + 0xEF0), InputSystem::PadInputGameAction::Special0) &&
			_mod->_inputSystem->is_back_input(threadCtxt, (REManagedObject*)(*(uintptr_t*)(pl0800 + 0xEF0)), 55.5f)
			&& *(UINT64*)(pl0800 + 0x1348) == 2
			)
		{
			for (auto& i : _mod->_vergilsList)
			{
				if (i->_pl0800 == pl0800 && !i->is_trick_stab_performing())
				{
					if (auto pl0300 = i->_pl0300.lock(); pl0300 != nullptr)
					{
						auto missionSettingsManager = (uintptr_t)pl0300->get_mission_setting_manager();
						auto missionSettings = *(uintptr_t*)(missionSettingsManager + 0xA0);
						if (missionSettings == 0)
							return false;
						auto plInfo = *(uintptr_t*)(missionSettings + 0x10);
						if (*(int*)(plInfo + 0x10) == 3)
							return false;
					}
					if(*(bool*)(pl0800 + 0xECA))
						*(float*)(pl0800 + 0x1B50) -= 85.0f;
					_mod->_vergilQSMod->request_end_quicksilver();
					_mod->_wtMod->request_stop_witchtime();
					//i->_pl0300.lock()->get_pl_set_action_method()->call(threadCtxt, pl0800, _waitStr->get_net_str(), 0, 0, 0, 0, 0, false, false, true, 0);
					i->change_pl_via_cam(threadCtxt, 3);
					i->start_trick_stab(*get_char_pos(pl0800), _mod->_pl0300TrickStabHCS, _mod->_pl0300TrickType);
					return false;
				}
			}
		}
		return _mod->_pl0800CheckCommandHook->get_original<decltype(pl0800_check_command_hook)>()(threadCtxt, pl0800);
	}

	//finishing capcom's job with 1 check
	static void pl_set_orb_efx_hook(uintptr_t threadCtxt, uintptr_t pl, int orbTypeEnum)
	{
		if (*(int*)(pl + 0xE64) != 3)
			_mod->_plSetOrbEfxHook->get_original<decltype(pl_set_orb_efx_hook)>()(threadCtxt, pl, orbTypeEnum);
	}

	void on_sdk_init() override
	{
		_waitStr = std::make_unique<gf::SysString>(L"Wait");
		_pl0800IsActionExecutableMethod = sdk::find_method_definition("app.PlayerVergilPL", "isActionExecutable(app.PlayerVergilPL.LevelUpAction)");
		_plMngrRequestRemoveMethod = sdk::find_method_definition("app.PlayerManager", "removePlayer(app.Player, System.Boolean)");
	}

public:
	BossVergilMoves()
	{
		_mod = this;
		_pl0300doppelsHCS.baseDmgReactionRate = 0.3F;
		_pl0300doppelsHCS.baseAttackRate = 0.5F;
		_instantTeleportParams.set_all(0);
		_airRaidSettings.attackNum = 1;
		_airRaidSettings.useOptionalTrick = false;
		_pl0300AirRaidHCS.baseDmgReactionRate = _pl0300TrickStabHCS.baseDmgReactionRate = 0;
		_vergilsList.reserve(1);
	}

	~BossVergilMoves()
	{
		GameplayStateTracker::after_pfb_manager_init_unsub(std::make_shared<Events::EventHandler<BossVergilMoves>>(this, &BossVergilMoves::on_pfb_manager_inited));
		PlayerTracker::on_pl_mng_pl_add_unsub(std::make_shared<Events::EventHandler<BossVergilMoves, uintptr_t, uintptr_t, uintptr_t>>(this, &BossVergilMoves::on_pl_added));
		PlayerTracker::on_pl_manager_pl_unload_unsub(std::make_shared<Events::EventHandler<BossVergilMoves, uintptr_t, uintptr_t, uintptr_t, bool>>(this, &BossVergilMoves::on_pl_remove));
	}

	bool is_boss_doppel_enabled() const noexcept { return _isBossDoppelEnabled; }

	inline bool is_air_raid_enabled() const noexcept { return _isAirRaidEnabled; }

	static inline bool cheaton = true;

	static inline uintptr_t _gameSyncMechRet = 0;
	static inline uintptr_t _gameSyncMechFix = 0;
	static inline uintptr_t _fuckCumpcomAddr = 0;

	//Idk dafuck cumpcom did but changing pl0300 dt state crash the game if there more then 10 network active game models (nullptr in rcx, 0x228 offset btw). And yep, it's default game's behaviour.
	//Need to found how to fix this like without just checking to null here, this is bullshit.
	//Same stuff with the boss spawn, i think it's some graphics res alloc func
	static naked void sync_global_detour()
	{
		__asm {
			test rcx, rcx
			je fix

			originalcode:
			mov rcx, [rcx + 0x00000228]
			jmp qword ptr [BossVergilMoves::_gameSyncMechRet]

			fix:
			jmp qword ptr [BossVergilMoves::_gameSyncMechFix]
		}
	}

	std::string_view get_name() const override
	{
		return "BossVergilMoves";
	}
	std::string get_checkbox_name() override
	{
		return m_check_box_name;
	};
	std::string get_hotkey_name() override
	{
		return m_hot_key_name;
	};

	std::optional<std::string> on_initialize() override
	{
		init_check_box_info();
		auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
		m_is_enabled = &cheaton;
		m_on_page = Page_VergilTrick;
		m_full_name_string = "Boss Vergil Moves WIP (+)";
		m_author_string = "V.P. Zadov";
		m_description_string = "Adds some of boss Vergil's moves to playable Vergil. Must be enabled before entering a mission. "
			"if something goes wrong during a boss move, press \"Reset camera\" + \"Change lock on\" to forcefully end the move.\nThis mod is currently in beta, see the issues tab for more details.";

		//set_up_hotkey();

		auto doCommSpecAddr = m_patterns_cache->find_addr(base, "C3 CC CC CC CC CC 48 89 5C 24 10 57 48 83 EC 20 48 8B 41 50 48 8B FA 48 8B 92");
		//DevilMayCry5.app_WeaponForceEdge__doCommandSpecial02222022 (-0x6)
		if (!doCommSpecAddr)
			return "Unable to find BossVergilMoves.doCommSpecAddr.";

		//_pl0800CheckCommandAddr = m_patterns_cache->find_addr(base, "FF 01 CC CC CC CC CC CC CC CC 48 89 5C 24 08").value_or(0);
		_pl0800CheckCommandAddr = m_patterns_cache->find_addr(base, "38 48 83 C4 20 5F C3 CC CC 48 89 5C 24 10 57 48 83 EC 20 48 8B 41 50 48 8B FA").value_or(0);
		//DevilMayCry5.app_PlayerVergilPL__checkCommand113989 (-0xA)
		//DevilMayCry5.app_PlayerVergilPL__updateCommandType114150 (-0x9)
		if (_pl0800CheckCommandAddr == 0)
			return "Unable to find BossVergilMoves._pl0800CheckCommandAddr.";
		else
			_pl0800CheckCommandAddr += 0x9;//0xA;

		_fuckCumpcomAddr = m_patterns_cache->find_addr(base, "48 8B 89 28 02 00 00 48 89").value_or(0);//DevilMayCry5.exe+2AE4540
		if (_fuckCumpcomAddr == 0)
			return "Unable to find BossVergilMoves._fuckCumpcomAddr.";

		auto plSetOrbEfxAddr = m_patterns_cache->find_addr(base, "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 41 50 48 8B FA 49");
		//DevilMayCry5.app_Player__setOrbEffect171251
		if (!plSetOrbEfxAddr)
			return "Unable to find BossVergilMoves.plSetOrbEfxAddr.";

		_gameSyncMechFix = _fuckCumpcomAddr + 0xC;

		GameplayStateTracker::after_pfb_manager_init_sub(std::make_shared<Events::EventHandler<BossVergilMoves>>(this, &BossVergilMoves::on_pfb_manager_inited));
		PlayerTracker::on_pl_mng_pl_add_sub(std::make_shared<Events::EventHandler<BossVergilMoves, uintptr_t, uintptr_t, uintptr_t>>(this, &BossVergilMoves::on_pl_added));
		PlayerTracker::on_pl_manager_pl_unload_sub(std::make_shared<Events::EventHandler<BossVergilMoves, uintptr_t, uintptr_t, uintptr_t, bool>>(this, &BossVergilMoves::on_pl_remove));

		_FE_doCommandSpecHook = std::make_unique<FunctionHook>(doCommSpecAddr.value() + 0x6, &BossVergilMoves::force_edge_do_command_spec_ds_hook);
		if (!_FE_doCommandSpecHook->create())
			return "Unable to create BossVergilMoves._FE_doCommandSpecHook.";

		if (!install_hook_absolute(_fuckCumpcomAddr, _fuckCumpcomSyncHook, &sync_global_detour, &_gameSyncMechRet, 0x7))
			return "Cant create BossVergilMoves._fuckCumpcomSyncHook.";

		_plSetOrbEfxHook = std::make_unique<FunctionHook>(plSetOrbEfxAddr.value(), &BossVergilMoves::pl_set_orb_efx_hook);
		if (!_plSetOrbEfxHook->create())
			return "Cant create BossVergilMoves._plSetOrbEfxHook (snow fix).";

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override 
	{
		_isBossDoppelEnabled = cfg.get<bool>("BossVergilMoves._isBossDoppelEnabled").value_or(true);
		_isDoppelsAutoDestroy = cfg.get<bool>("BossVergilMoves._isDoppelsAutoDestroy").value_or(true);
		_useInstantDoppelsTeleports = cfg.get<bool>("BossVergilMoves._useInstantDoppelsTeleports").value_or(false);
		_pl0300doppelsHCS.isAttackNoDie = cfg.get<bool>("BossVergilMoves._pl0300doppelsHCS.isAttackNoDie").value_or(true);
		_isAirRaidAutoSetup = cfg.get<bool>("BossVergilMoves._isAirRaidAutoSetup").value_or(true);
		_isAirRaidEnabled = cfg.get<bool>("BossVergilMoves._isAirRaidEnabled").value_or(true);
		_airRaidSettings.useOptionalTrick = cfg.get<bool>("BossVergilMoves._airRaidSettings.useOptionalTrick").value_or(true);
		_airRaidSettings.useSummonedSwords = cfg.get<bool>("BossVergilMoves._airRaidSettings.useSummonedSwords").value_or(false);
		_pl0300AirRaidHCS.isAttackNoDie = cfg.get<bool>("BossVergilMoves._pl0300AirRaidHCS.isAttackNoDie").value_or(false);
		_isTrickStabEnabled = cfg.get<bool>("BossVergilMoves._isTrickStabEnebled").value_or(true);
		_pl0300TrickStabHCS.isAttackNoDie = cfg.get<bool>("BossVergilMoves._pl0300TrickStabHCS.isAttackNoDie").value_or(false);
		_isSetHitStopAfterStab = cfg.get<bool>("BossVergilMoves._isSetHitStopAfterStab").value_or(false);
		_isExCostume = cfg.get<bool>("BossVergilMoves._isExCostume").value_or(false);

		_doppelLifeTime = cfg.get<float>("BossVergilMoves._doppelLifeTime").value_or(850.0F);
		_doppelAttackRate = cfg.get<float>("BossVergilMoves._doppelAttackRate").value_or(0.5F);
		_pl0300doppelsHCS.baseAttackRate = cfg.get<float>("BossVergilMoves._pl0300doppelsHCS.baseAttackRate").value_or(0.5F);
		_pl0300doppelsHCS.baseDmgReactionRate = 0.20F;
		_airRaidSettings.distanceCheckGround = cfg.get<float>("BossVergilMoves._airRaidSettings.distanceCheckGround").value_or(5.0F);
		_airRaidSettings.radiusOfArea = cfg.get<float>("BossVergilMoves._airRaidSettings.radiusOfArea").value_or(3.7F);
		_airRaidSettings.radiusOfRevolution = cfg.get<float>("BossVergilMoves._airRaidSettings.radiusOfRevolution").value_or(11.5F);
		_airRaidSettings.heightOfArenaSide = cfg.get<float>("BossVergilMoves._airRaidSettings.heightOfArenaSide").value_or(0.5F);
		_airRaidSettings.heightOnOutside = cfg.get<float>("BossVergilMoves._airRaidSettings.heightOnOutside").value_or(8.0F);
		_airRaidSettings.radiusFinishAttack = cfg.get<float>("BossVergilMoves._airRaidSettings.radiusFinishAttack").value_or(2.5F);
		_airRaidSettings.secConcentrate = cfg.get<float>("BossVergilMoves._airRaidSettings.secConcentrate").value_or(2.0F);
		_pl0300AirRaidHCS.baseAttackRate = cfg.get<float>("BossVergilMoves._pl0300AirRaidHCS.baseAttackRate").value_or(1.0f);
		_pl0300TrickStabHCS.baseAttackRate = cfg.get<float>("BossVergilMoves._pl0300TrickStabHCS.baseAttackRate").value_or(2.5f);
		_airRaidHeightOfArenaSideAutoOffs = cfg.get<float>("BossVergilMoves._airRaidHeightOfArenaSideAutoOffs").value_or(0.5f);
		_airRaidHeightOnOutsideAutoOffs = cfg.get<float>("BossVergilMoves._airRaidHeightOnOutsideAutoOffs").value_or(5.5f);
		_airRaidDistanceCheckGroundAutoOffs = cfg.get<float>("BossVergilMoves._airRaidDistanceCheckGroundAutoOffs").value_or(2.5f);
		_hitStopRate = cfg.get<float>("BossVergilMoves._hitStopRate").value_or(0.1f);
		_hitStopTime = cfg.get<float>("BossVergilMoves._hitStopTime").value_or(150.0f);

		_doppelsDtState = (PlCntr::DT)cfg.get<int>("BossVergilMoves._doppelsDtState").value_or((int)PlCntr::DT::Human);
		_airRaidSettings.attackNum = cfg.get<int>("BossVergilMoves._airRaidSettings.attackNum").value_or(4);
		PlPair::_stabTrickUpdateType = (Pl0300TrickType)cfg.get<int>("BossVergilMoves::PlPair::_stabTrickUpdateType").value_or((int)Pl0300TrickType::ToEnemy);
		PlPair::_stabEmReaction = (StabReaction)cfg.get<int>("BossVergilMoves::PlPair::_stabEmReaction").value_or((int)StabReaction::DamageStandL);
	}
	void on_config_save(utility::Config& cfg) override
	{
		cfg.set<bool>("BossVergilMoves._isBossDoppelEnabled", _isBossDoppelEnabled);
		cfg.set<bool>("BossVergilMoves._isDoppelsAutoDestroy", _isDoppelsAutoDestroy);
		cfg.set<bool>("BossVergilMoves._useInstantDoppelsTeleports", _useInstantDoppelsTeleports);
		cfg.set<bool>("BossVergilMoves._pl0300doppelsHCS.isAttackNoDie", _pl0300doppelsHCS.isAttackNoDie);
		cfg.set<bool>("BossVergilMoves._isAirRaidEnabled", _isAirRaidEnabled);
		cfg.set<bool>("BossVergilMoves._isAirRaidAutoSetup", _isAirRaidAutoSetup);
		cfg.set<bool>("BossVergilMoves._pl0300AirRaidHCS.isAttackNoDie", _pl0300AirRaidHCS.isAttackNoDie);
		cfg.set<bool>("BossVergilMoves._pl0300TrickStabHCS.isAttackNoDie", _pl0300TrickStabHCS.isAttackNoDie);
		cfg.set<bool>("BossVergilMoves._airRaidSettings.useOptionalTrick", _airRaidSettings.useOptionalTrick);
		cfg.set<bool>("BossVergilMoves._airRaidSettings.useSummonedSwords", _airRaidSettings.useSummonedSwords);
		cfg.set<bool>("BossVergilMoves._isTrickStabEnebled", _isTrickStabEnabled);
		cfg.set<bool>("BossVergilMoves._isSetHitStopAfterStab", _isSetHitStopAfterStab);
		cfg.set<bool>("BossVergilMoves._isExCostume", _isExCostume);

		cfg.set<float>("BossVergilMoves._doppelLifeTime", _doppelLifeTime);
		cfg.set<float>("BossVergilMoves._doppelAttackRate", _doppelAttackRate);
		cfg.set<float>("BossVergilMoves._pl0300doppelsHCS.baseAttackRate", _pl0300doppelsHCS.baseAttackRate);
		cfg.set<float>("BossVergilMoves._airRaidSettings.distanceCheckGround", _airRaidSettings.distanceCheckGround);
		cfg.set<float>("BossVergilMoves._airRaidSettings.heightOfArenaSide", _airRaidSettings.heightOfArenaSide);
		cfg.set<float>("BossVergilMoves._airRaidSettings.heightOnOutside", _airRaidSettings.heightOnOutside);
		cfg.set<float>("BossVergilMoves._airRaidSettings.radiusFinishAttack", _airRaidSettings.radiusFinishAttack);
		cfg.set<float>("BossVergilMoves._airRaidSettings.radiusOfRevolution", _airRaidSettings.radiusOfRevolution);
		cfg.set<float>("BossVergilMoves._airRaidSettings.secConcentrate", _airRaidSettings.secConcentrate);
		cfg.set<float>("BossVergilMoves._pl0300AirRaidHCS.baseAttackRate", _pl0300AirRaidHCS.baseAttackRate);
		cfg.set<float>("BossVergilMoves._pl0300TrickStabHCS.baseAttackRate", _pl0300TrickStabHCS.baseAttackRate);
		cfg.set<float>("BossVergilMoves._airRaidHeightOfArenaSideAutoOffs", _airRaidHeightOfArenaSideAutoOffs);
		cfg.set<float>("BossVergilMoves._airRaidHeightOnOutsideAutoOffs", _airRaidHeightOnOutsideAutoOffs);
		cfg.set<float>("BossVergilMoves._airRaidDistanceCheckGroundAutoOffs", _airRaidDistanceCheckGroundAutoOffs);
		cfg.set<float>("BossVergilMoves._hitStopRate", _hitStopRate);
		cfg.set<float>("BossVergilMoves._hitStopTime", _hitStopTime);

		cfg.set<int>("BossVergilMoves._doppelsDtState", (int)_doppelsDtState);
		cfg.set<float>("BossVergilMoves._airRaidSettings.attackNum", _airRaidSettings.attackNum);
		cfg.set<int>("BossVergilMoves::PlPair::_stabTrickUpdateType", (int)PlPair::_stabTrickUpdateType);
		cfg.set<int>("BossVergilMoves::PlPair::_stabEmReaction", (int)PlPair::_stabEmReaction);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override 
	{
		if (ImGui::CollapsingHeader("Issues"))
		{
			ImGui::TextWrapped
			(
				"This mod can can softlock or crash your game."
				//"\"New\" moves can't hit boss himself.\n"
				"Banishing the boss doppelganger or deleting any spawned-in boss Vergil's will break camera tracking during a boss move.\n"
				"The camera will briefly flicker when banishing a doppelganger.\n"
				"Boss moves can select random enemies as target.\n"
				"Air Raid can send you out of bounds.\n"
				"Air Raid can send you severely out of bounds and Vergil will fly off to a better DMC game.\n"
				"The current enemy wave can be forcelly ended if Vergil flies too far away with Air Raid.\n"
				"Sometimes moves will not start.\n"
				"Changing the Devil Trigger state while doing a boss move will crash the game if there are currently more than 10 bosses in an arena. "
				"I \"fixed\" that, but there may be some visual glitches on bosses for a few seconds.\n"
				"Urizen's spikes can knock Vergil out from his concentration state.\n" //more details on this one please.
				"Trick stab doesn't knock the enemy back (but it still deals damage).\n"
				"Trick stab can end early if it collides with the terrain.\n"
				"In bloody palace, vergil's outfit will change when performing a boss move.\n"
				"Air Raid on LDK can break enemy hitboxes until the mission restarts."
				);
		}

		ImGui::Checkbox("Ex costume", &_isExCostume);
		ImGui::ShowHelpMarker("If checked, boss will have an EX color costume, if not - the default one.");

		if(ImGui::CollapsingHeader("Boss's Doppelganger"))
		{
			ImGui::Checkbox("Enable ##1", &_isBossDoppelEnabled);
			bool isSelected = false;
			if (ImGui::BeginCombo("##doppelDelay", _delayNames[_doppelSpeed - 1]))
			{
				for (int i = 1; i <= _delayNames.size(); i++)
				{
					isSelected = (_doppelSpeed == (VergilDoppelInitSetup::DoppelDelayState)i);
					if (ImGui::Selectable(_delayNames[i - 1], isSelected))
						_doppelSpeed = (VergilDoppelInitSetup::DoppelDelayState)i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::ShowHelpMarker("Summon the boss's doppelganger when pressing on the selected delay button. Press again when the boss's doppelganger is active to banish it.");

			ImGui::TextWrapped("Doppelganger attack rate");
			UI::SliderFloat("##DoppelsAttackRate", &_doppelAttackRate, 0, 1.0f, "%.2f", 1.0f, ImGuiSliderFlags_AlwaysClamp);

			ImGui::Separator();

			ImGui::Checkbox("Use \"enhanced\" hit controller settings instead of just the attack rate:", &_useEnhancedDoppelsHCSettings);
			ImGui::TextWrapped("Doppelganger damage multiplier:");
			ImGui::InputFloat("##DoppelsHCAttackRate", &(_pl0300doppelsHCS.baseAttackRate), 0.05f, 0.5f, "%.2f", 1.0f);
			ImGui::Checkbox("Doppelganger can't kill enemy", &_pl0300doppelsHCS.isAttackNoDie);

			ImGui::Separator();

			ImGui::Checkbox("Auto-banish doppelganger", &_isDoppelsAutoDestroy);
			ImGui::TextWrapped("Time until doppelganger will automatically be banished");
			UI::SliderFloat("##DoppelAutoDestroyTime", &_doppelLifeTime, 600.0f, 1200.0f, "%.2f", 1.0F, ImGuiSliderFlags_AlwaysClamp);

			ImGui::Separator();

			ImGui::Checkbox("Scale doppelganger size to Vergil's current size.", &_isNeedToScaleDoppel);

			ImGui::Separator();

			ImGui::TextWrapped("Doppelganger form:");
			ImGui::RadioButton("Human", (int*)&_doppelsDtState, 0); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
			ImGui::RadioButton("SDT", (int*)&_doppelsDtState, 2);

			ImGui::Separator();

			ImGui::Checkbox("Instant doppelganger tricks", &_useInstantDoppelsTeleports);
		}
		ImGui::ShowHelpMarker("Spend 3 DT bars to summon the boss's version of the doppelganger instead of the player's. The doppelganger will fight by itself, and its AI depends on the missions current difficulty.");

		if (ImGui::CollapsingHeader("Air Raid"))
		{
			ImGui::Checkbox("Enable##1", &_isAirRaidEnabled);

			ImGui::Separator();
			ImGui::TextWrapped("Hit controller settings:");
			ImGui::TextWrapped("Damage rate for Air Raid:");
			UI::SliderFloat("##_pl0300AirRaidHCS.baseAttackRate", &_pl0300AirRaidHCS.baseAttackRate, 0.1, 3.5f, "%.2f", 1.0f);
			ImGui::Checkbox("Air Raid can't kill enemy:", &_pl0300AirRaidHCS.isAttackNoDie);
			ImGui::Separator();

			ImGui::TextWrapped("Air Raid settings:");
			ImGui::Spacing();

			ImGui::Separator();
			ImGui::TextWrapped("Arena detection settings:");

			ImGui::Checkbox("Automatically determine the size and shape of the arena. Used to determine where the Air Raid should travel.", &_isAirRaidAutoSetup);
			if (_isAirRaidAutoSetup)
			{
				ImGui::Spacing();
				ImGui::TextWrapped("Offset for inside height:");
				ImGui::InputFloat("##_airRaidHeightOfArenaSideAutoOffs", &_airRaidHeightOfArenaSideAutoOffs, 1.0f, 2.0f, "%.2f");

				ImGui::TextWrapped("Offset for outside height:");
				ImGui::InputFloat("##_airRaidHeightOnOutsideAutoOffs", &_airRaidHeightOnOutsideAutoOffs, 1.0f, 2.0f, "%.2f");

				ImGui::TextWrapped("Offset for ground check:");
				ImGui::InputFloat("##_airRaidDistanceCheckGroundAutoOffs", &_airRaidDistanceCheckGroundAutoOffs, 1.0f, 2.0f, "%.2f");
			}
			ImGui::Spacing();

			ImGui::TextWrapped("Height when flying outside of the area (default - 8):");
			ImGui::InputFloat("##_airRaidSettings.heightOnOutside", &_airRaidSettings.heightOnOutside, 2.0f, 20.0f, "%.2f");

			ImGui::TextWrapped("Height when flying inside of the area (default - 1.5):");
			ImGui::InputFloat("##_airRaidSettings.heightOfArenaSide", &_airRaidSettings.heightOfArenaSide, 2.0f, 20.0f, "%.2f");

			ImGui::TextWrapped("Ground checking distance (default - 5):");
			ImGui::InputFloat("##_airRaidSettings.distanceCheckGround", &_airRaidSettings.distanceCheckGround, 8.0f, 25.0f, "%.2f");

			ImGui::Separator();

			ImGui::TextWrapped("Radius of revolution (default - 80):"); //what is radius of revolution?
			UI::SliderFloat("##_airRaidSettings.radiusOfRevolution", &_airRaidSettings.radiusOfRevolution, 2.0f, 100.0f, "%.2f");

			ImGui::TextWrapped("Radius of the area (default - 43):");
			ImGui::InputFloat("##_airRaidSettings.radiusOfArea", &_airRaidSettings.radiusOfArea, 2.0f, 43.0f, "%.2f");

			ImGui::TextWrapped("Radius of the finish attack (default - 5):"); //what does radius of finish attack mean?
			UI::SliderFloat("##_airRaidSettings.radiusFinishAttack", &_airRaidSettings.radiusFinishAttack, 0.5f, 5.0f, "%.2f", 1.0F, ImGuiSliderFlags_AlwaysClamp);

			ImGui::TextWrapped("Startup time:");
			UI::SliderFloat("##_airRaidSettings.secConcentrate", &_airRaidSettings.secConcentrate, 0.5f, 3.5f, "%.1f", 1.0F, ImGuiSliderFlags_AlwaysClamp);

			ImGui::TextWrapped("Number of dive-bomb passes:");
			ImGui::ShowHelpMarker("Other settings may affect this in-game.");
			UI::SliderInt("##_airRaidSettings.attackNum", &_airRaidSettings.attackNum, 1, 10);

			ImGui::Checkbox("Spawn summon swords on enemies during air raid", &_airRaidSettings.useSummonedSwords);
			ImGui::Checkbox("Perform trick on final dive-bomb", &_airRaidSettings.useOptionalTrick);

			ImGui::Spacing();
		}
		ImGui::ShowHelpMarker("Instead of performing Deep Stinger in human form, Vergil slightly heals, restores full concentration and performs Air Raid. He invulnerable in this state, "
			"except for Dante's royal release or judgement attacks. Requires half of the SDT gauge.");

		if (ImGui::CollapsingHeader("Trick Stab"))
		{
			ImGui::Checkbox("Enable##2", &_isTrickStabEnabled);
			ImGui::Separator();

			ImGui::TextWrapped("Hit controller settings:");
			ImGui::TextWrapped("Damage rate for Trick Stab:");
			UI::SliderFloat("##_pl0300TrickStabHCS.baseAttackRate", &_pl0300TrickStabHCS.baseAttackRate, 0.1f, 5.5f, "%.2f", 1.0f);
			ImGui::Checkbox("Trick Stab can't kill enemy:", &_pl0300TrickStabHCS.isAttackNoDie);
			ImGui::Separator();

			ImGui::TextWrapped("Stab settings ");

			ImGui::Spacing();

			ImGui::TextWrapped("Stab teleport");
			ImGui::RadioButton("To enemy", (int*)&PlPair::_stabTrickUpdateType, (int)Pl0300TrickType::ToEnemy); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
			ImGui::RadioButton("In place", (int*)&PlPair::_stabTrickUpdateType, (int)Pl0300TrickType::InPlace);

			ImGui::Spacing();

			ImGui::TextWrapped("Enemy reaction to stab:");
			ImGui::ShowHelpMarker("Different enemy can have different behaviour for same settings.");
			ImGui::RadioButton("Stun", (int*)&PlPair::_stabEmReaction, (int)StabReaction::Stun); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
			ImGui::RadioButton("Dying state", (int*)&PlPair::_stabEmReaction, (int)StabReaction::Dying);
			ImGui::RadioButton("Stand knockback damage", (int*)&PlPair::_stabEmReaction, (int)StabReaction::DamageStandL); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
			ImGui::RadioButton("Slam damage", (int*)&PlPair::_stabEmReaction, (int)StabReaction::SlamDamage);
			ImGui::RadioButton("Down", (int*)&PlPair::_stabEmReaction, (int)StabReaction::DamageDownFaceUp); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
			ImGui::RadioButton("Directional blow damage", (int*)&PlPair::_stabEmReaction, (int)StabReaction::DamageDiagonalBlown);
			ImGui::RadioButton("Fly damage", (int*)&PlPair::_stabEmReaction, (int)StabReaction::DamageFlyL);

			ImGui::Spacing();

			ImGui::Checkbox("Slow down enemy which was hitted by stab", &_isSetHitStopAfterStab);
			ImGui::TextWrapped("Enemy speed rate:");
			UI::SliderFloat("##_hitStopRate", &_hitStopRate, 0, 0.9f, "%.3f", 1.0, ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextWrapped("Duration:");
			ImGui::InputFloat("##_hitStopTime", &_hitStopTime, 100.0f, 500.0f, "%.1f");
		}
		ImGui::ShowHelpMarker("When Yamato selected, press LockOn + Back + Trick + Attack with level 2 concentration to perform a trick stab. Consumes concentration on use.");

		if (ImGui::CollapsingHeader("Load info:"))
		{
			ImGui::TextWrapped("Is pl0300 loaded: %d", _isPl0300Loaded);
			ImGui::TextWrapped("Is pl0800 check command hook installed: %d", _isPl0800CheckCommHookInstalled);
		}
		ImGui::ShowHelpMarker("If any of these values are false, the mod will not work.");

		ImGui::Spacing();
		/*UI::SliderInt("_PlId", &_curPlId, 3, 4, "%d", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::Button("Change manual pl"))
		{
			_vergilsList[0].change_manual_pl(_curPlId, false);
			if (_curPlId == 3)
				_vergilsList[0]._pl0300.lock()->set_is_control(true);
		}

		ImGui::Spacing();*/

		/*if (ImGui::Button("AirRaid"))
		{
			_vergilsList[0].start_air_raid(*get_char_pos(_vergilsList[0]._pl0800), _airRaidSettings, _pl0300AirRaidHCS, false);
		}

		ImGui::Spacing();*/

		if (ImGui::Button("Force end boss's moves."))
		{
			for (auto& i : _vergilsList)
				i->force_end_moves();
		}
		ImGui::ShowHelpMarker("Press this if something goes wrong during a boss move."
			"You can also do this during gameplay by pressing \"Reset camera\" + \"Change lock on\".");
	}
};
//clang-format on