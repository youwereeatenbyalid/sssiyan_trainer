#pragma once
#include "Mod.hpp"
#include "GameFunctions/GameFunc.hpp"
#include "events/Events.hpp"
#include "PlayerTracker.hpp"
#include "EndLvlHooks.hpp"

//clang-format off
namespace gf = GameFunctions;

class PosActionEditor : public Mod, private EndLvlHooks::IEndLvl
{
public:

	class SpeedController
	{
		using sign = signed char;
	public:
		enum Axis : uint32_t
		{
			X = 1,
			Y = 2,
			Z = 4,
		};

		static inline constexpr int _flagsMax = 4;// capcom also had nx, ny, nz 

		inline friend Axis operator |(Axis a, Axis b)
		{
			return static_cast<Axis>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
		}

		inline friend Axis operator &(Axis a, Axis b)
		{
			return static_cast<Axis>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
		}

		class SharedSettings
		{
		private:
			std::string _saveKey = "PosActionEditor.SpeedController.SharedSettings.";
			std::string _speedLabels[3];//x,y,z
			std::string _accelLabels[3];

			const char* _dispalyMoveName;
			std::vector<const wchar_t*> _gameMoveNames;

			const Axis _moveAxis;

			gf::Vec3 _accel;
			gf::Vec3 _initSpeed;
			gf::Vec3 _dir;

			const bool _isAccel = false;
			bool _isEnabled = false;

		public:
			SharedSettings(const char* displayMoveName, std::vector<const wchar_t*> gameMoveNames, bool isAccel, Axis moveAxis, gf::Vec3 direction) : _dispalyMoveName(displayMoveName), 
				_gameMoveNames(std::move(gameMoveNames)), _isAccel(isAccel), _moveAxis(moveAxis), _dir(direction)
			{
				_saveKey += std::string(displayMoveName + std::string("."));

				_speedLabels[0] = "##" + std::string(displayMoveName) + "XSpeed";
				_speedLabels[1] = "##" + std::string(displayMoveName) + "YSpeed";
				_speedLabels[2] = "##" + std::string(displayMoveName) + "ZSpeed";

				_accelLabels[0] = "##" + std::string(displayMoveName) + "XAccel";
				_accelLabels[1] = "##" + std::string(displayMoveName) + "YAccel";
				_accelLabels[2] = "##" + std::string(displayMoveName) + "ZAccel";
			}

			void on_cfg_save(utility::Config& cfg) const
			{
				cfg.set<float>(_saveKey + "_initSpeedX", _initSpeed.x);
				cfg.set<float>(_saveKey + "_initSpeedY", _initSpeed.y);
				cfg.set<float>(_saveKey + "_initSpeedZ", _initSpeed.z);
				cfg.set<float>(_saveKey + "_accelX", _accel.x);
				cfg.set<float>(_saveKey + "_accelY", _accel.y);
				cfg.set<float>(_saveKey + "_accelZ", _accel.z);
				cfg.set<bool>(_saveKey + "_isEnabled", _isEnabled);
			}

			void on_cfg_load(const utility::Config& cfg)
			{
				_initSpeed.x = cfg.get<float>(_saveKey + "_initSpeedX").value_or(0);
				_initSpeed.y = cfg.get<float>(_saveKey + "_initSpeedY").value_or(0);
				_initSpeed.z = cfg.get<float>(_saveKey + "_initSpeedZ").value_or(0);
				_accel.x = cfg.get<float>(_saveKey + "_accelX").value_or(0);
				_accel.y = cfg.get<float>(_saveKey + "_accelY").value_or(0);
				_accel.z = cfg.get<float>(_saveKey + "_accelZ").value_or(0);
				_isEnabled = cfg.get<bool>(_saveKey + "_isEnabled").value_or(false);
			}

			inline void input_float(const char* label, float* v, float step = 0.5F, float step_fast = 1.0F, const char* format = "%.3f", ImGuiInputTextFlags flags = 0)
			{
				ImGui::InputFloat(label, v, step, step_fast, format, flags);
				if (*v < 0)
					*v = 0;
			}

			void print_settings()
			{
				ImGui::Checkbox(_dispalyMoveName, &_isEnabled);
				ImGui::Spacing();
				for (uint32_t i = 1; i <= _flagsMax; i += i)
				{
					if (((Axis)i & _moveAxis) == (Axis)i)
					{
						switch ((Axis)i)
						{
							case X:
							{
								ImGui::TextWrapped("X Speed:");
								input_float(_speedLabels[0].c_str(), &(_initSpeed.x));
								if (_isAccel)
								{
									ImGui::TextWrapped("X negative acceleration:");
									input_float(_accelLabels[0].c_str(), &(_accel.x));
								}
								break;
							}
							case Y:
							{
								ImGui::TextWrapped("Y Speed:");
								input_float(_speedLabels[1].c_str(), &(_initSpeed.y));
								if (_isAccel)
								{
									ImGui::TextWrapped("Y negative acceleration:");
									input_float(_accelLabels[1].c_str(), &(_accel.y));
								}
								break;
							}
							case Z:
							{
								ImGui::TextWrapped("Z Speed:");
								input_float(_speedLabels[2].c_str(), &(_initSpeed.z));
								if (_isAccel)
								{
									ImGui::TextWrapped("Z negative acceleration:");
									input_float(_accelLabels[2].c_str(), &(_accel.z));
								}
								break;
							}
							default:
								break;
						}
					}
				}
				ImGui::Separator();
			}

			std::vector<const wchar_t*> const* get_game_moves() const noexcept { return &_gameMoveNames; }

			inline bool is_enabled() const noexcept { return _isEnabled; }

			inline bool is_accel() const noexcept { return _isAccel; }

			inline Axis get_move_axis() const noexcept { return _moveAxis; }

			inline gf::Vec3 get_init_speed() const noexcept { return _initSpeed; }

			inline gf::Vec3 get_accel() const noexcept { return _accel; }

			inline gf::Vec3 get_dir()const noexcept { return _dir; }
		};

	private:

		const SharedSettings* _settings;

		gf::Vec3 _prevSpeed;

		uintptr_t _pl;

		bool _isActionStarted = true;

		int _lastMoveStateIndx = -1;

		inline bool check_action()
		{
			auto names = _settings->get_game_moves();
			for (int i = 0; i < names->size(); i++)
			{
				if (gf::StringController::str_cmp(*(uintptr_t*)(_pl + 0x198), (*names)[i]))
				{
					_lastMoveStateIndx = i;
					return true;
				}
			}
			_lastMoveStateIndx = -1;
			return false;
		}

		void on_action_end(uintptr_t threadCntx, uintptr_t fsm2PlayerPlayerAction, uintptr_t behaviourTreeActionArg, bool isNotifyOnly)
		{
			if (!cheaton || !_settings->is_enabled() || *(uintptr_t*)(fsm2PlayerPlayerAction + 0x28) != _pl)
				return;
			int prevIndx = _lastMoveStateIndx;
			bool onAction = check_action();
			if (!onAction)
			{
				_isActionStarted = true;
				return;
			}
			if (onAction && _settings->get_game_moves()->size() > 1)
			{
				if (prevIndx > _lastMoveStateIndx)
				{
					_isActionStarted = true;
					return;
				}
			}
			else//Should i do extra check with prev move state?..
			{
				_isActionStarted = true;
			}
		}

		sign get_sign(float curSpeed)
		{
			if(curSpeed < 0)
				return -1;
			if(curSpeed > 0)
				return 1;
			return 0;
		}

		void update_direction(gf::Vec3& speed)
		{
			auto dir = _settings->get_dir();
			speed.x *= dir.x;
			speed.y *= dir.y;
			speed.z *= dir.z;
		}

		gf::Vec3 copy_speed_axis(gf::Vec3 curSpeed, gf::Vec3 speedFrom)
		{
			for (uint32_t i = 1; i <= _flagsMax; i += i)
			{
				if (((Axis)i & _settings->get_move_axis()) == (Axis)i)
				{
					switch ((Axis)i)
					{
						case X:
						{
							curSpeed.x = speedFrom.x;
							break;
						}
						case Y:
						{
							curSpeed.y = speedFrom.y;
							break;
						}
						case Z:
						{
							curSpeed.z = speedFrom.z;
							break;
						}
						default:
							break;
					}
				}
			}
			return curSpeed;
		}

		void update_speed(gf::Vec3 &curSpeed, uintptr_t pl)
		{
			if (!_settings->is_accel())
			{
				curSpeed = copy_speed_axis(curSpeed, _settings->get_init_speed());
				update_direction(curSpeed);
			}
			else
			{
				if (_isActionStarted)
				{
					_isActionStarted = false;
					curSpeed = copy_speed_axis(curSpeed, _settings->get_init_speed());
					update_direction(curSpeed);
					_prevSpeed = curSpeed;
				}
				else
				{
					auto accel = _settings->get_accel();
					sign xSign = get_sign(_prevSpeed.x);
					sign ySign = get_sign(_prevSpeed.y);
					sign zSign = get_sign(_prevSpeed.z);

					for (uint32_t i = 1; i <= _flagsMax; i += i)
					{
						if (((Axis)i & _settings->get_move_axis()) == (Axis)i)
						{
							switch ((Axis)i)
							{
								case X:
								{
									if (_prevSpeed.x != 0)
									{
										curSpeed.x = abs(_prevSpeed.x) - abs(accel.x);
										if (get_sign(curSpeed.x) <= 0)
											curSpeed.x = _prevSpeed.x = 0;
										else
										{
											curSpeed.x *= xSign;
											_prevSpeed.x = curSpeed.x;
										}
									}
									else
										curSpeed.x = 0;
									break;
								}
								case Y:
								{
									if (_prevSpeed.y != 0)
									{
										curSpeed.y = abs(_prevSpeed.y) - abs(accel.y);
										if (get_sign(curSpeed.y) <= 0)
											curSpeed.y = _prevSpeed.y = 0;
										else
										{
											curSpeed.y *= ySign;
											_prevSpeed.y = curSpeed.y;
										}
									}
									else
										curSpeed.y = 0;
									break;
								}
								case Z:
								{
									if (_prevSpeed.z != 0)
									{
										curSpeed.z = abs(_prevSpeed.z) - abs(accel.z);
										if (get_sign(curSpeed.z) <= 0)
											curSpeed.z = _prevSpeed.z = 0;
										else
										{
											curSpeed.z *= zSign;
											_prevSpeed.z = curSpeed.z;
										}
									}
									else
										curSpeed.z = 0;
									break;
								}
								default:
									break;
								}
						}
					}
				}
			}
		}

	public:
		SpeedController(SharedSettings *sharedSettings, uintptr_t pl) : _pl(pl), _settings(sharedSettings) 
		{
			PlayerTracker::on_fsm2_player_player_action_notify_action_end_sub(std::make_shared<Events::EventHandler<SpeedController, uintptr_t, uintptr_t, uintptr_t, bool>>(this, &SpeedController::on_action_end));
		}

		SpeedController(const SpeedController& other)
		{
			_pl = other._pl;
			_settings = other._settings;
			PlayerTracker::on_fsm2_player_player_action_notify_action_end_sub(std::make_shared<Events::EventHandler<SpeedController, uintptr_t, uintptr_t, uintptr_t, bool>>(this, &SpeedController::on_action_end));
		}

		~SpeedController()
		{
			PlayerTracker::on_fsm2_player_player_action_notify_action_end_unsub(std::make_shared<Events::EventHandler<SpeedController, uintptr_t, uintptr_t, uintptr_t, bool>>(this, &SpeedController::on_action_end));
		}

		bool update_speed(uintptr_t fsmPosCntr)
		{
			if (!_settings->is_enabled() || !check_action())
				return false;
			auto posControlTrack = *(uintptr_t*)(fsmPosCntr + 0xA0);
			if (posControlTrack == 0)
				return false;
			auto pMoveSpeed = (gf::Vec3*)(posControlTrack + 0x20);
			auto pl = *(uintptr_t*)(fsmPosCntr + 0x28);
			if (pl == 0)
				return true;
			update_speed(*pMoveSpeed, pl /*isActionEnd*/);
			return true;
		}

		uintptr_t get_pl() const noexcept { return _pl; }
	};

private:

	static inline std::vector<SpeedController::SharedSettings> _settingsListVergil
	{
		{"Trick Dodge Forward", std::vector{L"TrickAction.TrickEscape.TrickEscape_Front"}, true, SpeedController::Y, gf::Vec3(0, 1, 0)},
		{"Trick Dodge Back", std::vector{L"TrickAction.TrickEscape.TrickEscape_Back"}, true, SpeedController::Y, gf::Vec3(0, -1, 0)},
		{"Trick Dodge Left", std::vector{L"TrickAction.TrickEscape.TrickEscape_Left"}, true, SpeedController::X, gf::Vec3(1, 0, 0)},
		{"Trick Dodge Right", std::vector{L"TrickAction.TrickEscape.TrickEscape_Right"}, true, SpeedController::X, gf::Vec3(-1, 0, 0)},
		{"Left Side Dodge", std::vector{L"Avoid.AvoidLeft.Trick_L_start", L"Avoid.AvoidLeft.Trick_L_Landing"}, true, SpeedController::X | SpeedController::Y, gf::Vec3(1, 1, 0)},
		{"Right Side Dodge", std::vector{L"Avoid.AvoidRight.Trick_R_Start", L"Avoid.AvoidRight.Trick_R_Landing"}, true, SpeedController::X | SpeedController::Y, gf::Vec3(- 1, -1, 0)},
		{"Trick Down (only on the ground)", std::vector{L"TrickAction.TrickDown.Trick_Down"}, false, SpeedController::Y, gf::Vec3(1, -1, 1)},
		{"Areal Cleave", std::vector{L"Yamato.YM_HelmBreaker.YMT_Divingslash_Start", L"Yamato.YM_HelmBreaker.YMT_Divingslash_Loop"}, true, SpeedController::Y, gf::Vec3(0, 1, 0)},
		{"FE Aerial Stinger", std::vector{L"Forceedge.FE_StingerAerial.FE_Stinger_Start Level2", L"Forceedge.FE_StingerAerial.FE_Stinger_Loop Level2"}, true, SpeedController::Y, gf::Vec3(0, 1, 0)},
		{"FE Aerial Stinger SDT", std::vector{L"Forceedge.FE_StingerAerial.FE_Stinger_Start Level2  Majin", L"Forceedge.FE_StingerAerial.FE_Stinger_Loop Level2  Majin"},
			true, SpeedController::Y, gf::Vec3(0, 1, 0)},
		{"Rapid Slash", std::vector{L"Yamato.YM_RapidSlash.YMT_Rapidslash_Start", L"Yamato.YM_RapidSlash.YMT_Rapidslash_Loop"}, false, SpeedController::Y, gf::Vec3(1, 1, 1)},
		{"Deep Stinger", std::vector{L"Forceedge.FE_Deadly.FE_Stinger_Start_DT", L"Forceedge.FE_Deadly.FE_Stinger_Loop_DT", L"Forceedge.FE_Deadly.FE_Stinger_Finish_DT"}, false, SpeedController::Y, gf::Vec3(0, 1, 0)}
		/*{"Starfall", std::vector{L"Beowulf.BW_StarFall.BW_Starfall", L"Beowulf.BW_StarFall.BW_Starfall2"}, false, SpeedController::Y | SpeedController::Z, gf::Vec3(0, 1, -1)},
		{"Starfall SDT", std::vector{L"Beowulf.BW_StarFall.BW_Starfall - コピー (1)"}, false, SpeedController::Y | SpeedController::Z, gf::Vec3(0, 1, 1)},
		{"Starfall Reflection", std::vector{L"Beowulf.BW_StarFall.BW_Starfall_Reflection_Start", L"Beowulf.BW_StarFall.BW_Starfall_Reflection_End"}, false, SpeedController::Y | SpeedController::Z, gf::Vec3(0, 1, 1)}*/
	};

	static inline std::vector<SpeedController::SharedSettings> _settingsListDante
	{
		{"Rebellion Stinger", std::vector{L"Rebellion.RB_Stinger.Step_Lv2.DT_Start", L"Rebellion.RB_Stinger.Step_Lv2.DT_Loop", L"Rebellion.RB_Stinger.Finish"}, false, SpeedController::Y, gf::Vec3(1, 1, 1)},
		//{"Rebellion stinger DT", std::vector{L"Rebellion.RB_Stinger.DT_Finish"}, false, SpeedController::Y, 1, 1, 1},//no:,)
		{"Sparda Stinger", std::vector{L"Spada.SP_Stinger.Step_Lv2.DT_Start", L"Spada.SP_Stinger.Step_Lv2.DT_Loop", L"Spada.SP_Stinger.Finish"}, false, SpeedController::Y, gf::Vec3(1, 1, 1)},
		//{"Sparda stinger DT", std::vector{L"Spada.SP_Stinger.DT"}, false, SpeedController::Y, 1, 1, 1},//no:,)
		{"DSD Stinger", std::vector{L"DevilSword.DS_Stinger.Step_Lv2.DT_Start", L"DevilSword.DS_Stinger.Step_Lv2.DT_loop", L"DevilSword.DS_Stinger.Finish_Lvl2"}, false, SpeedController::Y, gf::Vec3(1, 1, 1)},
		//{"DSD stinger DT", std::vector{L"DevilSword.DS_Stinger.DT"}, false, SpeedController::Y, 1, 1, 1},//no:,)
		{"Gun Stinger", std::vector{L"Coyote.CA_GunStinger.Start", L"Coyote.CA_GunStinger.Loop", L"Coyote.CA_GunStinger.Finish"}, false, SpeedController::Y, gf::Vec3(1, 1, 1)},
		{"Balrog Heavy Jolt", std::vector{L"Balrog.BRP_FrontHook.Start"/*, L"Balrog.BRP_FrontHook.Hook"*/}, false, SpeedController::Y, gf::Vec3(1, 1, 1)},
		{"Killer Bee", std::vector{L"Balrog.BRK_KillerBee.Start", L"Balrog.BRK_KillerBee.Loop"}, false, SpeedController::Y | SpeedController::Z, gf::Vec3(0, 1, -1)},
	};

	std::vector<SpeedController> _plMovesList;

	std::mutex _mtx;

	void reset(EndLvlHooks::EndType type) override
	{
		_plMovesList.clear();
	}

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	void on_fsm2_pos_cntr_action_update(uintptr_t threadCntxt, uintptr_t fsm2PosCntrAction)
	{
		std::lock_guard<std::mutex> lck(_mtx);
		if (!cheaton)
			return;
		auto pl = *(uintptr_t*)(fsm2PosCntrAction + 0x28);
		if (pl == 0)
			return;
		for (auto& i : _plMovesList)
		{
			if (i.get_pl() == pl)
			{
				if(i.update_speed(fsm2PosCntrAction))
					return;
			}
		}
	}

	void on_pl_added(uintptr_t threadCntx, uintptr_t plManager, uintptr_t pl)
	{
		int plId = *(int*)(pl + 0xE64);
		switch (plId)
		{
			case 1:
			{
				for (int i = 0; i < _settingsListDante.size(); i++)
					_plMovesList.emplace_back(&_settingsListDante[i], pl);
				break;
			}

			case 4:
			{
				for(int i = 0; i < _settingsListVergil.size(); i++)
				{
					_plMovesList.emplace_back(&_settingsListVergil[i], pl);
					_plMovesList.emplace_back(&_settingsListVergil[i], *(uintptr_t*)(pl + 0x18B0));
				}
				break;
			}
			default:
				break;
		}
	}

	void on_pl_remove(uintptr_t threadCntxt, uintptr_t plManager, uintptr_t pl, bool isUnload)
	{
		_plMovesList.erase(std::remove_if(_plMovesList.begin(), _plMovesList.end(), [=](const SpeedController& obj)
			{
				return obj.get_pl() == pl;
			}), _plMovesList.end());
	}

public:
	PosActionEditor()
	{
		PlayerTracker::pl_on_fsm2_pos_cntr_action_update_speed_sub(std::make_shared<Events::EventHandler<PosActionEditor, uintptr_t, uintptr_t>>(this, &PosActionEditor::on_fsm2_pos_cntr_action_update));
		PlayerTracker::on_pl_mng_pl_add_sub(std::make_shared<Events::EventHandler<PosActionEditor, uintptr_t, uintptr_t, uintptr_t>>(this, &PosActionEditor::on_pl_added));
		PlayerTracker::on_pl_manager_pl_unload_sub(std::make_shared<Events::EventHandler<PosActionEditor, uintptr_t, uintptr_t, uintptr_t, bool>>(this, &PosActionEditor::on_pl_remove));
	}

	~PosActionEditor()
	{
		PlayerTracker::pl_on_fsm2_pos_cntr_action_update_speed_unsub(std::make_shared<Events::EventHandler<PosActionEditor, uintptr_t, uintptr_t>>(this, &PosActionEditor::on_fsm2_pos_cntr_action_update));
		PlayerTracker::on_pl_mng_pl_add_unsub(std::make_shared<Events::EventHandler<PosActionEditor, uintptr_t, uintptr_t, uintptr_t>>(this, &PosActionEditor::on_pl_added));
		PlayerTracker::on_pl_manager_pl_unload_unsub(std::make_shared<Events::EventHandler<PosActionEditor, uintptr_t, uintptr_t, uintptr_t, bool>>(this, &PosActionEditor::on_pl_remove));
	}

	static inline bool cheaton = true;

	std::string_view get_name() const override
	{
		return "PosActionEditor";
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
		m_on_page = Page_Mechanics;
		m_full_name_string = "Moves speed editor";
		m_author_string = "V.P.Zadov";
		m_description_string = "Change speed of some actions that changes player's position (like stinger(s), trick dodge (yea now you can make boss's trick dodge in the trainer), ets).";

		set_up_hotkey();

		//movesSpeedDataNero.emplace_back(std::make_unique<MoveSpeedData>("Streak", "StreakLv0", true, MoveSpeedData::Y));//Doesn't work
		//movesSpeedDataNero.emplace_back(std::make_unique<MoveSpeedData>("Ex Streak", "ExStreak", true, MoveSpeedData::Y));//Doesn't work
		//movesSpeedDataNero.emplace_back(std::make_unique<MoveSpeedData>("Calibar", "CalibarLv0", true, MoveSpeedData::Y));//Works but suck
		//movesSpeedDataNero.emplace_back(std::make_unique<MoveSpeedData>("Ex Calibar lvl 1", "ExCalibarLv1", true, MoveSpeedData::Y));//Works but sucks
		//movesSpeedDataNero.emplace_back(std::make_unique<MoveSpeedData>("Ex Calibar lvl 2", "ExCalibarLv2", true, MoveSpeedData::Y));//Works but sucks
		//movesSpeedDataNero.emplace_back(std::make_unique<MoveSpeedData>("Ex Calibar lvl 3", "ExCalibarLv3", true, MoveSpeedData::Y));//Works but sucks

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override
	{
		for (auto& move : _settingsListVergil)
			move.on_cfg_load(cfg);
		for (auto& move : _settingsListDante)
			move.on_cfg_load(cfg);
	}
	void on_config_save(utility::Config& cfg) override
	{
		for (const auto& move : _settingsListVergil)
			move.on_cfg_save(cfg);
		for (const auto& move : _settingsListDante)
			move.on_cfg_save(cfg);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override 
	{
		ImGui::TextWrapped("Vergil");
		for (auto& move : _settingsListVergil)
			move.print_settings();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::TextWrapped("Dante");
		for (auto& move : _settingsListDante)
			move.print_settings();
	}
};
//clang-format on