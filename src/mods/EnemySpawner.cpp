#include "EnemySpawner.hpp"

void EnemySpawner::after_all_inits()
{
	_pl0300Manager = (Pl0300Controller::Pl0300ControllerManager*)(g_framework->get_mods()->get_mod("Pl0300ControllerManager"));
	_inputSystemMod = static_cast<InputSystem*>(g_framework->get_mods()->get_mod("InputSystem"));
	//GameplayStateTracker::after_pfb_manager_init_sub(std::make_shared<Events::EventHandler<EnemySpawner>>(this, &EnemySpawner::on_pfb_inits));
}

void EnemySpawner::reset(EndLvlHooks::EndType endType)
{
	_em6000FriendlyList.clear();
	_em6000PlHelpersList.clear();
}

std::optional<std::string> EnemySpawner::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = Page_Balance;
	m_full_name_string = "Enemy Spawner (+)";
	m_author_string = "V.P.Zadov";
	m_description_string = "Spawn enemies in any place. Delay can be happened if enemy prefab wasn't preloaded by default mission settings. Sometimes collisions are not working on spawned enemies.";

	_spawnEmCoroutine.set_delay(0.3f);

	return Mod::on_initialize();
}

void EnemySpawner::on_config_load(const utility::Config& cfg)
{
	_hcNextSpawnSettings.baseAttackRate = cfg.get<float>("EnemySpawner._hcNextSpawnSettings.baseAttackRate").value_or(0.5f);
	_hcNextSpawnSettings.isAttackNoDie = cfg.get<bool>("EnemySpawner._hcNextSpawnSettings.isAttackNoDie").value_or(false);
}

void EnemySpawner::on_config_save(utility::Config& cfg)
{
	cfg.set<float>("EnemySpawner._hcNextSpawnSettings.baseAttackRate", _hcNextSpawnSettings.baseAttackRate);
	cfg.set<bool>("EnemySpawner._hcNextSpawnSettings.isAttackNoDie", _hcNextSpawnSettings.isAttackNoDie);
}

gf::Vec3 EnemySpawner::get_pl_pos(const REManagedObject* plManager)
{
	auto manualPl = *(uintptr_t*)((uintptr_t)(plManager)+0x60);
	if (manualPl == 0)
		return gf::Vec3(0, 0, 0);
	auto gameObj = *(uintptr_t*)(manualPl + 0x10);
	auto transform = *(uintptr_t*)(gameObj + 0x18);
	return *(gf::Vec3*)(transform + 0x30);
}

void EnemySpawner::load_and_spawn(int emId, gf::Vec3 pos, int emNum, LoadType loadType)
{
	if (!GameplayStateTracker::isInMission)
	{
		_spawnEmCoroutine.stop();
		return;
	}
	volatile int* loadStep = 0;
	int id = loadType == LoadType::Enemy ? emId : 38;
	auto pfb = update_pfb(id, loadStep);
	if (pfb != 0 && loadStep != nullptr && *loadStep == 0)
	{
		switch (loadType)
		{
			case Enemy:
			{
				for (int i = 0; i < emNum; i++)
					spawn_enemy(id, pos, loadStep);
				break;
			}
			case FriendlyVergil:
			{
				auto em6000 = _pl0300Manager->create_em6000(Pl0300Controller::Pl0300Controller::Pl0300Type::Em6000Friendly, pos, loadStep);
				em6000.lock()->set_hitcontroller_settings(_hcNextSpawnSettings);
				_em6000FriendlyList.push_back(em6000);
				break;
			}
			case BOBVergil:
			{
				_em6000PlHelpersList.emplace_back(_pl0300Manager->create_em6000(Pl0300Controller::Pl0300Controller::Pl0300Type::PlHelper, pos, loadStep));
				auto pl0300 = _em6000PlHelpersList[_em6000PlHelpersList.size() - 1].lock();
				pl0300->set_hitcontroller_settings(_manualEm6000HcSettings);
				pl0300->set_em_step_enabled(_bobEmStep);
				//pl0300->set_network_base_active(true);
				pl0300->set_jcut_num(_bobJcNum);
				pl0300->use_custom_trick_update(true);
				pl0300->set_trick_update_f([](uintptr_t fsmPl0300Teleport, const std::shared_ptr<Pl0300Controller::Pl0300Controller>& pl0300, bool *skipOrig)
					{
						*skipOrig = true;
						auto upl0300 = pl0300->get_pl0300();
						if (_mod->_inputSystemMod != nullptr && _mod->_inputSystemMod->is_action_button_pressed(InputSystem::PadInputGameAction::CameraReset))
						{
							*(gf::Vec3*)(upl0300 + 0x1f40) = _mod->get_pl_pos(pl0300->get_pl_manager()) + _mod->_pl0300TeleportOffsets[_mod->_rndTeleportOffsIndx(_mod->_rndGen)];
							*(gf::Vec3*)(fsmPl0300Teleport + 0x80) = *(gf::Vec3*)(upl0300 + 0x1f40);
						}
						else
						{
							auto ccc = *(uintptr_t*)(upl0300 + 0x1818);
							if (ccc != 0 && *(uintptr_t*)(ccc + 0x58) != 0)
							    *(gf::Vec3*)(upl0300 + 0x1f40) = *(gf::Vec3*)(ccc + 0x60) + _mod->_pl0300TeleportOffsets[_mod->_rndTeleportOffsIndx(_mod->_rndGen)];
							else
							    *(gf::Vec3*)(upl0300 + 0x1f40) = _mod->get_pl_pos(pl0300->get_pl_manager()) + _mod->_pl0300TeleportOffsets[_mod->_rndTeleportOffsIndx(_mod->_rndGen)];
						}
						*(gf::Vec3*)(fsmPl0300Teleport + 0x80) = *(gf::Vec3*)(upl0300 + 0x1f40);
						*(int*)(upl0300 + 0x2008) = 29;//teleportActionThink, teleport2NearTarget
					});
				break;
			}
			default:
			{
				_spawnEmCoroutine.stop();
				break;
			}
		}
		_spawnEmCoroutine.stop();
	}
}

uintptr_t EnemySpawner::update_pfb(int emId, volatile int*& outLoadStep, unsigned int requestLoadCountAdd)
{
	uintptr_t pfb = 0;
	outLoadStep = 0;
	uintptr_t emManager = (uintptr_t)sdk::get_managed_singleton<REManagedObject>("app.EnemyManager");
	if (emManager == 0)
		return 0;
	auto pfbManager = *(uintptr_t*)((uintptr_t)emManager + 0x58);
	auto emPfbInfoList = *(uintptr_t*)((uintptr_t)pfbManager + 0x10);
	auto count = gf::ListController::get_list_count(emPfbInfoList);
	uintptr_t pfbInfo = 0;
	for (int i = 0; i < count; i++)
	{
		pfbInfo = gf::ListController::get_item<uintptr_t>(emPfbInfoList, i);
		auto pfbDataInfo = *(uintptr_t*)(pfbInfo + 0x10);
		int curId = *(int*)(pfbDataInfo + 0x10);
		if (curId == emId)
		{
			outLoadStep = (volatile int*)(pfbInfo + 0x40);
			if (*(uintptr_t*)(pfbInfo + 0x18) == 0 && ( * outLoadStep != 0 || *outLoadStep != 2))
			{
				//*(uintptr_t*)(pfbInfo + 0x18) = *(uintptr_t*)(pfbInfo + 0x20);
				if (requestLoadCountAdd != 0)
					*(int*)(pfbInfo + 0x30) += requestLoadCountAdd;
				//*outLoadStep = 1;//LoadStep 1 - loading;
				sdk::call_object_func_easy<void*>((REManagedObject*)pfbInfo, "requestLoad(System.Boolean)", false);
				sdk::call_object_func_easy<void*>((REManagedObject*)pfbInfo, "update()");
			}
			pfb = *(uintptr_t*)(pfbInfo + 0x18);
			return pfb;
		}
	}
	return pfb;
}

uintptr_t EnemySpawner::spawn_enemy(int emId, gf::Vec3 pos, volatile int*& outLoadStep, unsigned int requestLoadCountAdd)
{
	uintptr_t res;
	auto pfb = update_pfb(emId, outLoadStep, requestLoadCountAdd);
	if (pfb == 0)
		return 0;
	return (uintptr_t)sdk::call_object_func_easy<void*>((REManagedObject*)pfb, "instantiate(via.vec3)", pos);
}

void EnemySpawner::on_draw_ui()
{
	ImGui::TextWrapped("Select enemy:");
	ImGui::Combo("##emCombo", &selectedIndx, _emNames->data(), _emNames->size(), 25);
	ImGui::TextWrapped("Enemy num: ");
	UI::SliderInt("##EmNum", &emNum, 1, 20, "%d", 1.0F, ImGuiSliderFlags_AlwaysClamp);
	ImGui::TextWrapped("Spawn position:");
	ImGui::InputFloat3("##_spawnPos", (float*)&_spawnPos, "%.1f");
	if (GameplayStateTracker::nowFlow == 22 && ImGui::Button("Get player's position"))
		_spawnPos = CheckpointPos::get_player_coords();
	ImGui::Spacing();
	if (GameplayStateTracker::nowFlow == 22 && ImGui::Button("Try to spawn enemy"))
	{
		_spawnEmCoroutine.start(this, indx_to_id(selectedIndx), _spawnPos, emNum, Enemy);
	}
	ImGui::Spacing();
	if (GameplayStateTracker::nowFlow == 22 && ImGui::Button("Kill all enemies in current wave"))
	{
		auto emManager = sdk::get_managed_singleton<REManagedObject>("app.EnemyManager");
		if (emManager != nullptr)
			sdk::call_object_func_easy<void*>(emManager, "killAllEnemy()");
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Friendly Vergils settings"))
	{
		ImGui::TextWrapped("Damage multiplier:");
		UI::SliderFloat("##Em6000AttackRateF", &_hcNextSpawnSettings.baseAttackRate, 0, 1.0f, "%.2f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		ImGui::Checkbox("Attacks can't kill enemy ##0", &_hcNextSpawnSettings.isAttackNoDie);
		ImGui::Separator();

		if (GameplayStateTracker::nowFlow == 22 && ImGui::Button("Try to spawn friendly Vergil"))
		{
			if (_pl0300Manager == nullptr)
				return;
			_spawnEmCoroutine.start(this, indx_to_id(selectedIndx), _spawnPos, emNum, FriendlyVergil);
		}

		if (GameplayStateTracker::nowFlow == 22)
			ImGui::ShowHelpMarker("Try to spawn enemy Vergil with credit AI even when special fix from \"Enemy Fixes\" mod is disabled. This also prevent to add him to global enemy list "
			" and break regular enemy spawn so \"kill all enemies\" button will not remove friendly Vergils. His \"global\" AI depends of current game difficulty.");

		ImGui::Spacing();

		if (GameplayStateTracker::nowFlow == 22 && ImGui::Button("Change all friendly Vergils settings"))
		{
			if (_pl0300Manager == nullptr)
				return;
			for (auto &i : _em6000FriendlyList)
			{
				i.lock()->set_hitcontroller_settings(_hcNextSpawnSettings);
			}
		}

		ImGui::Spacing();

		if (GameplayStateTracker::nowFlow == 22 && ImGui::Button("Set player's position for all friendly Vergils"))
		{
			if (_pl0300Manager == nullptr)
				return;
			_pl0300Manager->set_pos_to_all(CheckpointPos::get_player_coords(), Pl0300Controller::Pl0300Controller::Pl0300Type::Em6000Friendly);
		}

		ImGui::Spacing();

		if (GameplayStateTracker::nowFlow == 22 && ImGui::Button("Kill all friendly Vergils"))
		{
			if (_pl0300Manager == nullptr)
				return;
			_pl0300Manager->kill_all_friendly_em6000();
		}
	}
	if (ImGui::CollapsingHeader("Battle Of Brothers Vergils settings"))
	{
		ImGui::TextWrapped("Damage multiplier:");
		UI::SliderFloat("##Em6000AttackRateB", &_manualEm6000HcSettings.baseAttackRate, 0, 1.0f, "%.2f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		ImGui::TextWrapped("JC num:");
		UI::SliderInt("##Em6000JcNum", &_bobJcNum, 1, 4, "%d", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		ImGui::Checkbox("Attacks can't kill enemy ##1", &_manualEm6000HcSettings.isAttackNoDie);
		ImGui::Checkbox("Enable junk boss's enemy step", &_bobEmStep);
		ImGui::Separator();

		if (GameplayStateTracker::nowFlow == 22 && ImGui::Button("Try to spawn BOB Vergil"))
		{
			if (_pl0300Manager == nullptr)
				return;
			_spawnEmCoroutine.start(this, indx_to_id(selectedIndx), _spawnPos, emNum, BOBVergil);
		}

		ImGui::Spacing();

		if (GameplayStateTracker::nowFlow == 22 && ImGui::Button("Change all BOB Vergils settings"))
		{
			for (const auto i : _em6000PlHelpersList)
			{
				auto pl0300Ctrl = i.lock();
				pl0300Ctrl->set_hitcontroller_settings(_hcNextSpawnSettings);
				pl0300Ctrl->set_jcut_num(_bobJcNum);
				pl0300Ctrl->set_em_step_enabled(_bobEmStep);
			}
		}

		ImGui::Spacing();

		if (GameplayStateTracker::nowFlow == 22 && ImGui::Button("Set player's position for all BOB Vergils"))
		{
			for (const auto i : _em6000PlHelpersList)
				i.lock()->set_pos_full(CheckpointPos::get_player_coords());
		}

		ImGui::Spacing();

		if (GameplayStateTracker::nowFlow == 22 && ImGui::Button("Kill all BOB Vergils"))
		{
			if (_pl0300Manager == nullptr)
				return;
			for (const auto i : _em6000PlHelpersList)
				_pl0300Manager->destroy_game_obj(i);
			_em6000PlHelpersList.clear();
		}

		ImGui::Spacing();
	}
	ImGui::ShowHelpMarker("Control enemy Vergil like in DMC3 M19. Hold reset camera + style button to summon Vegrgil(s) to your side.");
}