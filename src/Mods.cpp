
#include <spdlog/spdlog.h>
#include "Mods.hpp"
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
        #include "mods/DontHideWeaponWheel.hpp"
        //#include "mods/SCNPathEditor.hpp"
    // Gameplay
        #include "mods/AllOrNothing.hpp"
        #include "mods/DifficultySelect.hpp"
    // Nero
        #include "mods/BreakerSwitcher.hpp"
        #include "mods/DisableBreakaway.hpp"
        #include "mods/CaliburExceed.hpp"
        #include "mods/NothingCancelsBubble.hpp"
        #include "mods/NeroSuperMovesNoDT.hpp"
        #include "mods/ExceedValue.hpp"
        #include "mods/DTWingsOnly.hpp"
    // Dante
    // V
       #include "mods/InfiniteSummonPowerup.hpp"
       #include "mods/InfiniteSummonVitality.hpp"
       #include "mods/PetChargeNoInterrupt.hpp"
       #include "mods/InfiniteGambits.hpp"
       #include "mods/GilverNoDTCooldown.hpp"
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
        #include "mods/HideHUD.hpp"
        #include "mods/CameraSettings.hpp"
    // Gameplay
        #include "mods/BufferedReversals.hpp"
        #include "mods/BufferedReversalsOriginal.hpp"
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
    // Nero
        #include "mods/NeroInfBreakers.hpp"
        #include "mods/NeroDisableWiresnatch.hpp"
        #include "mods/NeroSwapWiresnatch.hpp"
        #include "mods/NeroTomboyLockOn.hpp"
        #include "mods/NeroSwapSidesteps.hpp"
        #include "mods/NeroInfPunchline.hpp"
        #include "mods/NeroSkipCS2.hpp"
        #include "mods/NeroNoDTCooldown.hpp"
        #include "mods/DTWingsOnly.hpp"
        #include "mods/modNeroAlwaysInitialDT.hpp"
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
       #include "mods/EnemyWaveSettings.hpp"
       #include "mods/CheckpointPos.hpp"
       #include "mods/MissionManager.hpp"
       #include "mods/BossDanteSetup.hpp"
    // Nero
    // Dante
       #include "mods/DanteAirTrickSettings.hpp"
       #include "mods/GroundTrickNoDistanceRestriction.hpp"
    // V
    // Vergil
       #include "mods/VergilSDTFormTracker.hpp"
       #include "mods//VergilNoAfterimages.hpp"
       #include "mods/VergilDisableSDTAccumulate.hpp"
       #include "mods/VergilSetMaxJJC.hpp"
       #include "mods/VergilAdditionalJJC.hpp"
       #include "mods/VergilSDTAccumulateRework.hpp"
       #include "mods/VergilSDTNoConcentrationLose.hpp"
       #include "mods/VergilAirTrick.hpp"
       //#include "mods/VergilSDTTrickEfx.hpp" //Removed intil better times
       #include "mods//InfiniteTrickUp.hpp"
Mods::Mods() 
    : redrawfocusedwindow{ false }, m_config{"DMC2_fw_config.txt"} {
  // Example
        m_mods.emplace_back(std::make_unique<SimpleMod>());
// Darkness
    // Background
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
        m_mods.emplace_back(std::make_unique<HoldToMash>());
        m_mods.emplace_back(std::make_unique<SpardaWorkshop>());
        m_mods.emplace_back(std::make_unique<DontHideWeaponWheel>());
        //m_mods.emplace_back(std::make_unique<SCNPathEditor>());
    // Gameplay
        m_mods.emplace_back(std::make_unique<AllOrNothing>());
        m_mods.emplace_back(std::make_unique<DifficultySelect>());
    // Nero
        m_mods.emplace_back(std::make_unique<BreakerSwitcher>());
        m_mods.emplace_back(std::make_unique<DisableBreakaway>());
        m_mods.emplace_back(std::make_unique<CaliburExceed>());
        m_mods.emplace_back(std::make_unique<NothingCancelsBubble>());
        m_mods.emplace_back(std::make_unique<ExceedValue>());
        m_mods.emplace_back(std::make_unique<NeroSuperMovesNoDT>());
        m_mods.emplace_back(std::make_unique<DTWingsOnly>());
    // Dante
    // V
        m_mods.emplace_back(std::make_unique <InfiniteSummonPowerup>());
        m_mods.emplace_back(std::make_unique <InfiniteSummonVitality>());
        m_mods.emplace_back(std::make_unique <PetChargeNoInterrupt>());
        m_mods.emplace_back(std::make_unique <InfiniteGambits>());
        m_mods.emplace_back(std::make_unique <GilverNoDTCooldown>());
        m_mods.emplace_back(std::make_unique <EmpoweredCane>());
    // Vergil
        m_mods.emplace_back(std::make_unique<DoppelWeaponSwitcher>());

// Siyan
    // Background
        m_mods.emplace_back(std::make_unique<DamageTypeLean>());
    // Common
        m_mods.emplace_back(std::make_unique<DeepTurbo>());
        m_mods.emplace_back(std::make_unique<BpStageJump>());
        m_mods.emplace_back(std::make_unique<FreezeBPTimer>());
        m_mods.emplace_back(std::make_unique<NoScreenShake>());
        m_mods.emplace_back(std::make_unique<DisplayEnemyHPInOrbs>());
        m_mods.emplace_back(std::make_unique<MovingTargetSwitch>());
        m_mods.emplace_back(std::make_unique<HideHUD>());
        m_mods.emplace_back(std::make_unique<CameraSettings>());
    // Gameplay
        m_mods.emplace_back(std::make_unique<BufferedReversals>());
        m_mods.emplace_back(std::make_unique<OriginalReversals>());
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
    // Nero
        m_mods.emplace_back(std::make_unique<NeroInfBreakers>());
        m_mods.emplace_back(std::make_unique<NeroDisableWiresnatch>());
        m_mods.emplace_back(std::make_unique<NeroSwapWiresnatch>());
        m_mods.emplace_back(std::make_unique<NeroTomboyLockOn>());
        m_mods.emplace_back(std::make_unique<NeroSwapSidesteps>());
        m_mods.emplace_back(std::make_unique<NeroInfPunchline>());
        m_mods.emplace_back(std::make_unique<NeroSkipCS2>());
        m_mods.emplace_back(std::make_unique<NeroNoDTCooldown>());
        m_mods.emplace_back(std::make_unique<NeroAlwaysInitialDT>());
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

// Dr.penguin
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
        m_mods.emplace_back(std::make_unique<BossDanteSetup>());
        m_mods.emplace_back(std::make_unique<MissionManager>());//Must initilize before EmSwapper
        m_mods.emplace_back(std::make_unique<EnemySwapper>());//Must initilize before EnemyDataSettings
        m_mods.emplace_back(std::make_unique<EnemyDataSettings>());
        m_mods.emplace_back(std::make_unique<EnemyWaveSettings>());
        m_mods.emplace_back(std::make_unique<CheckpointPos>());

        // Nero
        // Dante
        m_mods.emplace_back(std::make_unique<DanteAirTrickSettings>());
        m_mods.emplace_back(std::make_unique<GroundTrickNoDistanceRestriction>());
        // V
        //Vergil
        m_mods.emplace_back(std::make_unique<VergilSDTFormTracker>());
        m_mods.emplace_back(std::make_unique<VergilNoAfterimages>());//Must initilize after VergilSDTFormTracker
        m_mods.emplace_back(std::make_unique<VergilDisableSDTAccumulate>());
        m_mods.emplace_back(std::make_unique<VergilSetMaxJJC>());
        m_mods.emplace_back(std::make_unique<VergilAdditionalJJC>());//Must initilize after VergilSetMaxJJC and VergilSDTFormTracker
        m_mods.emplace_back(std::make_unique<VergilSDTAccumulateRework>());
        m_mods.emplace_back(std::make_unique<VergilSDTNoConcentrationLose>());
        m_mods.emplace_back(std::make_unique<VergilAirTrick>());
        //m_mods.emplace_back(std::make_unique<VergilSDTTrickEfx>());//Removed intil better times
        m_mods.emplace_back(std::make_unique<InfiniteTrickUp>());
#ifdef DEVELOPER
    m_mods.emplace_back(std::make_unique<DeveloperTools>());
#endif
}

std::optional<std::string> Mods::on_initialize() const {
    for (auto& mod : m_mods) {
        spdlog::info("{:s}::on_initialize()", mod->get_name().data());

        if (auto e = mod->on_initialize(); e != std::nullopt) {
            spdlog::info("{:s}::on_initialize() has failed: {:s}", mod->get_name().data(), *e);
            return e;
        }
    }

    /*utility::Config m_config{ "re2_fw_config.txt" };

    for (auto& mod : m_mods) {
        spdlog::info("{:s}::on_config_load()", mod->get_name().data());
        mod->on_config_load(m_config);
    }
    */
    load_mods();
    //this is still very not great
    focusedmod = "nomod";
    return std::nullopt;
}


const std::unique_ptr<Mod>& Mods::get_mod(std::string modname) const {
    //recursive call in case we can't find the mod being looked for
  for (auto& mod : m_mods) {
    if (modname == mod->get_name()) {
      return mod;
    }
  }
  return get_mod("SimpleMod");
}

std::string Mods::get_focused_mod() const {
  return focusedmod;
}

void Mods::on_frame() const {
    for (auto& mod : m_mods) {
        mod->on_frame();
    }
}

void Mods::save_mods() {
    for (auto& mod : m_mods) {
        spdlog::info("{:s}::on_config_save()", mod->get_name().data());
        std::string togglename = std::string{mod->get_name()};
        togglename.append("_on");
        if(mod->ischecked){
            m_config.set<bool>(togglename, *mod->ischecked);
        }else{
            m_config.set<bool>(togglename, false);
        }
        mod->on_config_save(m_config);
        //and then probably call the rest of the stuff here;
    }
    // dorime
    //namespace fs = std::filesystem;
    //std::filesystem::path mypath = fs::current_path() / "DMC2_fw_config.txt" ;
    //auto m_conf_path             = mypath.string();
    // ameno
    m_config.save();
}


void Mods::load_mods() const {
  for (auto& mod : m_mods) {
    spdlog::info("{:s}::on_config_load()", mod->get_name().data());
    std::string togglename = std::string{mod->get_name()};
    togglename.append("_on");
	if (mod->ischecked) {
		*(mod->ischecked) = m_config.get<bool>(togglename).value_or(false);
		mod->on_config_load(m_config);
	}
    // and then probably call the rest of the stuff here;
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

void Mods::draw_entry(std::unique_ptr<Mod>& mod){
    //mod->get_hotkey_name()
    ImGui::Checkbox(mod->get_checkbox_name().c_str(), mod->ischecked);
    ImGui::SameLine();
    if (ImGui::Selectable(mod->full_name_string.c_str(), focusedmod == mod->get_name())) {
        focusedmod = mod->get_name();
    }
}


void Mods::on_pagelist_ui(int page){
  for (auto& mod : m_mods) {
    if (page == mod->onpage) {
        draw_entry(mod);
      }
      //mod->modkeytoggle.draw(mod->get_name());
  }
}

