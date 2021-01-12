#include <spdlog/spdlog.h>
#include "Mods.hpp"

// Example
        // #include "mods/SimpleMod.hpp"

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
        #include "mods/BypassBPCav.hpp"
        #include "mods/FreezeBPTimer.hpp"
        #include "mods/NoScreenShake.hpp"
        #include "mods/BufferedReversals.hpp"
        #include "mods/DamageMultiplier.hpp"
        #include "mods/DisplayEnemyHPInOrbs.hpp"
        #include "mods/FileFrameCuts.hpp"
        #include "mods/EntitySpeeds.hpp"
        #include "mods/MovingTargetSwitch.hpp"
    // Nero

    // Dante
        #include "mods/DanteMaxSDT.hpp"
        #include "mods/DanteQuickSDT.hpp"
        #include "mods/DanteAlwaysQ4SDT.hpp"
        #include "mods/DanteInfQ4ExitWindow.hpp"
    // V
    // Vergil
        #include "mods/VergilInfConcentration.hpp"
        #include "mods/VergilAirTrickNoSS.hpp"
        #include "mods/VergilInfSDT.hpp"
        #include "mods/VergilDirectionalMappings.hpp"
        #include "mods/VergilInstantSDT.hpp"
        #include "mods/VergilInfJdCs.hpp"

Mods::Mods()
{
// Example
        // m_mods.emplace_back(std::make_unique<SimpleMod>());

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
        m_mods.emplace_back(std::make_unique<BypassBPCav>());
        m_mods.emplace_back(std::make_unique<FreezeBPTimer>());
        m_mods.emplace_back(std::make_unique<NoScreenShake>());
        m_mods.emplace_back(std::make_unique<BufferedReversals>());

        m_mods.emplace_back(std::make_unique<DamageMultiplier>());
        m_mods.emplace_back(std::make_unique<DisplayEnemyHPInOrbs>());
        m_mods.emplace_back(std::make_unique<FileFrameCuts>());
        m_mods.emplace_back(std::make_unique<EntitySpeeds>());
        m_mods.emplace_back(std::make_unique<MovingTargetSwitch>());
        
    // Nero
    // Dante
        m_mods.emplace_back(std::make_unique<DanteMaxSDT>());
        m_mods.emplace_back(std::make_unique<DanteQuickSDT>());
        m_mods.emplace_back(std::make_unique<DanteAlwaysQ4SDT>());
        m_mods.emplace_back(std::make_unique<DanteInfQ4ExitWindow>());
    // V
    // Vergil
        m_mods.emplace_back(std::make_unique<VergilInfConcentration>());
        m_mods.emplace_back(std::make_unique<VergilAirTrickNoSS>());
        m_mods.emplace_back(std::make_unique<VergilInfSDT>());
        m_mods.emplace_back(std::make_unique<VergilDirectionalMappings>());
        m_mods.emplace_back(std::make_unique<VergilInstantSDT>());
        m_mods.emplace_back(std::make_unique<VergilInfJdCs>());

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

    utility::Config cfg{ "re2_fw_config.txt" };

    for (auto& mod : m_mods) {
        spdlog::info("{:s}::on_config_load()", mod->get_name().data());
        mod->on_config_load(cfg);
    }

    return std::nullopt;
}

void Mods::on_frame() const {
    for (auto& mod : m_mods) {
        mod->on_frame();
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

void Mods::on_pagelist_ui(int page) const {
  for (auto& mod : m_mods) {
    //std::string_view hiddenname = "##" + mod->get_name();
    if (page == mod->onpage) {
      ImGui::Checkbox(std::string{mod->get_name()}.c_str(), &mod->ischecked);
    }
  }
}

