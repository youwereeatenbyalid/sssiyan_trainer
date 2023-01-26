#pragma once
#include "Mod.hpp"
#include "Mods.hpp"
#include "GameFunctions/GameFunc.hpp"
#include "GameplayStateTracker.hpp"
#include "CheckpointPos.hpp"
#include "InputSystem.hpp"
#include "EnemyData.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h"
#include <future>
#include "mods/Pl0300ControllerManager.hpp"
#include "mods/Coroutine/Coroutines.hpp"

//clang-format off
namespace gf = GameFunctions;

class EnemySpawner : public Mod, private EndLvlHooks::IEndLvl
{
private:
	enum LoadType
	{
		Enemy,
		FriendlyVergil,
		BOBVergil
	};

	static inline const std::array<const char*, 40> *_emNames = EnemyData::get_em_names();

	PlCntr::Pl0300Cntr::Pl0300ControllerManager* _pl0300Manager;
	InputSystem* _inputSystemMod = nullptr;
	static inline EnemySpawner* _mod = nullptr;

	int selectedIndx = 0;
	int emNum = 1;
	int _bobJcNum = 2;

	bool _bobEmStep = false;
	bool _isPlSpawned = false;

	gf::Vec3 _spawnPos{ 0,0,0 };

	PlCntr::HitControllerSettings _hcNextSpawnSettings;
	PlCntr::HitControllerSettings _manualEm6000HcSettings;

	std::vector<std::weak_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>> _em6000PlHelpersList;
	std::vector<std::weak_ptr<PlCntr::Pl0300Cntr::Pl0300Controller>> _em6000FriendlyList;

	void load_and_spawn(int emId, gf::Vec3 pos, int emNum, LoadType loadType = Enemy);

	using actionType = decltype(&EnemySpawner::load_and_spawn);

	Coroutines::Coroutine<actionType, EnemySpawner*, int, gf::Vec3, int, LoadType> _spawnEmCoroutine{ &EnemySpawner::load_and_spawn };

	std::array<gf::Vec3, 3> _pl0300TeleportOffsets = { gf::Vec3(1.2f, 1.2f, 0), gf::Vec3(1.45f, -1.8f, 0), gf::Vec3(-1.35f, 2.0f, 0) };

	std::mt19937 _rndGen{};
	std::uniform_int_distribution<int> _rndTeleportOffsIndx{ 0, _pl0300TeleportOffsets.size() - 1 };

	void reset(EndLvlHooks::EndType endType) override;

	gf::Vec3 get_pl_pos(const REManagedObject* plManager);

	void on_pl_added(uintptr_t threadCntxt, uintptr_t pl)
	{
		_isPlSpawned = true;
	}

	inline int indx_to_id(int indx)
	{
		if (indx == 39)
			return 55;
		if (indx >= 20)
			return indx + 3;
		return indx;
	}

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

public:
	EnemySpawner()
	{
		_mod = this;
		PlayerTracker::pl_added_event_sub(std::make_shared<Events::EventHandler<EnemySpawner, uintptr_t, uintptr_t>>(this, &EnemySpawner::on_pl_added));

	}

	static inline bool cheaton = true;

	std::string_view get_name() const override
	{
		return "EnemySpawner";
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
	void after_all_inits() override;

	uintptr_t update_pfb(int emId, volatile int*& outLoadStep, unsigned int requestLoadCountAdd = 0);

	uintptr_t spawn_enemy(int emId, gf::Vec3 pos, volatile int*& outLoadStep, unsigned int requestLoadCountAdd = 0);

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override;
};
//clang-format on