#pragma once
#include "Mod.hpp"
#include "GameFunctions/GameFunc.hpp"
#include "events/Events.hpp"
#include "PlSetActionData.hpp"
#include "FsmPlPosCtrActionStartHooks.hpp"

//clang-format off
namespace gf = GameFunctions;

class PosActionEditor : public Mod
{
private:

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	void on_move_speed_changed(std::shared_ptr<ActionStartHooks::PosCtrlSpeedEventArgs> args)
	{
		edit_speed_asm(args->get_pos_ctrl());
	}

public:

	class MoveSpeedData
	{
		using sign = signed char;
	public:
		enum Axis
		{
			X,
			Y,
			Z,
			Any
		};

	protected:
		std::string SAVE_KEY = "PosActionEditor.MoveSpeedData.";
		const char* _dispalyMoveName;
		const char* _gameMoveName;

		const Axis _moveAxis;

		gf::Vec3 _accel;
		gf::Vec3 _initSpeed;
		gf::Vec3 _prevSpeed;
		gf::Vec3 _prevDoppelSpeed;

		bool _isAccel = false;
		bool _isEnabled = false;
		bool _isActionStart = false;
		bool _isDoppelActionStart = false;
		bool _isCustomSignFirst;

		sign _customFirst[3];

	private:

		std::mutex _updateSpeedMtx;
		std::mutex _actionStartMtx;

		Events::Event<const MoveSpeedData*> speedSignChanged;

		sign get_sign(float curSpeed)
		{
			if(curSpeed < 0)
				return -1;
			else if(curSpeed != 0)
				return 1;
			else return 0;
		}

		void on_action_start(const std::array<char, 65>* actionStr, uintptr_t threadCntxt, uintptr_t str, uintptr_t pl)
		{
			std::lock_guard<std::mutex> lck(_actionStartMtx);
			if(pl == 0 || *(int*)(pl + 0xE64) == 3 || *(int*)(pl + 0x108) == 1)
				return;
			else if(*(int*)(pl + 0xE64) == 4 && *(bool*)(pl + 0x17F0))
			{
				_isDoppelActionStart = true;
				_prevDoppelSpeed = gf::Vec3(0, 0, 0);
			}
			else
			{
				_isActionStart = true;
				_prevSpeed = gf::Vec3(0,0,0);
			}
		}

		void set_sign(float& n, sign s)
		{
			if (s == -1)
				n = -abs(n);
			else
				n = abs(n);
		}

		gf::Vec3 correct_sign(gf::Vec3 init, gf::Vec3 signFrom)
		{
			gf::Vec3 res = init;
			set_sign(res.x, get_sign(signFrom.x));
			set_sign(res.y, get_sign(signFrom.y));
			set_sign(res.z, get_sign(signFrom.z));
			return res;
		}

		gf::Vec3 copy_speed_axis(gf::Vec3 curSpeed, gf::Vec3 speedFrom)
		{
			gf::Vec3 res = curSpeed;
			switch (_moveAxis)
			{
				case X:
				{
					res.x = speedFrom.x;
					break;
				}
				case Y:
				{
					res.y = speedFrom.y;
					break;
				}
				case Z:
				{
					res.z = speedFrom.z;
					break;
				}
				case Any:
				{
					res = speedFrom;
					break;
				}
			}
			return res;
		}

		bool update_speed(gf::Vec3 &prevSpeed, gf::Vec3 &curSpeed, bool isActStart)
		{
			std::lock_guard lck(_updateSpeedMtx);
			if (!isActStart)
			{
				if (prevSpeed.x != prevSpeed.y != prevSpeed.z != 0)
				{
					sign cX = get_sign(curSpeed.x);
					sign cY = get_sign(curSpeed.y);
					sign cZ = get_sign(curSpeed.z);
					sign pX = get_sign(prevSpeed.x);
					sign pY = get_sign(prevSpeed.y);
					sign pZ = get_sign(prevSpeed.z);
					if (cX != pX || cY != pY || cZ != pZ)
					{
						curSpeed = copy_speed_axis(curSpeed, gf::Vec3(0, 0, 0));
						speedSignChanged.invoke(this);
						return true;
					}
				}
				else
					speedSignChanged.invoke(this);
			}
			
			if (_isAccel)
			{
				gf::Vec3 speedTmp;
				if (isActStart)
				{
					if (!_isCustomSignFirst)
					{
						speedTmp = correct_sign(copy_speed_axis(curSpeed, _initSpeed), curSpeed);
						curSpeed = prevSpeed = speedTmp;
					}
					else
					{
						curSpeed = _initSpeed;
						set_sign(curSpeed.x, _customFirst[0]);
						set_sign(curSpeed.y, _customFirst[1]);
						set_sign(curSpeed.z, _customFirst[2]);
						prevSpeed = curSpeed;
					}
				}
				else
				{
					speedTmp = correct_sign(copy_speed_axis(prevSpeed, _accel), prevSpeed);
					curSpeed = prevSpeed - speedTmp;
					prevSpeed = curSpeed;
				}
			}
			else
			{
				if (isActStart)
				{
					if (!_isCustomSignFirst)
						curSpeed = prevSpeed = correct_sign(copy_speed_axis(curSpeed, _initSpeed), curSpeed);
					else
					{
						curSpeed = _initSpeed;
						set_sign(curSpeed.x, _customFirst[0]);
						set_sign(curSpeed.y, _customFirst[1]);
						set_sign(curSpeed.z, _customFirst[2]);
						prevSpeed = curSpeed;
					}
				}
				else
					curSpeed = prevSpeed = correct_sign(copy_speed_axis(curSpeed, _initSpeed), curSpeed);				
			}
			return true;
		}

	public:

		MoveSpeedData(const char* displayMoveName, const char* gameMoveName, bool isAccel, Axis moveAxis, bool isCustomFirstSign = false, sign signFirstX = 1, sign signFirstY = 1, sign signFirstZ = 1 ) : _dispalyMoveName(displayMoveName),
		_gameMoveName(gameMoveName), _isAccel(isAccel), _moveAxis(moveAxis), _isCustomSignFirst(isCustomFirstSign)
		{
			PlSetActionData::new_action_event_sub(std::make_shared<Events::EventHandler<MoveSpeedData, const std::array<char, PlSetActionData::ACTION_STR_LENGTH>*, uintptr_t, 
				uintptr_t, uintptr_t>>(this, &MoveSpeedData::on_action_start));
			SAVE_KEY += displayMoveName;
			_customFirst[0] = signFirstX;
			_customFirst[1] = signFirstY;
			_customFirst[2] = signFirstZ;
		}

		~MoveSpeedData()
		{
			PlSetActionData::new_action_event_unsub(std::make_shared<Events::EventHandler<MoveSpeedData, const std::array<char, PlSetActionData::ACTION_STR_LENGTH>*, uintptr_t, 
				uintptr_t, uintptr_t>>(this, &MoveSpeedData::on_action_start));
		}

		void on_cfg_save(utility::Config& cfg) const
		{
			cfg.set<float>(SAVE_KEY + "_initSpeedX", _initSpeed.x);
			cfg.set<float>(SAVE_KEY + "_initSpeedY", _initSpeed.y);
			cfg.set<float>(SAVE_KEY + "_initSpeedZ", _initSpeed.z);
			cfg.set<float>(SAVE_KEY + "_accelX", _accel.x);
			cfg.set<float>(SAVE_KEY + "_accelY", _accel.y);
			cfg.set<float>(SAVE_KEY + "_accelZ", _accel.z);
			cfg.set<bool>(SAVE_KEY + "_isEnabled", _isEnabled);
		}

		void on_cfg_load(const utility::Config& cfg)
		{
			_initSpeed.x = cfg.get<float>(SAVE_KEY + "_initSpeedX").value_or(0);
			_initSpeed.y = cfg.get<float>(SAVE_KEY + "_initSpeedY").value_or(0);
			_initSpeed.z = cfg.get<float>(SAVE_KEY + "_initSpeedZ").value_or(0);
			_accel.x = cfg.get<float>(SAVE_KEY + "_accelX").value_or(0);
			_accel.y = cfg.get<float>(SAVE_KEY + "_accelY").value_or(0);
			_accel.z = cfg.get<float>(SAVE_KEY + "_accelZ").value_or(0);
			_isEnabled = cfg.get<bool>(SAVE_KEY + "_isEnabled").value_or(false);
		}

		void print_settings()
		{
			ImGui::Checkbox(_dispalyMoveName, &_isEnabled);
			ImGui::Spacing();
			float *pInitSpeed;
			float *pAccel;
			std::string axisName;
			if (_isEnabled)
			{
				switch (_moveAxis)
				{
					case X:
					{
						pInitSpeed = &_initSpeed.x;
						pAccel = &_accel.x;
						axisName += "(X axis)";
						break;
					}
					case Y:
					{
						pInitSpeed = &_initSpeed.y;
						pAccel = &_accel.y;
						axisName += "(Y axis)";
						break;
					}
					case Z:
					{
						pInitSpeed = &_initSpeed.z;
						pAccel = &_accel.z;
						axisName += "(Z axis)";
						break;
					}
					case Any:
					{
						pInitSpeed = (float*)&_initSpeed;
						pAccel = (float*)&_accel;
						axisName += "(x,z,y)";
						break;
					}
				}

				if (_isAccel)
				{
					ImGui::TextWrapped(("Player's init speed " + axisName + ":").c_str());
					if (_moveAxis != Any)
					{
						UI::SliderFloat((std::string("##_initSpeed ") + _dispalyMoveName).c_str() , pInitSpeed, 0, 20.0f, "%.2f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
						ImGui::TextWrapped("Player's negative acceleration");
						UI::SliderFloat((std::string("##_accel ") + _dispalyMoveName).c_str(), pAccel, 0, 20.0f, "%.2f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
					}
					else
					{
						ImGui::InputFloat3((std::string("##_initSpeed ") + _dispalyMoveName).c_str(), pInitSpeed, "%.2f");
						ImGui::TextWrapped("Player's negative acceleration");
						ImGui::InputFloat3((std::string("##_accel ") + _dispalyMoveName).c_str(), pAccel, "%.2f");
					}
				}
				else
				{
					ImGui::TextWrapped(("Player's speed " + axisName + ":").c_str());
					if (_moveAxis != Any)
						UI::SliderFloat((std::string("##_initSpeed") + _dispalyMoveName).c_str(), pInitSpeed, 0, 20.0f, "%.2f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
					else
						ImGui::InputFloat3((std::string("##_initSpeed") + _dispalyMoveName).c_str(), pInitSpeed, "%.2f");
				}
			}
			ImGui::Separator();
		}

		bool edit_speed(uintptr_t pl, gf::Vec3& speed) 
		{
			if (!_isEnabled || !PlSetActionData::cmp_real_cur_action(_gameMoveName))
				return false;
			bool isDoppel = *(int*)(pl + 0xE64) == 4 && *(bool*)(pl + 0x17F0);
			bool res;
			if (isDoppel)
			{
				res = update_speed(_prevDoppelSpeed, speed, _isDoppelActionStart);
				_isDoppelActionStart = false;
			}
			else
			{
				res = update_speed(_prevSpeed, speed, _isActionStart);
				_isActionStart = false;
			}
			return true;
		}

		const char* const get_display_name() const noexcept { return _dispalyMoveName; }
		const char* const get_game_move_name() const noexcept {return _gameMoveName; }

		template<typename T>
		void speed_sign_changed_sub(std::shared_ptr<Events::EventHandler<T, const PosActionEditor::MoveSpeedData*>> handler)
		{
			speedSignChanged.subscribe(handler);
		}

		template<typename T>
		void speed_sign_changed_unsub(std::shared_ptr<Events::EventHandler<T, const PosActionEditor::MoveSpeedData*>> handler)
		{
			speedSignChanged.unsubscribe(handler);
		}
	};

private:

	static inline std::vector<std::unique_ptr<MoveSpeedData>> movesSpeedDataVergil;
	static inline std::vector<std::unique_ptr<MoveSpeedData>> movesSpeedDataDante;
	static inline std::vector<std::unique_ptr<MoveSpeedData>> movesSpeedDataNero;

public:
	PosActionEditor()
	{
		ActionStartHooks::FsmPlPosCtrActionStartHooks::move_speed_change_sub(std::make_shared<Events::EventHandler<PosActionEditor, std::shared_ptr<ActionStartHooks::PosCtrlSpeedEventArgs>>>(this, &PosActionEditor::on_move_speed_changed));
	}

	~PosActionEditor()
	{
		ActionStartHooks::FsmPlPosCtrActionStartHooks::move_speed_change_unsub(std::make_shared<Events::EventHandler<PosActionEditor, std::shared_ptr<ActionStartHooks::PosCtrlSpeedEventArgs>>>(this, &PosActionEditor::on_move_speed_changed));
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

	template<typename T>
	void speed_sign_changed_sub(std::shared_ptr<Events::EventHandler<T, const PosActionEditor::MoveSpeedData*>> handler)
	{
		for(auto &i : movesSpeedDataVergil)
			i->speed_sign_changed_sub(handler);
		for (auto &i : movesSpeedDataDante)
			i->speed_sign_changed_sub(handler);
		for (auto &i : movesSpeedDataNero)
			i->speed_sign_changed_sub(handler);
	}

	template<typename T>
	void speed_sign_changed_unsub(std::shared_ptr<Events::EventHandler<T, const PosActionEditor::MoveSpeedData*>> handler)
	{
		for (auto &i : movesSpeedDataVergil)
			i->speed_sign_changed_unsub(handler);
		for (auto &i : movesSpeedDataDante)
			i->speed_sign_changed_unsub(handler);
		for (auto &i : movesSpeedDataNero)
			i->speed_sign_changed_unsub(handler);
	}

	static void edit_speed_asm(uintptr_t posControllerAction)
	{
		if(!cheaton || posControllerAction == 0)
			return;
		auto character = *(uintptr_t*)(posControllerAction + 0x28);
		auto posCtrlTrack = *(uintptr_t*)(posControllerAction + 0xA0);
		if (posCtrlTrack == 0 || character == 0 || *(int*)(character + 0xE64) == 3 || *(int*)(character + 0x108) == 1)
			return;
		gf::Vec3* speed = (gf::Vec3*)(posCtrlTrack + 0x20);
		for (auto& moveEdit : movesSpeedDataVergil)
		{
			if(moveEdit->edit_speed(character, *speed))
				return;
		}
		for (auto& moveEdit : movesSpeedDataDante)
		{
			if (moveEdit->edit_speed(character, *speed))
				return;
		}
		for (auto& moveEdit : movesSpeedDataNero)
		{
			if (moveEdit->edit_speed(character, *speed))
				return;
		}
	}

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

		movesSpeedDataVergil.emplace_back(std::make_unique<MoveSpeedData>("Trick Dodge Forward", "TrickEscape_Front", true, MoveSpeedData::Y));
		movesSpeedDataVergil.emplace_back(std::make_unique<MoveSpeedData>("Trick Dodge Back", "TrickEscape_Back", true, MoveSpeedData::Y, true, 1, -1));
		movesSpeedDataVergil.emplace_back(std::make_unique<MoveSpeedData>("Trick Dodge Left", "TrickEscape_Left", true, MoveSpeedData::X));
		movesSpeedDataVergil.emplace_back(std::make_unique<MoveSpeedData>("Trick Dodge Right", "TrickEscape_Right", true, MoveSpeedData::X, true, -1));
		movesSpeedDataVergil.emplace_back(std::make_unique<MoveSpeedData>("Left side dodge", "AvoidLeft", true, MoveSpeedData::Any));
		movesSpeedDataVergil.emplace_back(std::make_unique<MoveSpeedData>("Right side dodge", "AvoidRight", true, MoveSpeedData::Any, true, -1, -1));
		movesSpeedDataVergil.emplace_back(std::make_unique<MoveSpeedData>("Trick down (only on the ground)", "TrickDown", false, MoveSpeedData::Y, true, 1, -1));
		movesSpeedDataVergil.emplace_back(std::make_unique<MoveSpeedData>("Areal Cleave", "YM_HelmBreaker", false, MoveSpeedData::Y));
		movesSpeedDataVergil.emplace_back(std::make_unique<MoveSpeedData>("FE aerial stinger", "FE_StingerAerial", true, MoveSpeedData::Y));
		movesSpeedDataVergil.emplace_back(std::make_unique<MoveSpeedData>("Rapid slash", "YM_RapidSlash", false, MoveSpeedData::Y));
		movesSpeedDataVergil.emplace_back(std::make_unique<MoveSpeedData>("Deep stinger", "FE_Deadly", false, MoveSpeedData::Y));
		//movesSpeedDataVergil.emplace_back(std::make_unique<MoveSpeedData>("FE stinger", "FE_Stinger", true, MoveSpeedData::Y));//No

		movesSpeedDataDante.emplace_back(std::make_unique<MoveSpeedData>("Rebellion stinger", "RB_Stinger", false, MoveSpeedData::Y));
		movesSpeedDataDante.emplace_back(std::make_unique<MoveSpeedData>("Sparda stinger", "SP_Stinger", false, MoveSpeedData::Y));
		movesSpeedDataDante.emplace_back(std::make_unique<MoveSpeedData>("DSD stinger", "DS_Stinger", false, MoveSpeedData::Y));
		movesSpeedDataDante.emplace_back(std::make_unique<MoveSpeedData>("Gun stinger", "CA_GunStinger", false, MoveSpeedData::Y));

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
		for (const auto &move : movesSpeedDataVergil)
			move->on_cfg_load(cfg);
		for (const auto& move : movesSpeedDataDante)
			move->on_cfg_load(cfg);
		for (const auto& move : movesSpeedDataNero)
			move->on_cfg_load(cfg);
	}
	void on_config_save(utility::Config& cfg) override
	{
		for (auto& move : movesSpeedDataVergil)
			move->on_cfg_save(cfg);
		for (auto& move : movesSpeedDataDante)
			move->on_cfg_save(cfg);
		for (auto& move : movesSpeedDataNero)
			move->on_cfg_save(cfg);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override 
	{
		ImGui::TextWrapped("Vergil");
		for (auto& move : movesSpeedDataVergil)
			move->print_settings();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::TextWrapped("Dante");
		for (auto& move : movesSpeedDataDante)
			move->print_settings();
		ImGui::Spacing();
		ImGui::Separator();
		/*ImGui::TextWrapped("Nero");
		for (auto& move : movesSpeedDataNero)
			move->print_settings();*/
	}
};
//clang-format on