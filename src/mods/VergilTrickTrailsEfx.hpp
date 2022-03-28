#pragma once
#include "Mod.hpp"
#include "GameFunctions/GameModelRequestSetEffect.hpp"
#include "GameFunctions/PlayerCheckNormalJump.hpp"
#include "PlayerTracker.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h"

namespace gf = GameFunctions;
class VergilTrickTrailsEfx : public Mod
{
private:

#define SELECTABLE_STYLE_ACT	ImVec4(0.26f, 0.39f, 0.58f, 0.41f)
#define SELECTABLE_STYLE_HVR	ImVec4(0.26f, 0.59f, 0.98f, 0.61f)
#define SELECTABLE_STYLE_ACT_HVR	ImVec4(0.26f, 0.59f, 0.98f, 0.91f) 

	static inline std::shared_ptr<gf::GameModelRequestSetEffect::EffectID> trickEfxID = nullptr;
	static inline std::shared_ptr<gf::GameModelRequestSetEffect::EffectID> trickEfxIDReverse = nullptr;

	static constexpr float sin45 = 0.7071067811865f;

	static inline const gf::Quaternion verticalRot {sin45, 0, 0, sin45};
	static inline const gf::Quaternion reverseVerticalRot { -sin45, 0, 0, sin45 };
	static inline const gf::Quaternion horizontalRot { 0, sin45, 0, sin45 };
	static inline const gf::Quaternion reverseHorRot{ 0, 0, 1.0f, 0};

	// (0; 0; 1; 0;) * (0; 0.5; 0.965; 0) - 45* down-forward
	//(-0.382; 0; 0; 0.923) - 45* down-back (z offs - -0.8, ox - -2);
	//(0.923; 0; 0; 0.382) - 45 forward-up (z - 2; ox - 2);

	enum Efx
	{
		BossStartTrails,
		SmallTrails,
		M18WindAndDust,
		BlueFlame,
		YMStinger,
		SDTTrickDodge,
		SDTAirTrick,
		PlBlurTrick,
		JceEndTeleport,
		JceEndTeleLighttnings
	};

	enum TrickType
	{
		None,
		mAirTrick,
		mTrickUp,
		mTrickDown,
		mTrickDodge
	};

	static inline TrickType lastTrick = mAirTrick;

	class IActionFxModSetup
	{
		virtual void print_fx_list_selection() = 0;
		virtual void print_spec_settings() = 0;
		virtual void load_data(const utility::Config &cfg) = 0;
		virtual void save_data(utility::Config &cfg) const = 0 ;
	};

	struct EfxInfo
	{
		const Efx efx;
		const int containerID;
		const int elementID;
		const char* name;
		const bool canBeRotated = true;
		const bool canBeSideRotated;
		float offsetXY;
		float offsetZ;		

		EfxInfo(Efx fx, int containerId, int elemId, const char* fxName, bool canRotate = true, bool can2Spawn = true, float offsXY = 0, float offsZ = 0) : efx(fx), containerID(containerId), elementID(elemId), name(fxName), 
		offsetXY(offsXY), offsetZ(offsZ), canBeRotated(canRotate), canBeSideRotated(can2Spawn) {}

		EfxInfo(const EfxInfo& obj) : EfxInfo(obj.efx, obj.containerID, obj.elementID, obj.name, obj.canBeRotated, obj.canBeSideRotated, obj.offsetXY, obj.offsetZ) {}
	};

	struct ModeInfo
	{
	private:
		TrickType mMode;
		const char* mName;
	public:
		ModeInfo(TrickType mode, const char* name) : mMode(mode), mName(name)
		{
		}
		TrickType get_mode() const noexcept
		{
			return mMode;
		}
		const char* get_name() const noexcept
		{
			return mName;
		}
	};

	class TrickModeSetup : IActionFxModSetup
	{
	protected:
		std::vector<std::unique_ptr<EfxInfo>> linkedEfx;

		TrickType mode;

		const char* trickName;

		std::string modCfgName = "VergilTrickTrailsEfx.";

		enum SpawnPosType
		{
			Back,
			Front,
			Both
		};

		SpawnPosType spawnType = Back;
		SpawnPosType endSpawnType = Back;

		bool isEnabled = false;
		bool isSpawnOnEnd = false;
		bool isAlwaysVertical = false;

		bool is_already_linked(Efx efx)
		{
			for (const auto &fx : linkedEfx)
			{
				if(fx->efx == efx)
					return true;
			}
			return false;
		}

		void get_pl_transform_data(uintptr_t vergil, gf::Vec3 &pPos, gf::Quaternion &pRot)
		{
			auto gameObj = *(uintptr_t*)(vergil + 0x10);
			auto transform = *(uintptr_t*)(gameObj + 0x18);
			pPos = *(gf::Vec3*)(transform + 0x30);
			pRot = *(gf::Quaternion*)(transform + 0x40);
		}

		void print_common_fx_info()
		{
			ImGui::Checkbox(("Spawn after end of the trick move##" + std::string(trickName)).c_str(), &isSpawnOnEnd);
			ImGui::TextWrapped("Spawn position type:");
			ImGui::RadioButton(("Back side##" + std::string(trickName)).c_str(), (int*)&spawnType, 0);
			ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
			ImGui::RadioButton(("Front side##" + std::string(trickName)).c_str(), (int*)&spawnType, 1);
			ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
			ImGui::RadioButton(("Both sides##" + std::string(trickName)).c_str(), (int*)&spawnType, 2);
			if (isSpawnOnEnd)
			{
				ImGui::TextWrapped("End spawn position type:");
				ImGui::RadioButton(("Back side##End" + std::string(trickName)).c_str(), (int*)&endSpawnType, 0);
				ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
				ImGui::RadioButton(("Front side##End" + std::string(trickName)).c_str(), (int*)&endSpawnType, 1);
				ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
				ImGui::RadioButton(("Both sides##End" + std::string(trickName)).c_str(), (int*)&endSpawnType, 2);
			}
			ImGui::Spacing();
			for (auto& fx : linkedEfx)
			{
				std::string uniqStr = "Offset for " + std::string(fx->name) + " XY";// + " ##" + std::string(trickName);
				ImGui::TextWrapped(uniqStr.c_str());
				uniqStr += " ##" + std::string(trickName);
				ImGui::InputFloat(std::string("##" + uniqStr).c_str(), (float*)&(fx->offsetXY), 0.1f, 0.1f, "%.2f");
				ImGui::ShowHelpMarker("Forward-backward offset for spawn efx");
				uniqStr = "Offset for " + std::string(fx->name) + " Z";// + " ##" + std::string(trickName);
				ImGui::TextWrapped(uniqStr.c_str());
				uniqStr += " ##" + std::string(trickName);
				ImGui::InputFloat(std::string("##" + uniqStr).c_str(), (float*)&(fx->offsetZ), 0.1f, 0.1f, "%.2f");
			}
		}

		void print_name_selection()
		{
			ImGui::Checkbox(("##" + std::string(trickName)).c_str(), &isEnabled);
			ImGui::SameLine();
			ImGui::TextWrapped(trickName);
			//ImGui::Separator();
		}

		TrickModeSetup() {}

		void trick_pos_type(const EfxInfo* efx, gf::Vec3 spawnOffsAngle, const gf::Quaternion& pRot, uintptr_t vergil, gf::Vec3& spawnPos, gf::Quaternion& spawnRot)
		{
			if (!efx->canBeSideRotated)
				return;
			switch (spawnType)
			{
				case VergilTrickTrailsEfx::TrickModeSetup::Back:
				{

					break;
				}
				case VergilTrickTrailsEfx::TrickModeSetup::Front:
				{
					if (efx->canBeRotated)
						spawnRot = isAlwaysVertical ? pRot * reverseVerticalRot : spawnRot * reverseHorRot;
					else
						spawnRot *= reverseHorRot;
					spawnPos = spawnPos - (2 * spawnOffsAngle);
					break;
				}
				case VergilTrickTrailsEfx::TrickModeSetup::Both:
				{
					if(!efx->canBeSideRotated)
						break;
					gf::Quaternion spawnRot2;
					if (efx->canBeRotated)
						spawnRot2 = isAlwaysVertical ? pRot * reverseVerticalRot : spawnRot * reverseHorRot;
					else
						spawnRot2 = spawnRot * reverseHorRot;
					gf::Vec3 spawnPos2;
					gf::GameModelRequestSetEffect fxSecReq { vergil, trickEfxIDReverse };
					trickEfxIDReverse->ContainerID = efx->containerID;
					trickEfxIDReverse->ElementID = efx->elementID;
					trickEfxIDReverse->IsDataContainerIndxOnly = false;
					trickEfxIDReverse->DataContainerIndx = 1;
					spawnPos2 = spawnPos - (2 * spawnOffsAngle);
					if (isAlwaysVertical)
						spawnPos2.z -= efx->offsetZ;
					fxSecReq(spawnPos2, spawnRot2);
					break;
				}
				default:
					break;
			}
		}

	public:

		TrickModeSetup(TrickType trickType, const char* trickActionName) : mode(trickType), trickName(trickActionName) 
		{
			modCfgName += std::string(trickActionName);
		}

		TrickType get_mode() const noexcept
		{
			return mode;
		}

		const char* get_trick_action_name() const noexcept
		{
			return trickName;
		}

		void add_efx(Efx efx)
		{
			if (is_already_linked(efx))
				return;
			for (const auto fx : efxList)
			{
				if(fx->efx == efx)
					linkedEfx.emplace_back(std::make_unique<EfxInfo>(*fx));
			}
		}

		void add_efx(const EfxInfo *efx)
		{
			if (is_already_linked(efx->efx))
				return;
			linkedEfx.emplace_back(std::make_unique<EfxInfo>(*efx));
		}

		void remove_efx(int indx)
		{
			try
			{
				linkedEfx.erase(linkedEfx.begin() + indx);
			}
			catch (const std::exception& e)
			{
			}
		}

		void remove_efx(Efx efx)
		{
			for (int i = 0; i < linkedEfx.size(); i++)
			{
				if (linkedEfx[i]->efx == efx)
				{
					linkedEfx.erase(linkedEfx.begin() + i);
					break;
				}
			}
		}

		const EfxInfo& operator[](int indx) const
		{
			return *(linkedEfx[indx]);
		}

		int get_linked_efx_size() const
		{
			return linkedEfx.size();
		}

		virtual volatile void* request_efx(uintptr_t vergil)
		{
			if(!isEnabled)
				return nullptr;
			volatile void* res = nullptr;
			gf::Vec3 pPos;
			gf::Quaternion pRot;
			get_pl_transform_data(vergil, pPos, pRot);
			gf::Quaternion spawnRot;
			gf::Vec3 spawnPos;
			gf::GameModelRequestSetEffect fxRequest { vergil, trickEfxID };
			
			for (const auto &efx : linkedEfx)
			{
				spawnPos = pPos;
				spawnRot = pRot;
				trickEfxID->ContainerID = efx->containerID;
				trickEfxID->ElementID = efx->elementID;
				
				gf::Vec3 spawnOffsAngle;
				if (efx->offsetXY != 0)
				{
					gf::Vec3 fVec(0, efx->offsetXY, 0);
					spawnOffsAngle = gf::Quaternion::q_rot(spawnRot, fVec);
					spawnPos += spawnOffsAngle;
				}

				if(efx->offsetZ != 0)
					spawnPos.z += efx->offsetZ;

				if (isAlwaysVertical && efx->canBeRotated)
					spawnRot *= verticalRot;

				trick_pos_type(efx.get(), spawnOffsAngle, pRot, vergil, spawnPos, spawnRot);

				res = fxRequest(spawnPos, spawnRot);
			}
			lastTrick = mAirTrick;
			return res;
		}

		virtual volatile void* request_end_efx(uintptr_t vergil)
		{
			volatile void *res = nullptr;
			if (isEnabled && isSpawnOnEnd && lastTrick == mode && *(int*)(vergil + 0xE64) == 4)
			{
				SpawnPosType temp = spawnType;
				spawnType = endSpawnType;
				res = request_efx(vergil);
				spawnType = temp;
			}
			lastTrick = None;
			return res;
		}

		// Inherited via IActionFxModSetup
		void print_fx_list_selection() override
		{
			if (isEnabled)
			{
				bool isSelected = false;
				ImGui::TextWrapped("Select efx:");
				if (ImGui::BeginListBox(("##FxList" + std::string(trickName)).c_str()))
				{
					for (const auto& fx : efxList)
					{
						isSelected = is_already_linked(fx->efx);
						ImVec4 backgroundcolor = isSelected ? ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered) : SELECTABLE_STYLE_ACT;
						ImGui::PushStyleColor(ImGuiCol_Header, backgroundcolor);
						ImGui::PushStyleColor(ImGuiCol_HeaderHovered, SELECTABLE_STYLE_ACT_HVR);
						if (!isSelected)
						{
							if (ImGui::Selectable((std::string(fx->name) + "##" + std::string(trickName)).c_str(), isSelected))
								add_efx(fx->efx);
							else
								backgroundcolor = SELECTABLE_STYLE_HVR;
						}
						else
						{
							if (ImGui::Selectable((std::string(fx->name) + "##" + std::string(trickName)).c_str(), isSelected))
								remove_efx(fx->efx);
						}

						ImGui::PopStyleColor(2);
					}
					ImGui::EndListBox();
				}
			}
		};

		void print_spec_settings() override
		{
			print_name_selection();
			if (isEnabled)
			{
				ImGui::Checkbox("Always vertical efx", &isAlwaysVertical);
				print_common_fx_info();
			}
		};

		void load_data(const utility::Config& cfg) override
		{
			linkedEfx.clear();
			isEnabled = cfg.get<bool>(modCfgName + ".isEnabled").value_or(true);
			isAlwaysVertical = cfg.get<bool>(modCfgName + ".isAlwaysVertical").value_or(false);
			isSpawnOnEnd = cfg.get<bool>(modCfgName + ".isSpawnOnEnd").value_or(false);
			spawnType = (SpawnPosType)cfg.get<int>(modCfgName + ".spawnType").value_or(SpawnPosType::Back);
			endSpawnType = (SpawnPosType)cfg.get<int>(modCfgName + ".endSpawnType").value_or(SpawnPosType::Back);
			for (const auto fx : efxList)
			{
				auto tmp = cfg.get<bool>(modCfgName + "_" + std::string(fx->name)).value_or(false);
				if (tmp)
				{
					add_efx(fx);
					linkedEfx[linkedEfx.size() - 1]->offsetXY = cfg.get<float>(modCfgName + "_" + std::string(fx->name) + ".offsetXY").value_or(0);
					linkedEfx[linkedEfx.size() - 1]->offsetZ = cfg.get<float>(modCfgName + "_" + std::string(fx->name) + ".offsetZ").value_or(0);
				}
			}
		}

		void save_data(utility::Config& cfg) const override
		{
			cfg.set<bool>(modCfgName + ".isEnabled", isEnabled);
			cfg.set<bool>(modCfgName + ".isAlwaysVertical", isAlwaysVertical);
			cfg.set<bool>(modCfgName + ".isSpawnOnEnd", isSpawnOnEnd);
			cfg.set<int>(modCfgName + ".spawnType", (int)spawnType);
			cfg.set<int>(modCfgName + ".endSpawnType", (int)endSpawnType);
			for(const auto fx : efxList)
				cfg.set<bool>(modCfgName + "_" + std::string(fx->name), false);
			for (const auto& fx : linkedEfx)
			{
				std::string fxName = modCfgName + "_" + std::string(fx->name);
				cfg.set<bool>(fxName, true);
				cfg.set<float>(fxName + ".offsetXY", fx->offsetXY);
				cfg.set<float>(fxName + ".offsetZ", fx->offsetZ);
			}
		}

		inline bool is_enabled() const noexcept { return isEnabled; }

		inline bool is_spawn_in_the_end() const noexcept { return isSpawnOnEnd; }
	};

	class TrickUpSetup : public TrickModeSetup
	{
	private:
		bool isAlwaysHorizontal = false;
	public:

		TrickUpSetup() : TrickModeSetup()
		{
			mode = mTrickUp;
			trickName = "Trick Up";
			modCfgName += std::string(trickName);
		}

		volatile void* request_efx(uintptr_t vergil) override
		{
			isAlwaysVertical = !isAlwaysHorizontal;
			auto res = TrickModeSetup::request_efx(vergil);
			lastTrick = mTrickUp;
			return res;
		}

		void print_spec_settings() override
		{
			print_name_selection();
			if (isEnabled)
			{
				ImGui::Checkbox("Always horizontal efx", &isAlwaysHorizontal);
				print_common_fx_info();
			}
		}

		void load_data(const utility::Config& cfg) override
		{
			TrickModeSetup::load_data(cfg);
			isAlwaysHorizontal = cfg.get<bool>(modCfgName + ".isAlwaysHorizontal").value_or(false);
		}

		void save_data(utility::Config& cfg) const override
		{
			TrickModeSetup::save_data(cfg);
			cfg.set<bool>(modCfgName + ".isAlwaysHorizontal", isAlwaysHorizontal);
		}

		bool inline is_always_horizontal() const noexcept { return isAlwaysHorizontal; }
	};

	class TrickDownSetup : public TrickModeSetup
	{
	private:
		enum EfxDirection
		{
			Horizontal,
			Vertical,
			AngleDefault
		};

		EfxDirection fxDirection = AngleDefault;
		
	public:

		TrickDownSetup()
		{
			mode = mTrickDown;
			trickName = "Trick Down";
			modCfgName += std::string(trickName);
		}

		volatile void* request_efx(uintptr_t vergil) override
		{
			if (!isEnabled)
				return nullptr;
			volatile void* res = nullptr;
			gf::Vec3 pPos;
			gf::Quaternion pRot;
			get_pl_transform_data(vergil, pPos, pRot);
			gf::Quaternion spawnRot;
			gf::Vec3 spawnPos;
			gf::GameModelRequestSetEffect fxRequest { vergil, trickEfxID };

			for (const auto& efx : linkedEfx)
			{
				spawnPos = pPos;
				spawnRot = pRot;
				trickEfxID->ContainerID = efx->containerID;
				trickEfxID->ElementID = efx->elementID;

				gf::Vec3 spawnOffsAngle;
				if (efx->offsetXY != 0)
				{

					gf::Vec3 fVec(0,0,0);//(0, efx->offsetXY, 0);
					if(spawnType == Front && lastTrick != mTrickDown)
						fVec.y = -efx->offsetXY;
					else
						fVec.y = efx->offsetXY;
					spawnOffsAngle = gf::Quaternion::q_rot(spawnRot, fVec);
					spawnPos += spawnOffsAngle;
				}

				if (efx->offsetZ != 0)
					spawnPos.z += efx->offsetZ;

				if (isAlwaysVertical && efx->canBeRotated)
					spawnRot *= verticalRot;

				auto dirTmp = fxDirection;
				if(!efx->canBeSideRotated)
					dirTmp = Horizontal;

				switch (fxDirection)
				{
					case VergilTrickTrailsEfx::TrickDownSetup::Horizontal:
					{
						isAlwaysVertical = false;
						trick_pos_type(efx.get(), spawnOffsAngle, pRot, vergil, spawnPos, spawnRot);
						break;
					}
						
					case VergilTrickTrailsEfx::TrickDownSetup::Vertical:
					{
						isAlwaysVertical = true;
						trick_pos_type(efx.get(), spawnOffsAngle, pRot, vergil, spawnPos, spawnRot);
						break;
					}
					case VergilTrickTrailsEfx::TrickDownSetup::AngleDefault:
					{
						isAlwaysVertical = false;
						gf::PlayerCheckNormalJump checkAir{vergil};
						if (!checkAir() && lastTrick != mTrickDown)
						{
							/*if(spawnType == Front)
								efx->offsetXY *= -1.0f;*/
							if (spawnType == Back)
							{
								spawnPos.z += 1.95f;
								spawnPos -= spawnOffsAngle;
							}
							spawnRot *= gf::Quaternion(0.923f, 0, 0, 0.382f);
							trick_pos_type(efx.get(), spawnOffsAngle, pRot, vergil, spawnPos, spawnRot);
							/*if (spawnType == Front)
								efx->offsetXY *= -1.0f;*/
						}
						else
							trick_pos_type(efx.get(), spawnOffsAngle, pRot, vergil, spawnPos, spawnRot);
						break;
					}
					default:
						break;
				}
				res = fxRequest(spawnPos, spawnRot);
				fxDirection = dirTmp;
			}
			lastTrick = mTrickDown;
			return res;
		}

		void print_spec_settings() override
		{
			print_name_selection();
			if (isEnabled)
			{
				ImGui::TextWrapped("Select trick down efx angle spawn type:");
				ImGui::RadioButton("Always horizontal", (int*)&fxDirection, 0);
				ImGui::SameLine(); ImGui::Separator(); ImGui::SameLine();
				ImGui::RadioButton("Always vertical", (int*)&fxDirection, 1);
				ImGui::SameLine(); ImGui::Separator(); ImGui::SameLine();
				ImGui::RadioButton("Angled dependence of current Vergils air state", (int*)&fxDirection, 2);
				print_common_fx_info();
			}
		}

		void load_data(const utility::Config& cfg) override
		{
			TrickModeSetup::load_data(cfg);
			fxDirection = (EfxDirection)cfg.get<int>(modCfgName + ".fxDirection").value_or(2);
		}

		void save_data(utility::Config& cfg) const override
		{
			TrickModeSetup::save_data(cfg);
			cfg.set<int>(modCfgName + ".fxDirection", (int)fxDirection);
		}
	};

	void init_check_box_info() override;

	static inline const std::array<EfxInfo*, 8> efxList {
		new EfxInfo{BossStartTrails, 1, 12, "Boss start trick trails", true, true, -2.0f, 0},
		new EfxInfo{SmallTrails, 0, 20, "Rapid slash'es or yamato helm breaker's trails", true, true, 0.5f, 0},
		new EfxInfo{M18WindAndDust, 3, 3, "Some dust and wind", false, false, 0, 0},
		new EfxInfo{BlueFlame, 4, 47, "(M)FE unused blue flame efx", true, true, 0, 0},
		//new EfxInfo{YMStinger, 0, 16, "Yamato stinger flashes", -0.5f, 0},
		new EfxInfo{SDTTrickDodge, 300, 17, "Player's SDT trick dodge", true, true, 0.95f, 0},
		new EfxInfo{SDTAirTrick, 300, 27, "Player's SDT air trick", false, false, -0.5f, 0},
		new EfxInfo{PlBlurTrick, 1, 17, "Player's blur when trick (boss's cloud if using file mod)", false, true, 0, 0},
		new EfxInfo{JceEndTeleport, 10, 33, "JCE end teleport", true, true, 0, 0},
		//new EfxInfo{JceEndTeleLighttnings, 10, 34, "JCE end lightnings", true, true, 0, 0}
	};

	static inline std::array<std::unique_ptr<TrickModeSetup>, 3> trickActionsSettings {
		std::make_unique<TrickModeSetup>(mAirTrick, "Air Trick"),
		std::make_unique<TrickUpSetup>(),
		std::make_unique<TrickDownSetup>()
	};

	std::unique_ptr<FunctionHook> m_trick_start_hook;
	std::unique_ptr<FunctionHook> m_air_trick_end_hook;
	std::unique_ptr<FunctionHook> m_trick_set_draw_self_hook;

public:

	static inline bool cheaton = false;

	static inline uintptr_t airTrickEndRet = 0;
	static inline uintptr_t actionSetDrawSelfRet = 0;
	static inline uintptr_t airTrickEndDrawSelfCall = 0;

	VergilTrickTrailsEfx() = default;

	~VergilTrickTrailsEfx()
	{
		for(int i = 0; i < efxList.size(); i++)
			delete (efxList[i]);
	}

	std::string_view get_name() const override
	{
		return "VergilTrickTrailsEfx";
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

	static inline bool useCustomRot = false;

	static void set_up_end_efx_asm(uintptr_t vergil);

	static void set_trick_efx(uintptr_t netStr, uintptr_t vergil);
};