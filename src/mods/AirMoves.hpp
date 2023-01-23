#pragma once
#include "Mod.hpp"
#include "GameFunctions/GameFunc.hpp"
#include <initializer_list>
#include "PlayerTracker.hpp"

namespace gf = GameFunctions;

class AirMoves : public Mod
{
public:
	enum Moves
	{
		None,
		RapidSlash,
		FEDrive,
		VoidSlash,
		RBStinger,
		DSStinger,
		SPStinger,
		BRKUpdraft,
		BWKick13,
		IceAge,
		CerbBlitz,
		SpardaRT,
		RebellionRT,
		JCE
	};
private:

	class MoveInfo
	{
	private:
		Moves _move;
		const char* _name;
		const char* _inGameName;
	public:
		bool cheatOn;

		MoveInfo() = delete;

		MoveInfo(Moves move, const char *name, const char *inGameName) : _move(move), cheatOn(false), _name(name), _inGameName(inGameName){ }

		Moves get_move() const noexcept { return _move; }

		const char* const get_name() const noexcept { return _name; }

		const char* const get_ingame_name() const noexcept { return _inGameName; }

	};

	class HookedMoves
	{
	private:
		std::vector<MoveInfo> moves;
	public:
		HookedMoves(const std::initializer_list<MoveInfo> &movesList) : moves(movesList) { }

		size_t size() const { return moves.size(); }

		MoveInfo &operator[](int indx) { return moves[indx]; }

		MoveInfo *operator[](Moves move)
		{
			for ( auto& mv : moves)
			{
				if (mv.get_move() == move)
					return &mv;
			}
			return nullptr;
		}
	};

	void init_check_box_info() override;
	std::unique_ptr<FunctionHook> m_rapidslash_air_hook;
	std::unique_ptr<FunctionHook> m_fe_drive_air_hook;
	std::unique_ptr<FunctionHook> m_void_slash_air_hook;
	std::unique_ptr<FunctionHook> m_rb_stinger_air_hook;
	std::unique_ptr<FunctionHook> m_sp_stinger_air_hook;
	std::unique_ptr<FunctionHook> m_ds_stinger_air_hook;
	std::unique_ptr<FunctionHook> m_brk_updraft_air_hook;
	std::unique_ptr<FunctionHook> m_kick_air_hook;
	std::unique_ptr<FunctionHook> m_cerberus_ice_edge_air_hook;
	std::unique_ptr<FunctionHook> m_cerberus_blitz_air_hook;
	std::unique_ptr<FunctionHook> _spRtAirHook;
	std::unique_ptr<FunctionHook> _rbRtAirHook;
	std::unique_ptr<FunctionHook> _jceAirHook;
	std::unique_ptr<FunctionHook> m_check_ground_hit_hook;
	std::unique_ptr<FunctionHook> m_air_dodge_crash_hook;

public:
	
	static inline bool cheaton = false;
	static inline bool isAirProcess = false;
	static inline bool originalCheck = false;

	static inline uintptr_t rapidSlashIsAirRet = 0;
	static inline uintptr_t feDriveIsAirRet = 0;
	static inline uintptr_t voidSlashIsAirRet = 0;
	static inline uintptr_t rbStingerAirRet = 0;
	static inline uintptr_t spStingerAirRet = 0;
	static inline uintptr_t dsStingerAirRet = 0;
	static inline uintptr_t balrogUpdraftAirRet = 0;
	static inline uintptr_t kick13AirRet = 0;
	static inline uintptr_t cerberusIceAgeAirRet = 0;
	static inline uintptr_t cerberusBlitzAirRet = 0;
	static inline uintptr_t _spardaRTAirRet = 0;
	static inline uintptr_t _rbRTAirRet = 0;
	//static inline uintptr_t _jceAirRet = 0;

	static inline uintptr_t checkGroundHitCallRet = 0;
	static inline uintptr_t airTrickDodgeCrashRet = 0;
	static inline uintptr_t airTrickDodgeCrashJne = 0;

	static inline MoveInfo *curMoveHook = nullptr;

	static inline std::unique_ptr<HookedMoves> vergilMoves = std::make_unique<HookedMoves>(std::initializer_list<MoveInfo>({
	{ Moves::RapidSlash, "Rapid slash/Rising Star", "YM_RapidSlash"},
	{ Moves::FEDrive, "Drive##Vergil", "FE_Drive"},
	{ Moves::VoidSlash, "Void slash", "YM_Spaceslash"},
	{ Moves::BWKick13, "Kick 13", "BW_Kick13"}
	//{ Moves::JCE, "JCE", "YM_JudgeMentCutEnd"}//Vergil falling during jce preparation :(
	}));

	static inline std::unique_ptr<HookedMoves> danteMoves = std::make_unique<HookedMoves>(std::initializer_list<MoveInfo>({
	{ Moves::RBStinger, "Rebellion stinger", "RB_Stinger"},
	{ Moves::SPStinger, "Sparda stinger", "SP_Stinger"},
	{ Moves::DSStinger, "DSD stinger", "DS_Stinger" },
	{ Moves::RebellionRT, "Rebellion round trip", "RB_RoundTrip" },
	{ Moves::SpardaRT, "Sparda round trip", "SP_RoundTrip" },
	{ Moves::BRKUpdraft, "Balrog Updraft", ""/*"BRK_SomerSalt"*/},//Empty str to skip ground reset hook
	{ Moves::IceAge, "Cerberus Ice Age", "CBN_IceAge" },
	{ Moves::CerbBlitz, "Cerberus Percussion", "CBT_Blitz" },
	}));

	AirMoves() = default;

	std::string_view get_name() const override
	{
		return "AirMoves";
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

	static bool is_movecheat_enabled_asm(Moves move);
	static void str_cur_action_asm(uintptr_t curPl);
};