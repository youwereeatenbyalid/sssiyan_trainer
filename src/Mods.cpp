
#include <spdlog/spdlog.h>
#include "Mods.hpp"
// Example
         #include "mods/SimpleMod.hpp"

// Hitch
    // Game
        #include "mods/PlayerTracker.hpp"
        #include "mods/StyleRank.hpp"
        #include "mods/SinCoordinate.hpp"
        #include "mods/CosCoordinate.hpp"
        #include "mods/MoveID.hpp"
        #include "mods/EnemyNumber.hpp"
    // Nero
    // Dante
    // V
    // Vergil

// Siyan
    // Game
        #include "mods/DeepTurbo.hpp"
        #include "mods/AlwaysSTaunts.hpp"
        #include "mods/NoJCCooldown.hpp"
        #include "mods/FreezeBPTimer.hpp"
        #include "mods/NoScreenShake.hpp"
        #include "mods/BufferedReversals.hpp"
        #include "mods/DamageMultiplier.hpp"
        #include "mods/InfDT.hpp"
        #include "mods/DisplayEnemyHPInOrbs.hpp"
        #include "mods/FileFrameCuts.hpp"
        #include "mods/EntitySpeeds.hpp"
        #include "mods/MovingTargetSwitch.hpp"
        #include "mods/DisableEnemyAI.hpp"
        #include "mods/HideHUD.hpp"
        #include "mods/CameraSettings.hpp"
    // Nero
        #include "mods/NeroDisableWiresnatch.hpp"
    // Dante
        #include "mods/BypassBPCav.hpp"
        #include "mods/DanteMaxSDT.hpp"
        #include "mods/DanteQuickSDT.hpp"
        #include "mods/DanteAlwaysQ4SDT.hpp"
        #include "mods/DanteInfQ4ExitWindow.hpp"
    // V
    // Vergil
        #include "mods/VergilAirTrickNoSS.hpp"
        #include "mods/VergilDirectionalMappings.hpp"
        #include "mods/VergilInfJdCs.hpp"
        #include "mods/VergilInfSDT.hpp"
        #include "mods/VergilInstantSDT.hpp"
        #include "mods/VergilInfConcentration.hpp"

Mods::Mods()
{
// Example
        m_mods.emplace_back(std::make_unique<SimpleMod>());
// Hitch
    // Game
        m_mods.emplace_back(std::make_unique<PlayerTracker>());
        m_mods.emplace_back(std::make_unique<StyleRank>());
        m_mods.emplace_back(std::make_unique<SinCoordinate>());
        m_mods.emplace_back(std::make_unique<CosCoordinate>());
        m_mods.emplace_back(std::make_unique<MoveID>());
        m_mods.emplace_back(std::make_unique<EnemyNumber>());
    // Nero
    // Dante
    // V
    // Vergil

// Siyan
    // Game
        m_mods.emplace_back(std::make_unique<DeepTurbo>());
        m_mods.emplace_back(std::make_unique<AlwaysSTaunts>());
        m_mods.emplace_back(std::make_unique<NoJCCooldown>());
        m_mods.emplace_back(std::make_unique<FreezeBPTimer>());
        m_mods.emplace_back(std::make_unique<NoScreenShake>());
        m_mods.emplace_back(std::make_unique<BufferedReversals>());
        m_mods.emplace_back(std::make_unique<DamageMultiplier>());
        m_mods.emplace_back(std::make_unique<InfDT>());
        m_mods.emplace_back(std::make_unique<DisplayEnemyHPInOrbs>());
        m_mods.emplace_back(std::make_unique<FileFrameCuts>());
        m_mods.emplace_back(std::make_unique<EntitySpeeds>());
        m_mods.emplace_back(std::make_unique<MovingTargetSwitch>());
        m_mods.emplace_back(std::make_unique<DisableEnemyAI>());
        m_mods.emplace_back(std::make_unique<HideHUD>());
        m_mods.emplace_back(std::make_unique<CameraSettings>());
    // Nero
        m_mods.emplace_back(std::make_unique<NeroDisableWiresnatch>());
    // Dante
        m_mods.emplace_back(std::make_unique<BypassBPCav>());
        m_mods.emplace_back(std::make_unique<DanteMaxSDT>());
        m_mods.emplace_back(std::make_unique<DanteQuickSDT>());
        m_mods.emplace_back(std::make_unique<DanteAlwaysQ4SDT>());
        m_mods.emplace_back(std::make_unique<DanteInfQ4ExitWindow>());
    // V
    // Vergil
        m_mods.emplace_back(std::make_unique<VergilAirTrickNoSS>());
        m_mods.emplace_back(std::make_unique<VergilDirectionalMappings>());
        m_mods.emplace_back(std::make_unique<VergilInfJdCs>());
        m_mods.emplace_back(std::make_unique<VergilInfSDT>());
        m_mods.emplace_back(std::make_unique<VergilInstantSDT>());
        m_mods.emplace_back(std::make_unique<VergilInfConcentration>());

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

    /*utility::Config cfg{ "re2_fw_config.txt" };

    for (auto& mod : m_mods) {
        spdlog::info("{:s}::on_config_load()", mod->get_name().data());
        mod->on_config_load(cfg);
    }
    */
    load_mods();
    //this is still very not great
    focusedmod = "nomod";
    return std::nullopt;
}
std::shared_ptr<Mod> Mods::get_mod(std::string modname) const {
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
void Mods::save_mods() const {
  utility::Config cfg{"DMC2_fw_config.txt"};
    for (auto& mod : m_mods) {
        spdlog::info("{:s}::on_config_save()", mod->get_name().data());
        std::string togglename = std::string{mod->get_name()};
        togglename.append("_on");
        cfg.set<bool>(togglename, mod->ischecked);
        mod->on_config_save(cfg);
        //and then probably call the rest of the stuff here;
    }
    // dorime
    //namespace fs = std::filesystem;
    //std::filesystem::path mypath = fs::current_path() / "DMC2_fw_config.txt" ;
    //auto m_conf_path             = mypath.string();
    // ameno
    cfg.save("DMC2_fw_config.txt");

}


void Mods::load_mods() const {
  utility::Config cfg{"DMC2_fw_config.txt"};
  for (auto& mod : m_mods) {
    spdlog::info("{:s}::on_config_load()", mod->get_name().data());
    std::string togglename = std::string{mod->get_name()};
    togglename.append("_on");
    mod->ischecked = cfg.get<bool>(togglename).value_or(false);
    mod->on_config_load(cfg);
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



void Mods::on_pagelist_ui(int page) const{
  for (auto& mod : m_mods) {
    std::string checkboxname = "##";
    std::string hotkeyname   = "key ";
    checkboxname.append(std::string{mod->get_name()});
    hotkeyname.append(std::string{mod->get_name()});
    if (page == mod->onpage) {
      ImGui::Checkbox(checkboxname.c_str(), &mod->ischecked);
      ImGui::SameLine();
      if (ImGui::Selectable(mod->full_name_string.c_str(), focusedmod == mod->get_name())) {
        focusedmod = mod->get_name();
      }
      //mod->modkeytoggle.draw(mod->get_name());
    }
  }
}

