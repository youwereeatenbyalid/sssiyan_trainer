#pragma once
#include "Mod.hpp"
#include "Mods.hpp"
#include "PlayerTracker.hpp"
#include "EnemyData.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h"
#include "GameFunctions/PlayerCheckNormalJump.hpp"

//clang-format off

class ParryWithFinesse : public Mod, private EndLvlHooks::IEndLvl
{
public:
	enum class ParryReact
	{
		Stun,
		StunReturn,
		Dying,
		DyingReturn,
		ParryOptional
	};

	enum class Pl0800ParryType
	{
		Default,
		PlParry,
		YamatoPlParry,
		YMComboBCounter
	};

private:

	enum EnemyAction
	{
		None = -1,
		Stun = 3200,//0xC80
		StunReturn = 3201,//0xC81
		Dying = 3230,//0xC9E
		DyingReturn = 3231,//0xC9F
		ParryWin = 200,//0xC8
		ParryLose = 201,//0xC9
		ParryDraw = 202,//0xCA
		Guard = 70,//0x46
		GuardHit = 71,//0x47
		GuardReturn = 72,//0x48
		GuardBreak = 73,//0x49
		GuardRepel = 74,//0x4A
		DamageStandS = 3000,//0xBB8
		DamageStandL = 3001//0xBB9
	};

	sdk::REMethodDefinition* _emSetActionMethod;
	//sdk::REMethodDefinition* _emSetActionStrMethod;
	sdk::REMethodDefinition* _plSetActionMethod;
	sdk::REMethodDefinition *_pl0800ChangeWeaponSMethod;
	sdk::REMethodDefinition* _plSetActionFullMethod;

	ParryReact _parryReact = ParryReact::ParryOptional;

	Pl0800ParryType _plParryType = Pl0800ParryType::Default;

	std::unique_ptr<gf::SysString> _ymComboB3;

	inline EnemyAction convert_to_em_action(ParryReact react)
	{
		switch (react)
		{
			case ParryWithFinesse::ParryReact::Stun:
				return EnemyAction::Stun;
			case ParryWithFinesse::ParryReact::StunReturn:
				return EnemyAction::StunReturn;
			case ParryWithFinesse::ParryReact::Dying:
				return EnemyAction::Dying;
			case ParryWithFinesse::ParryReact::DyingReturn:
				return EnemyAction::DyingReturn;
			/*case ParryWithFinesse::ParryReact::ParryOptional:
				break;*/
			default:
				return StunReturn;
		}
	}

	void reset(EndLvlHooks::EndType type) override
	{
		_ymComboB3 = nullptr;
	}

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	inline void set_em_parry(uintptr_t threadCntx, uintptr_t em, EnemyAction action)
	{
		_emSetActionMethod->call(threadCntx, em, (int32_t)action, 0, 10.0f, 0.0f, 1, 0, true);
	}

	void on_pl0800_block(uintptr_t threadCntx, uintptr_t pl0800, uintptr_t hitCtrlDamageInfo, int* fRes)
	{
		if (!cheaton || *(int*)(pl0800 + 0x1B5C) != 2 || hitCtrlDamageInfo == 0)
			return;
		auto attackHitCtrlOwner = *(uintptr_t*)(hitCtrlDamageInfo + 0xD0);
		auto damageHitCtrlOwner = *(uintptr_t*)(hitCtrlDamageInfo + 0xD8);
		auto attackHitCntr = *(uintptr_t*)(hitCtrlDamageInfo + 0xB8);
		//auto damageHitCntr = *(uintptr_t*)(hitCtrlDamageInfo + 0xC0);
		if (attackHitCtrlOwner == 0 || damageHitCtrlOwner == 0)
			return;

		uintptr_t attackChar = 0;
		auto idOwner = EnemyData::get_em_id(*(uintptr_t*)(attackHitCtrlOwner + 0xA0));
		if(idOwner != EnemyData::Shadow)
			attackChar = *(uintptr_t*)(attackHitCntr + 0xA0);
		else
			attackChar = *(uintptr_t*)(attackHitCtrlOwner + 0xA0);

		if (attackChar == 0)
			return;
		
		auto emId = idOwner == EnemyData::Shadow ? idOwner : EnemyData::get_em_id(attackChar);
		if (emId == EnemyData::None)
			return;

		bool isPlAttaker = emId == EnemyData::Dante || emId == EnemyData::Vergil;

		if (isPlAttaker)
		{
			switch (_parryReact)
			{
				case ParryWithFinesse::ParryReact::Stun:
				{
					set_pl_action(threadCntx, attackChar, L"Stun", 0, 0, 0, 0, 0, true, true, false, 0);
					break;
				}
				case ParryWithFinesse::ParryReact::StunReturn:
				{
					set_pl_action(threadCntx, attackChar, L"StunEnd", 0, 0, 0, 0, 0, true, true, false, 0);
					break;
				}
				default:
				{
					gf::PlayerCheckNormalJump checkJump{ attackChar };
					if(checkJump())
						set_pl_action(threadCntx, attackChar, L"Parry", 0, 0, 0, 0, 0, true, true, false, 0);
					else
						set_pl_action(threadCntx, attackChar, L"ParryAir", 0, 0, 0, 0, 0, true, true, false, 0);
					break;
				}
			}
		}
		else
		{
			if (_parryReact != ParryReact::ParryOptional)
			{
				set_em_parry(threadCntx, attackChar, convert_to_em_action(_parryReact));
			}
			else// i regret nothing
			{
				EnemyAction action = Dying;
				switch (emId)
				{
					case EnemyData::None:
						break;
					case EnemyData::HellCaina:
					{
						action = GuardBreak;
						break;
					}
					case EnemyData::HellAntenora:
					{
						action = ParryLose;
						break;
					}
					case EnemyData::HellJudecca:
					{
						action = StunReturn;
						break;
					}
					case EnemyData::Empusa:
					{
						action = Dying;
						break;
					}
					case EnemyData::GreenEmpusa:
					{
						action = StunReturn;
						break;
					}
					case EnemyData::RedEmpusa:
					{
						action = Dying;
						break;
					}
					case EnemyData::EmpusaQueen:
					{
						action = (EnemyAction)0x18BC2;//EmpusaQueenParryLose
						break;
					}
					case EnemyData::Riot:
					{
						action = (EnemyAction)0x18E76;//RiotDamageRepelled
						break;
					}
					case EnemyData::Chaos:
					{
						action = (EnemyAction)0x18ED6;//ChaosDamageRollingAir
						break;
					}
					case EnemyData::Fury:
					{
						action = (EnemyAction)0x18F42;//ChaosDamageRollingAir
						break;
					}
					/*case EnemyData::Baphomet:
						break;*/
					case EnemyData::Lusachia:
					{
						action = (EnemyAction)0x192CB;//LusachiaTried
						break;
					}
					case EnemyData::Behemoth:
					{
						action = DamageStandL;
						break;
					}
					case EnemyData::Nobody:
					{
						action = ParryLose;
						break;
					}
					case EnemyData::ScudoAngelo:
					{
						action = GuardBreak;
						break;
					}
					case EnemyData::ProtoAngelo:
					{
						action = ParryLose;
						break;
					}
					case EnemyData::DeathScissors:
					{
						action = (EnemyAction)0x1A1FD;//BreakWeaponFull
						break;
					}
					case EnemyData::Hellbat:
					{
						action = DamageStandL;
						break;
					}
					case EnemyData::Pyrobat:
					{
						action = DamageStandL;
						break;
					}
					case EnemyData::QliphotsTentacle:
					{
						action = ParryLose;
						break;
					}
					case EnemyData::Goliath:
					{
						action = DamageStandS;
						break;
					}
					case EnemyData::Malphas:
					{
						action = (EnemyAction)0x24DDA;
						break;
					}
					/*case EnemyData::Nidhogg:
						break;*/
					case EnemyData::Artemis:
					{
						action = (EnemyAction)0xC1F;//DamageDownFaceDown
						break;
					}
					/*case EnemyData::Gilgamesh:
						break;*/
					case EnemyData::ElderGeryonKnight:
					{
						action = Dying;
						if (auto cav = *(uintptr_t*)(attackChar + 0xDB0); cav != 0)
							set_em_parry(threadCntx, cav, ParryLose);
						break;
					}
					case EnemyData::Cavaliere:
					{
						action = ParryLose;
						if (auto horse = *(uintptr_t*)(attackChar + 0xE88); horse != 0)
							set_em_parry(threadCntx, horse, DyingReturn);
						break;
					}
					case EnemyData::QliphothRootBoss:
					{
						action = ParryLose;
						break;
					}
					case EnemyData::KingCerberus:
					{
						action = Dying; //0x26567
						break;
					}
					case EnemyData::Griffon:
					{
						action = DamageStandS;
						break;
					}
					case EnemyData::Shadow:
					{
						action = DamageStandL;
						break;
					}
					case EnemyData::Nightmare:
					{
						action = DamageStandS;
						break;
					}
					case EnemyData::Urizen2:
					{
						action = DamageStandS;
						break;
					}
					case EnemyData::Urizen3:
					{
						action = DamageStandS;
						break;
					}
					case EnemyData::PhantomGoliath:
					{
						action = DamageStandS;
						break;
					}
					case EnemyData::PhantomArtemis:
					{
						action = (EnemyAction)0xC1F;//DamageDownFaceDown
						break;
					}
					case EnemyData::PhantomCavaliere:
					{
						action = ParryLose;
						break;
					}
					default:
					{
						action = StunReturn;
						break;
					}
				}
				set_em_parry(threadCntx, attackChar, action);
			}
			
		}

		gf::PlayerCheckNormalJump checkJump{ pl0800 };

		switch (_plParryType)
		{
			case ParryWithFinesse::Pl0800ParryType::PlParry:
			{
				if (checkJump())
					set_pl_action(threadCntx, pl0800, L"Parry");
				else
					set_pl_action(threadCntx, pl0800, L"ParryAir");
				break;
			}
			case ParryWithFinesse::Pl0800ParryType::YamatoPlParry:
			{
				pl0800_change_weapon(threadCntx, pl0800, 0);
				if (checkJump())
					set_pl_action(threadCntx, pl0800, L"Parry");
				else
					set_pl_action(threadCntx, pl0800, L"ParryAir");
				break;
			}
			case Pl0800ParryType::YMComboBCounter:
			{
				pl0800_change_weapon(threadCntx, pl0800, 0);
				if (_ymComboB3 == nullptr)
					_ymComboB3 = std::make_unique<gf::SysString>(L"Yamato.YM_Combo.YMT_ComboA.A-03");
				set_pl_action_full(threadCntx, pl0800, _ymComboB3.get());
			}
			default:
				break;
		}
	}

	inline void pl0800_change_weapon(uintptr_t threadCntx, uintptr_t pl0800, int weapon)
	{
		int curWeapon = *(int*)(pl0800 + 0x1978);
		if (curWeapon != weapon)
			_pl0800ChangeWeaponSMethod->call(threadCntx, pl0800, weapon, -1);
	}

	inline void set_pl_action(uintptr_t threadCntx, uintptr_t pl, const wchar_t* action, int layerNo = 0, float startFrame = 0.0f, float interpolationFrame = 0.0f, int mode = 0, 
		int curve = 0, bool isImmediate = true, bool passSelect = false, bool isPuppetTransition = false, int priority = 0)
	{
		gf::SysString str(action);
		_plSetActionMethod->call(threadCntx, pl, str.get_net_str(), layerNo, startFrame, interpolationFrame, mode, curve, isImmediate, passSelect, isPuppetTransition, priority);
	}

	inline void set_pl_action_full(uintptr_t threadCntx, uintptr_t pl, const gf::SysString *action, bool fullName = false, int layerNo = 0, float startFrame = 0.0f, float interpolationFrame = 0.0f, int mode = 0,
		int curve = 0, bool isImmediate = true, bool passSelect = false, bool isPuppetTransition = false, int priority = 0)
	{
		_plSetActionFullMethod->call(threadCntx, pl, action->get_net_str(), layerNo, startFrame, interpolationFrame, mode, curve, isImmediate, passSelect, isPuppetTransition, priority);
	}

	void on_sdk_init() override
	{
		_emSetActionMethod = sdk::find_method_definition("app.Enemy",
			"setAction(app.Enemy.ActionEnum, System.UInt32, System.Single, System.Single, via.motion.InterpolationMode, via.motion.InterpolationCurve, System.Boolean)");
		_plSetActionMethod = sdk::find_method_definition("app.Player", "setAction(System.String, System.UInt32, System.Single, System.Single, via.motion.InterpolationMode, via.motion.InterpolationCurve, "
			"System.Boolean, System.Boolean, System.Boolean, app.GameModel.ActionPriority)");
		_pl0800ChangeWeaponSMethod = sdk::find_method_definition("app.PlayerVergilPL", "changeWeaponS(app.PlayerVergilPL.WeaponS, System.Int32)");
		_plSetActionFullMethod = sdk::find_method_definition("app.Player", "setActionFullName(System.String, System.UInt32, System.Single, System.Single, via.motion.InterpolationMode, "
			"via.motion.InterpolationCurve, System.Boolean, System.Boolean, System.Boolean, app.GameModel.ActionPriority)");
	}

public:
	ParryWithFinesse() = default;

	~ParryWithFinesse()
	{
		PlayerTracker::after_pl0800_guard_check_unsub(std::make_shared<Events::EventHandler<ParryWithFinesse, uintptr_t, uintptr_t, uintptr_t, int*>>(this, &ParryWithFinesse::on_pl0800_block));
	}

	static inline bool cheaton = true;

	std::string_view get_name() const override
	{
		return "ParryWithFinesse";
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
		m_depends_on = { "PlayerTracker" };
		m_full_name_string = "Parry With Finesse (+)";
		m_author_string = "V.P.Zadov";
		m_description_string = "Modifies Vergil's level 2 motivation block to a full counter.";

		set_up_hotkey();

		PlayerTracker::after_pl0800_guard_check_sub(std::make_shared<Events::EventHandler<ParryWithFinesse, uintptr_t, uintptr_t, uintptr_t, int*>>(this, &ParryWithFinesse::on_pl0800_block));

		return Mod::on_initialize();
	}

	void on_config_load(const utility::Config& cfg) override
	{
		_parryReact = (ParryReact)cfg.get<int>("ParryWithFinesse._parryReact").value_or((int)ParryReact::ParryOptional);
		_plParryType = (Pl0800ParryType)cfg.get<int>("ParryWithFinesse._plParryType").value_or((int)Pl0800ParryType::YamatoPlParry);
	}
	void on_config_save(utility::Config& cfg) override
	{
		cfg.set<int>("ParryWithFinesse._parryReact", (int)_parryReact);
		cfg.set<int>("ParryWithFinesse._plParryType", (int)_plParryType);
	}

	void on_draw_ui() override
	{
		ImGui::TextWrapped("Select player parry action:");
		ImGui::RadioButton("Default guard action", (int*)&_plParryType, (int)Pl0800ParryType::Default); 
		ImGui::RadioButton("Player parry action", (int*)&_plParryType, (int)Pl0800ParryType::PlParry); 
		ImGui::ShowHelpMarker("Animation depend of current weapon. Game treat this as attack action so you can't parry again before animation is ended/interrupted.");
		ImGui::RadioButton("Player parry but always with Yamato", (int*)&_plParryType, (int)Pl0800ParryType::YamatoPlParry);
		ImGui::ShowHelpMarker("Same as previous option but Vergil will automatically change weapon to Yamato when parry.");
		ImGui::RadioButton("Counter attack with \"Yamato Combo B\" finish slash", (int*)&_plParryType, (int)Pl0800ParryType::YMComboBCounter);
						
		ImGui::Separator();

		ImGui::TextWrapped("Select parry reaction:");
		ImGui::RadioButton("Stun", (int*)&_parryReact, (int)ParryReact::Stun); 
		ImGui::ShowHelpMarker("Stun will not end before you deal some damage on most of enemies.");
		ImGui::RadioButton("Stun End", (int*)&_parryReact, (int)ParryReact::StunReturn);
		ImGui::RadioButton("Dying", (int*)&_parryReact, (int)ParryReact::Dying); 
		ImGui::RadioButton("Dying end", (int*)&_parryReact, (int)ParryReact::DyingReturn); 
		ImGui::ShowHelpMarker("Same is prev but shorter.");
		ImGui::RadioButton("Parry if possible", (int*)&_parryReact, (int)ParryReact::ParryOptional); 
		ImGui::ShowHelpMarker("If enemy has parry animation - it will be applied. Otherwise some another animation will be happened.");
	}
};
//clang-format on