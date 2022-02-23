#include <spdlog/spdlog.h>
#include "Mods.hpp"
#include "Config.hpp"
#include "imgui_internal.h"
// Example
         #include "mods/SimpleMod.hpp"
// Darkness
    // Background
        #include "mods/FileEditor.hpp"
    // Common
    // Gameplay
    // Nero
    // Dante
    // V
      // Not even in my wildest dreams
    // Vergil
// Hitch
    // Background
        #include "mods/PlayerTracker.hpp"
        #include "mods/StyleRank.hpp"
        #include "mods/GameInput.hpp"
        #include "mods/TextEditor.hpp"
    // Common
        #include "mods/HeavyDay.hpp"
        #include "mods/MoveReplacer.hpp"
        #include "mods/Inertia.hpp"
        #include "mods/LDK.hpp"
        #include "mods/TauntSelector.hpp"
        #include "mods/DisableAutoAssist.hpp"
        #include "mods/DisableTitleTimer.hpp"
        #include "mods/HoldToMash.hpp"
        #include "mods/SpardaWorkshop.hpp"
        //#include "mods/SCNPathEditor.hpp"
    // Gameplay
        #include "mods/AllOrNothing.hpp"
		#include "mods/OneHitKill.hpp"
		#include "mods/NoOneTakesDamage.hpp"
        #include "mods/DifficultySelect.hpp"
    // Nero
        #include "mods/BreakerSwitcher.hpp"
        //#include "mods/DisableBreakaway.hpp"
        #include "mods/CaliburExceed.hpp"
        #include "mods/NothingCancelsBubble.hpp"
		#include "mods/LongerRagtimeBubble.hpp"
        #include "mods/NeroSuperMovesNoDT.hpp"
        #include "mods/ExceedValue.hpp"
        #include "mods/DTWingsOnly.hpp"
    // Dante
    // V
       #include "mods/InfiniteSummonPowerup.hpp"
       #include "mods/InfiniteSummonVitality.hpp"
       #include "mods/PetChargeNoInterrupt.hpp"
       #include "mods/InfiniteGambits.hpp"
       #include "mods/EmpoweredCane.hpp"
    // Vergil
        #include "mods/DoppelWeaponSwitcher.hpp"
// Siyan
    // Background
        #include "mods/DamageTypeLean.hpp"
    // Common
        #include "mods/DeepTurbo.hpp"
        #include "mods/BpStageJump.hpp"
        #include "mods/FreezeBPTimer.hpp"
        #include "mods/NoScreenShake.hpp"
        #include "mods/DisplayEnemyHPInOrbs.hpp"
        #include "mods/MovingTargetSwitch.hpp"
        #include "mods/HUDOptions.hpp"
        #include "mods/CameraSettings.hpp"
        #include "mods/LandCancels.hpp"
        #include "mods/InfHP.hpp"
    // Gameplay
        #include "mods/Reversals.hpp"
        #include "mods/NoJCCooldown.hpp"
        #include "mods/AlwaysSTaunts.hpp"
        #include "mods/AllStart.hpp"
        #include "mods/AllStartManual.hpp"
        #include "mods/JumpStart.hpp"
        #include "mods/DamageMultiplier.hpp"
        #include "mods/PlayerDamageMult.hpp"
        #include "mods/InfDT.hpp"
        #include "mods/EntitySpeeds.hpp"
        #include "mods/FileFrameCuts.hpp"
        #include "mods/SprintInBattle.hpp"
        #include "mods/EnemyStepBanHeight.hpp"
        #include "mods/DisableEnemyAI.hpp"
        #include "mods/EnemyInstantDT.hpp"
        #include "mods/WalkOnKeyboard.hpp"
        #include "mods/ChargeChecker.hpp"
        #include "mods/WeightReset.hpp"
        #include "mods/AerialPushback.hpp"
        #include "mods/AerialPushbackVertical.hpp"
        #include "mods/DisableGauntletStages.hpp"
        #include "mods/NoSlowmoOrHitstop.hpp"
    // Nero
        //#include "mods/NeroInfBreakers.hpp"
        #include "mods/NeroDisableWiresnatch.hpp"
        #include "mods/NeroSwapWiresnatch.hpp"
        #include "mods/NeroTomboyLockOn.hpp"
        #include "mods/NeroSwapSidesteps.hpp"
        #include "mods/NeroInfPunchline.hpp"
        #include "mods/NeroSkipCS2.hpp"
        #include "mods/NoDTCooldown.hpp"
        #include "mods/DTWingsOnly.hpp"
        #include "mods/modNeroAlwaysInitialDT.hpp"
        #include "mods/NeroInfCalibur.hpp"
    // Dante
        #include "mods/BypassBPCav.hpp"
        #include "mods/DanteMaxSDT.hpp"
        #include "mods/DanteQuickSDT.hpp"
        #include "mods/DanteAlwaysQ4SDT.hpp"
        #include "mods/DanteInfQ4ExitWindow.hpp"
        #include "mods/DanteTwoTricks.hpp"
        #include "mods/DanteAlwaysOvertop.hpp"
        #include "mods/DanteInfIgnition.hpp"
        #include "mods/DanteGuardflyWip.hpp"
        #include "mods/DanteVariableGuard.hpp"
        #include "mods/DanteRedlineCav.hpp"
        #include "mods/DanteSDTRework.hpp"
    // V
    // Vergil
        #include "mods/VergilAirTrickNoSS.hpp"
        #include "mods/VergilDirectionalMappings.hpp"
        #include "mods/VergilInfJdCs.hpp"
        #include "mods/VergilInfSDT.hpp"
        #include "mods/VergilInstantSDT.hpp"
        #include "mods/VergilInfConcentration.hpp"
        #include "mods/VergilNoTrickRestriction.hpp"
        #include "mods/VergilTrickUpLockedOn.hpp"
    // Dr.penguin
    // Background
    // Common
    // Gameplay
    // Nero
       #include "mods/NeroGP01overGerberaPickup.hpp"
    // Dante
    // V
    // Vergil
       #include "mods/VergilDoppelBanish.hpp"
// VPZadov
    // Background
    // Common
    // Gameplay
       #include "mods/EnemySwapper.hpp"
       #include "mods/EnemyDataSettings.hpp"
       //#include "mods/EnemyWaveSettings.hpp"
       #include "mods/CheckpointPos.hpp"
       #include "mods/MissionManager.hpp"
       //#include "mods/EnemyWaveEditor.hpp"
       #include "mods/SecretMissionTimer.hpp"
       #include "mods/BossDanteSetup.hpp"
    // Nero
    // Dante
       #include "mods/DanteAirTrickSettings.hpp"
       #include "mods/GroundTrickNoDistanceRestriction.hpp"
       #include "mods/DanteNoSdtStun.hpp"
       #include "mods/JudgementCustomCost.hpp"
       #include "mods/DanteDtNoActivationCost.hpp"
    // V
    // Vergil
       #include "mods/VergilSDTFormTracker.hpp"
       #include "mods//VergilNoAfterimages.hpp"
       #include "mods/VergilDisableSDTAccumulate.hpp"
       #include "mods/VergilSetMaxJJC.hpp"
       //#include "mods/VergilAdditionalJJC.hpp"
       #include "mods/VergilSDTAccumulateRework.hpp"
       #include "mods/VergilSDTNoConcentrationLose.hpp"
       #include "mods/VergilAirTrick.hpp"
       //#include "mods/VergilSDTTrickEfx.hpp" //Removed until better times
       #include "mods//InfiniteTrickUp.hpp"
       #include "mods/DMC3JCE.hpp"
       #include "mods/JCENoMotivationLimit.hpp" // akasha51 https://www.nexusmods.com/devilmaycry5/users/1241088
       #include "mods/TrickDodgeNoDisappear.hpp"
       #include "mods/VergilWalkingGuard.hpp"
       #include "mods/VergilGuardYamatoBlock.hpp"
       #include "mods/AirTrickDodge.hpp"
       #include "mods/VergilNoRoyalForkDelay.hpp"


static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }

Mods::Mods() 
    : m_config{CONFIG_FILENAME}
{
  // Example
        m_mods.emplace_back(std::make_unique<SimpleMod>());
//// Darkness
//    // Background
        m_mods.emplace_back(std::make_unique<FileEditor>());
    // Common
    // Gameplay
    // Nero
    // Dante
    // V
      // Yeah sure :idk:
    // Vergil
// Hitch
    // Background
        m_mods.emplace_back(std::make_unique<PlayerTracker>());
        m_mods.emplace_back(std::make_unique<StyleRank>());
        //m_mods.emplace_back(std::make_unique<SinCoordinate>());
        //m_mods.emplace_back(std::make_unique<CosCoordinate>());
        //m_mods.emplace_back(std::make_unique<MoveID>());
		m_mods.emplace_back(std::make_unique<HoldToMash>()); //has to come before game input??
        m_mods.emplace_back(std::make_unique<TextEditor>());
        m_mods.emplace_back(std::make_unique<GameInput>());
    // Common
        m_mods.emplace_back(std::make_unique<HeavyDay>());
        m_mods.emplace_back(std::make_unique<MoveReplacer>());
        m_mods.emplace_back(std::make_unique<Inertia>());
        m_mods.emplace_back(std::make_unique<LDK>());
        m_mods.emplace_back(std::make_unique<TauntSelector>());
        m_mods.emplace_back(std::make_unique<DisableAutoAssist>());
        m_mods.emplace_back(std::make_unique<DisableTitleTimer>());

        m_mods.emplace_back(std::make_unique<SpardaWorkshop>());
        //m_mods.emplace_back(std::make_unique<SCNPathEditor>());
    // Gameplay
        m_mods.emplace_back(std::make_unique<AllOrNothing>());
		m_mods.emplace_back(std::make_unique<OneHitKill>());
		m_mods.emplace_back(std::make_unique<NoOneTakesDamage>());
        m_mods.emplace_back(std::make_unique<DifficultySelect>());
    // Nero
        m_mods.emplace_back(std::make_unique<BreakerSwitcher>());
        //m_mods.emplace_back(std::make_unique<DisableBreakaway>());
        m_mods.emplace_back(std::make_unique<CaliburExceed>());
        m_mods.emplace_back(std::make_unique<NothingCancelsBubble>());
		m_mods.emplace_back(std::make_unique<LongerRagtimeBubble>());
        m_mods.emplace_back(std::make_unique<ExceedValue>());
        m_mods.emplace_back(std::make_unique<NeroSuperMovesNoDT>());
        m_mods.emplace_back(std::make_unique<DTWingsOnly>());
    // Dante
    // V
        m_mods.emplace_back(std::make_unique <InfiniteSummonPowerup>());
        m_mods.emplace_back(std::make_unique <InfiniteSummonVitality>());
        m_mods.emplace_back(std::make_unique <PetChargeNoInterrupt>());
        m_mods.emplace_back(std::make_unique <InfiniteGambits>());
        m_mods.emplace_back(std::make_unique <EmpoweredCane>());
    // Vergil
        m_mods.emplace_back(std::make_unique<DoppelWeaponSwitcher>());

//// Siyan
    // Background
        m_mods.emplace_back(std::make_unique<DamageTypeLean>());
    // Common
        m_mods.emplace_back(std::make_unique<DeepTurbo>());
        m_mods.emplace_back(std::make_unique<BpStageJump>());
        m_mods.emplace_back(std::make_unique<FreezeBPTimer>());
        m_mods.emplace_back(std::make_unique<NoScreenShake>());
        m_mods.emplace_back(std::make_unique<DisplayEnemyHPInOrbs>());
        m_mods.emplace_back(std::make_unique<MovingTargetSwitch>());
        m_mods.emplace_back(std::make_unique<HUDOptions>());
        m_mods.emplace_back(std::make_unique<CameraSettings>());
        m_mods.emplace_back(std::make_unique<LandCancels>());
        m_mods.emplace_back(std::make_unique<InfHP>());
    // Gameplay
        m_mods.emplace_back(std::make_unique<Reversals>());
        m_mods.emplace_back(std::make_unique<NoJCCooldown>());
        m_mods.emplace_back(std::make_unique<AlwaysSTaunts>());
        m_mods.emplace_back(std::make_unique<AllStart>());
        m_mods.emplace_back(std::make_unique<AllStartManual>());
        m_mods.emplace_back(std::make_unique<JumpStart>());           // Must initialize after AllStart
        m_mods.emplace_back(std::make_unique<DamageMultiplier>());
        m_mods.emplace_back(std::make_unique<PlayerDamageMult>());
        m_mods.emplace_back(std::make_unique<InfDT>());
        m_mods.emplace_back(std::make_unique<EntitySpeeds>());
        m_mods.emplace_back(std::make_unique<FileFrameCuts>());
        m_mods.emplace_back(std::make_unique<SprintInBattle>());
        m_mods.emplace_back(std::make_unique<EnemyStepBanHeight>());
        m_mods.emplace_back(std::make_unique<DisableEnemyAI>());
        m_mods.emplace_back(std::make_unique<EnemyInstantDT>());
        m_mods.emplace_back(std::make_unique<ChargeChecker>());
        m_mods.emplace_back(std::make_unique<WalkOnKeyboard>());
        m_mods.emplace_back(std::make_unique<WeightReset>());
        m_mods.emplace_back(std::make_unique<DisableGauntletStages>());
        m_mods.emplace_back(std::make_unique<NoSlowmoOrHitstop>());
    // Nero
        //m_mods.emplace_back(std::make_unique<NeroInfBreakers>());
        m_mods.emplace_back(std::make_unique<NeroDisableWiresnatch>());
        m_mods.emplace_back(std::make_unique<NeroSwapWiresnatch>());
        m_mods.emplace_back(std::make_unique<NeroTomboyLockOn>());
        m_mods.emplace_back(std::make_unique<NeroSwapSidesteps>());
        m_mods.emplace_back(std::make_unique<NeroInfPunchline>());
        m_mods.emplace_back(std::make_unique<NeroSkipCS2>());
        m_mods.emplace_back(std::make_unique<NoDTCooldown>());
        m_mods.emplace_back(std::make_unique<NeroAlwaysInitialDT>());
        m_mods.emplace_back(std::make_unique<NeroInfCalibur>());
    // Dante
        m_mods.emplace_back(std::make_unique<BypassBPCav>());
        m_mods.emplace_back(std::make_unique<DanteMaxSDT>());
        m_mods.emplace_back(std::make_unique<DanteQuickSDT>());
        m_mods.emplace_back(std::make_unique<DanteAlwaysQ4SDT>());
        m_mods.emplace_back(std::make_unique<DanteInfQ4ExitWindow>());
        m_mods.emplace_back(std::make_unique<DanteTwoTricks>());
        m_mods.emplace_back(std::make_unique<DanteAlwaysOvertop>());
        m_mods.emplace_back(std::make_unique<DanteInfIgnition>());
        m_mods.emplace_back(std::make_unique<DanteGuardflyWip>());
        m_mods.emplace_back(std::make_unique<DanteVariableGuard>());
        m_mods.emplace_back(std::make_unique<AerialPushbackVertical>());
        m_mods.emplace_back(std::make_unique<AerialPushback>());      // init after AerialPushbackVertical
        m_mods.emplace_back(std::make_unique<DanteRedlineCav>());
        m_mods.emplace_back(std::make_unique<DanteSDTRework>());
    // V
    // Vergil
        m_mods.emplace_back(std::make_unique<VergilAirTrickNoSS>());
        m_mods.emplace_back(std::make_unique<VergilDirectionalMappings>());
        m_mods.emplace_back(std::make_unique<VergilInfJdCs>());
        m_mods.emplace_back(std::make_unique<VergilInfSDT>());
        m_mods.emplace_back(std::make_unique<VergilInstantSDT>());
        m_mods.emplace_back(std::make_unique<VergilInfConcentration>());
        m_mods.emplace_back(std::make_unique<VergilNoTrickRestriction>());
        m_mods.emplace_back(std::make_unique<VergilTrickUpLockedOn>());

//// Dr.penguin
    // Background
    // Common
    // Gameplay
    // Nero
        m_mods.emplace_back(std::make_unique<NeroGP01overGerberaPickup>());
    // Dante
    // V
    // Vergil
        m_mods.emplace_back(std::make_unique<VergilDoppelBanish>());

// VPZadov
        // Background
        // Common
        // Gameplay
        m_mods.emplace_back(std::make_unique<MissionManager>());//Must initilize before EmSwapper
        m_mods.emplace_back(std::make_unique<EnemySwapper>());//Must initilize before EnemyDataSettings
        m_mods.emplace_back(std::make_unique<EnemyDataSettings>());
        //m_mods.emplace_back(std::make_unique<EnemyWaveSettings>());
        m_mods.emplace_back(std::make_unique<CheckpointPos>());
        m_mods.emplace_back(std::make_unique<BossDanteSetup>());
        m_mods.emplace_back(std::make_unique<WaveEditorMod::EnemyWaveEditor>());
        m_mods.emplace_back(std::make_unique<SecretMissionTimer>());
        // Nero
        // Dante
        m_mods.emplace_back(std::make_unique<DanteAirTrickSettings>());
        m_mods.emplace_back(std::make_unique<GroundTrickNoDistanceRestriction>());
        m_mods.emplace_back(std::make_unique<DanteNoSdtStun>());
        m_mods.emplace_back(std::make_unique<JudgementCustomCost>());
        m_mods.emplace_back(std::make_unique<DanteDtNoActivationCost>());
        // V
        //Vergil
        m_mods.emplace_back(std::make_unique<VergilSDTFormTracker>());
        m_mods.emplace_back(std::make_unique<VergilNoAfterimages>());//Must initilize after VergilSDTFormTracker
        m_mods.emplace_back(std::make_unique<VergilDisableSDTAccumulate>());
        m_mods.emplace_back(std::make_unique<VergilSetMaxJJC>());
        //m_mods.emplace_back(std::make_unique<VergilAdditionalJJC>());//Must initilize after VergilSetMaxJJC and VergilSDTFormTracker
        m_mods.emplace_back(std::make_unique<VergilSDTAccumulateRework>());
        m_mods.emplace_back(std::make_unique<VergilSDTNoConcentrationLose>());
        m_mods.emplace_back(std::make_unique<VergilAirTrick>());
        //m_mods.emplace_back(std::make_unique<VergilSDTTrickEfx>());//Removed intil better times
        m_mods.emplace_back(std::make_unique<InfiniteTrickUp>());
        m_mods.emplace_back(std::make_unique<DMC3JCE>());
        m_mods.emplace_back(std::make_unique<JCENoMotivationLimit>()); // akasha51 https://www.nexusmods.com/devilmaycry5/users/1241088
        m_mods.emplace_back(std::make_unique<TrickDodgeNoDisappear>());
        m_mods.emplace_back(std::make_unique<VergilWalkingGuard>());
        m_mods.emplace_back(std::make_unique<VergilGuardYamatoBlock>());
        m_mods.emplace_back(std::make_unique<AirTrickDodge>());
        m_mods.emplace_back(std::make_unique<VergilNoRoyalForkDelay>());

#ifdef DEVELOPER
    m_mods.emplace_back(std::make_unique<DeveloperTools>());
#endif
}

std::optional<std::string> Mods::on_initialize(const bool& load_configs) const {
    for (auto& mod : m_mods) {
        spdlog::info("{:s}::on_initialize()", mod->get_name().data());

        if (auto e = mod->on_initialize(); e != std::nullopt) {
            spdlog::info("{:s}::on_initialize() has failed: {:s}", mod->get_name().data(), *e);
            return e;
        }
    }
    if(Mod::patterns->is_changed())
        Mod::patterns->save();
    Mod::patterns->free();

    if (load_configs)
    {
        load_mods();
    }

	m_focused_mod = "";
    return std::nullopt;
}


Mod* Mods::get_mod(std::string modName) const {
  if(modName == "None")
  {
      return nullptr;
  }


  for (auto& mod : m_mods) {
    if (modName == mod->get_name()) {
      return mod.get();
    }
  }

  return nullptr;
}

std::string Mods::get_focused_mod() const {
  return m_focused_mod;
}

void Mods::set_focused_mod(const std::string& modName) const
{
  m_focused_mod = modName;
}

void Mods::on_frame() const {
    for (auto& mod : m_mods) {
        mod->on_frame();
    }
}

void Mods::save_mods() {
    for (auto& mod : m_mods) {
        spdlog::info("{:s}::on_config_save()", mod->get_name().data());
        std::string togglename = std::string(mod->get_name());
        togglename.append("_on");

        if(mod->m_is_enabled){
            m_config.set<bool>(togglename, *mod->m_is_enabled);
        }else{
            m_config.set<bool>(togglename, false);
        }

        mod->on_config_save(m_config);
    }
}


void Mods::load_mods(const std::optional<utility::Config>& cfg) const {
    if(cfg)
    {
        m_config = *cfg;
    }

    for (auto& mod : m_mods) {
        spdlog::info("{:s}::on_config_load()", mod->get_name().data());
        std::string togglename = std::string(mod->get_name());
        togglename.append("_on");

	    if (mod->m_is_enabled) {
	    	*mod->m_is_enabled = m_config.get<bool>(togglename).value_or(false);
	    	mod->on_config_load(m_config);
	    }
    }
}

void Mods::on_draw_debug_ui() const {
	for (auto& mod : m_mods) {
		mod->on_draw_debug_ui();
	}
}

void Mods::on_draw_ui() const {
    for (auto& mod : m_mods) {
        mod->on_draw_ui();
    }
}

void Mods::draw_entry(Mod* mod){

    if(mod == nullptr)
    {
        return;
    }

    const auto window = ImGui::GetCurrentWindow();

    ImGui::Checkbox(mod->get_checkbox_name().c_str(), mod->m_is_enabled);
    ImGui::SameLine();

    if (ImGui::Selectable(mod->m_full_name_string.c_str(), m_focused_mod == mod->get_name(), 0, ImGui::CalcTextSize(mod->m_full_name_string.c_str()))) {
        m_focused_mod = mod->get_name();
    }

    //if (!ImGui::IsWindowFocused())
    //{
    //    return;
    //}

    const ImRect areaOfModName(ImGui::GetItemRectMin(), ImVec2(window->Pos.x + window->Size.x, ImGui::GetItemRectMin().y + ImGui::GetItemRectSize().y));

    const auto mousePos = ImGui::GetMousePos();

    const bool isHovered = mousePos.x >= areaOfModName.Min.x && mousePos.y >= areaOfModName.Min.y && mousePos.x < areaOfModName.Max.x && mousePos.y < areaOfModName.Max.y;

    if (isHovered) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 0.0f));
        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3.0f * g_framework->get_scale());
    	KeyBindButton(mod->m_raw_full_name, std::string(mod->get_name()), g_framework->get_kcw_buffers(), 1.0f, true, UI::BUTTONCOLOR);
		ImGui::PopStyleVar(1);
    }
}


void Mods::on_pagelist_ui(int page, float indent) {
  for (auto& mod : m_mods) {
    if (page == mod->m_on_page) {
      if (indent != 0.f) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
      }
      draw_entry(mod.get());
    }
      //mod->modkeytoggle.draw(mod->get_name());
  }
}

