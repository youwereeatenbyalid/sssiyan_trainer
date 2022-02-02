#pragma once
#include "Mod.hpp"
#include "mods/GameFunctions/PositionController.hpp"
#include "PlayerTracker.hpp"
class VergilAirTrick : public Mod
{
public:
	enum TeleportType
	{
		Front,
		Behind
	};

	static inline TeleportType trickType = Front;

	static bool cheaton;
	static bool isSpeedUp;
	static bool isCustomOffset;
	static bool isCustomWaitTime;
	static inline bool isTeleport = false;
	static inline bool isDoppelOppositeTeleport = false;

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
	static inline float teleportZOffs = -1.3f;

	static uintptr_t initSpeedRet;
    static uintptr_t speedAccRet;
	static uintptr_t finishRangeRet;
	static uintptr_t airTrickRet;
    static uintptr_t finishOffsetRet;
	static uintptr_t waitTimeRet;
    static uintptr_t maxSpeedZRet;
	static uintptr_t maxXZRet;
	static uintptr_t routineStartRet;

	VergilAirTrick() = default;

	std::string_view get_name() const override { return "VergilAirTrick"; }
	std::string get_checkbox_name() override { return m_check_box_name; };
	std::string get_hotkey_name() override { return m_hot_key_name; };

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

	static void change_pos_asm(uintptr_t trickAction);

private:
	void init_check_box_info() override;
	static void xypos_teleport(uintptr_t vergil, TeleportType type, float &x, float &y, GameFunctions::Vec3 pPos, float trickX, float trickY, float trickLen);
	//static inline std::unique_ptr<GameFunctions::Transform_SetPosition> set_pos{nullptr};
	std::unique_ptr<FunctionHook> m_airtrick_hook;
    std::unique_ptr<FunctionHook> m_initspeed_hook;
	std::unique_ptr<FunctionHook> m_waittime_hook;
    std::unique_ptr<FunctionHook> m_finish_offset_hook;
    std::unique_ptr<FunctionHook> m_maxspeed_z_hook;
    std::unique_ptr<FunctionHook> m_max_xz_hook;
    std::unique_ptr<FunctionHook> m_speed_acc_hook;
	std::unique_ptr<FunctionHook> m_teleport_hook;
	std::unique_ptr<FunctionHook> m_finish_range_hook;
};

