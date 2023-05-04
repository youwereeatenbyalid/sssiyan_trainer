#pragma once
#include "Mod.hpp"
#include "mods/GameFunctions/PositionController.hpp"
#include "PlayerTracker.hpp"
#include "InputSystem.hpp"
#include "PlSetActionData.hpp"
#include "Mods.hpp"
#include "GameFunctions/CreateShell.hpp"

class VergilAirTrick : public Mod
{
public:
	static bool cheaton;
	static bool isSpeedUp;
	static bool isCustomOffset;
	static bool isCustomWaitTime;
	static inline bool isTeleport = false;
	static inline bool isDoppelOppositeTeleport = false;
	static inline bool forceGroundTrick = true;
	static inline bool isAutoRotate = true;

	/*static constexpr float defaultInitSpeed = 0.7f;
	static constexpr float defaultFinishRange = 1.2f;
	static constexpr float defaultZReduceAfter = 5.0f;
	static constexpr float defaultMaxXZ = 5.0f;
	static constexpr float defaultSpeedAcc = 0.3f;
	static constexpr float defaultMaxSpeedZ = 1.0f;*/
	static constexpr float modMaxXZ = 100000.0f;
	static constexpr float modMaxSpeedZ = 100000.0f;
	static constexpr float modZReduceAfter = 100000.0f;
	static constexpr float modFinishRange = 1.8f;
	static constexpr float modFinishOffsetX = 1.5f;
    static constexpr float modSpeedAcc         = 0.0f;

	static inline float trickCorrection = 1.8f;
	static float initSpeed;
	static float waitTime;
	static float finishOffsetZ;
	

	static uintptr_t initSpeedRet;
    static uintptr_t speedAccRet;
	static uintptr_t airTrickRet;
    static uintptr_t finishOffsetRet;
	static uintptr_t waitTimeRet;
    static uintptr_t maxSpeedZRet;
	static uintptr_t maxXZRet;
	static uintptr_t routineStartRet;
	static inline uintptr_t trickUpdateStartRet = 0;

	VergilAirTrick() = default;

	std::string_view get_name() const override { return "VergilAirTrick"; }
	std::string get_checkbox_name() override { return m_check_box_name; };
	std::string get_hotkey_name() override { return m_hot_key_name; };

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

	static void change_pos_asm(uintptr_t trickAction);

private:
	enum TeleportType
	{
		Front,
		Behind,
		Dynamic
	};

	enum GroundTrickType
	{
		Default,
		AlwaysGround
	};

	TeleportType trickType = Front;

	GroundTrickType groundTrickType = Default;

	bool _isIgnoringTrickFinishRange = false;
	bool _isDefaultFinishRangeRequested = false;

	float teleportZOffs = -1.3f;
	const float colliderZUp = 0.75f;

	void init_check_box_info() override;
	void pos_teleport(TeleportType type, gf::Vec3 &outVec, GameFunctions::Vec3 pPos, gf::Vec3 trickVec, float trickCorrect, float trickLen);

	static float trick_action_get_range_hook(uintptr_t threadCntx, uintptr_t fsm2TrickAction);
	static void on_push_hit_hook(uintptr_t threadCntx, uintptr_t fsm2TrickAction, uintptr_t gameObj);

	std::shared_ptr<Detour_t> m_airtrick_hook;
    std::shared_ptr<Detour_t> m_initspeed_hook;
	std::shared_ptr<Detour_t> m_waittime_hook;
    std::shared_ptr<Detour_t> m_finish_offset_hook;
    std::shared_ptr<Detour_t> m_maxspeed_z_hook;
    std::shared_ptr<Detour_t> m_max_xz_hook;
    std::shared_ptr<Detour_t> m_speed_acc_hook;
	std::shared_ptr<Detour_t> m_teleport_hook;
	std::shared_ptr<Detour_t> m_finish_range_hook;
	std::shared_ptr<Detour_t> m_on_push_hit_hook;

	InputSystem *_inputSystem = nullptr;
	static inline VergilAirTrick *_mod;
};
