set(MODS_SRC
    mods/SimpleMod.hpp
    mods/SimpleMod.cpp
    mods/PlayerTracker.hpp 
    mods/PlayerTracker.cpp
    mods/GameInput.hpp 
    mods/GameInput.cpp 
    mods/HeavyDay.hpp 
    mods/HeavyDay.cpp 
    mods/StyleRank.hpp 
    mods/StyleRank.cpp 
    mods/SinCoordinate.hpp 
    mods/SinCoordinate.cpp 
    mods/CosCoordinate.hpp 
    mods/CosCoordinate.cpp 
    mods/LDK.hpp 
    mods/LDK.cpp 
    mods/DeepTurbo.hpp 
    mods/DeepTurbo.cpp 
    mods/BypassBPCav.hpp 
    mods/BypassBPCav.cpp 
    mods/DanteMaxSDT.hpp 
    mods/DanteMaxSDT.cpp 
    mods/FreezeBPTimer.hpp 
    mods/FreezeBPTimer.cpp 
    mods/NoScreenShake.hpp 
    mods/NoScreenShake.cpp 
    mods/DanteQuickSDT.hpp 
    mods/DanteQuickSDT.cpp 
    mods/DanteAlwaysQ4SDT.hpp 
    mods/DanteAlwaysQ4SDT.cpp 
    mods/DanteInfQ4ExitWindow.hpp 
    mods/DanteInfQ4ExitWindow.cpp 
    mods/VergilInfConcentration.hpp 
    mods/VergilInfConcentration.cpp 
    mods/VergilAirTrickNoSS.hpp 
    mods/VergilAirTrickNoSS.cpp 
    mods/VergilInfSDT.hpp 
    mods/VergilInfSDT.cpp 
    mods/VergilDirectionalMappings.hpp 
    mods/VergilDirectionalMappings.cpp 
    mods/DamageMultiplier.hpp 
    mods/DamageMultiplier.cpp 
    mods/DisplayEnemyHPInOrbs.hpp 
    mods/DisplayEnemyHPInOrbs.cpp 
    mods/VergilInstantSDT.hpp 
    mods/VergilInstantSDT.cpp 
    mods/FileFrameCuts.hpp 
    mods/FileFrameCuts.cpp 
    mods/EntitySpeeds.hpp 
    mods/EntitySpeeds.cpp 
    mods/MovingTargetSwitch.hpp 
    mods/MovingTargetSwitch.cpp 
    mods/VergilInfJdCs.hpp 
    mods/VergilInfJdCs.cpp 
    mods/NeroDisableWiresnatch.hpp 
    mods/NeroDisableWiresnatch.cpp 
    mods/NeroInfBreakers.hpp 
    mods/NeroInfBreakers.cpp 
    mods/NoJCCooldown.hpp 
    mods/NoJCCooldown.cpp 
    mods/DisableEnemyAI.hpp 
    mods/DisableEnemyAI.cpp 
    mods/HUDOptions.hpp 
    mods/HUDOptions.cpp 
    mods/CameraSettings.hpp 
    mods/CameraSettings.cpp 
    mods/AlwaysSTaunts.hpp 
    mods/AlwaysSTaunts.cpp 
    mods/InfDT.hpp 
    mods/InfDT.cpp 
    mods/DamageTypeLean.hpp 
    mods/DamageTypeLean.cpp 
    mods/NeroSwapWiresnatch.hpp 
    mods/NeroSwapWiresnatch.cpp 
    mods/DanteTwoTricks.hpp 
    mods/DanteTwoTricks.cpp 
    mods/DanteAlwaysOvertop.hpp 
    mods/DanteAlwaysOvertop.cpp 
    mods/DanteInfIgnition.hpp 
    mods/DanteInfIgnition.cpp 
    mods/VergilNoTrickRestriction.hpp 
    mods/VergilNoTrickRestriction.cpp 
    mods/AllStart.hpp 
    mods/AllStart.cpp 
    mods/JumpStart.hpp 
    mods/JumpStart.cpp 
    mods/AllOrNothing.hpp 
    mods/AllOrNothing.cpp 
    mods/BreakerSwitcher.hpp 
    mods/BreakerSwitcher.cpp
    mods/DisableBreakaway.hpp 
    mods/DisableBreakaway.cpp 
    mods/MoveReplacer.hpp 
    mods/MoveReplacer.cpp 
    mods/Inertia.hpp 
    mods/Inertia.cpp 
    mods/BpStageJump.hpp 
    mods/BpStageJump.cpp 
    mods/NeroTomboyLockOn.hpp 
    mods/NeroTomboyLockOn.cpp 
    mods/SprintInBattle.hpp 
    mods/SprintInBattle.cpp 
    mods/NeroSwapSidesteps.hpp 
    mods/NeroSwapSidesteps.cpp 
    mods/NeroInfPunchline.hpp 
    mods/NeroInfPunchline.cpp 
    mods/NeroSkipCS2.hpp 
    mods/NeroSkipCS2.cpp 
    mods/NoDTCooldown.hpp 
    mods/NoDTCooldown.cpp 
    mods/PlayerDamageMult.hpp 
    mods/PlayerDamageMult.cpp 
    mods/TauntSelector.hpp 
    mods/TauntSelector.cpp 
    mods/DanteGuardflyWip.hpp 
    mods/DanteGuardflyWip.cpp 
    mods/VergilTrickUpLockedOn.hpp 
    mods/VergilTrickUpLockedOn.cpp 
    mods/DoppelWeaponSwitcher.hpp 
    mods/DoppelWeaponSwitcher.cpp 
    mods/DisableAutoAssist.hpp 
    mods/DisableAutoAssist.cpp 
    mods/DisableTitleTimer.hpp 
    mods/DisableTitleTimer.cpp 
    mods/EnemyStepBanHeight.hpp 
    mods/EnemyStepBanHeight.cpp 
    mods/SpardaWorkshop.hpp 
    mods/SpardaWorkshop.cpp 
    mods/NeroGP01overGerberaPickup.hpp 
    mods/NeroGP01overGerberaPickup.cpp 
    mods/SCNPathEditor.hpp 
    mods/SCNPathEditor.cpp 
    mods/TextEditor.hpp
    mods/TextEditor.cpp
    mods/WalkOnKeyboard.hpp 
    mods/WalkOnKeyboard.cpp 
    mods/Reversals.hpp 
    mods/Reversals.cpp 
    mods/EnemyInstantDT.hpp 
    mods/EnemyInstantDT.cpp 
    mods/ChargeChecker.hpp 
    mods/ChargeChecker.cpp     
    mods/VergilDoppelBanish.hpp 
    mods/VergilDoppelBanish.cpp 
    mods/FileEditor.hpp 
    mods/FileEditor.cpp 
    mods/AllStartManual.hpp 
    mods/AllStartManual.cpp 
    mods/DanteVariableGuard.hpp 
    mods/DanteVariableGuard.cpp 
    mods/CaliburExceed.hpp 
    mods/CaliburExceed.cpp
    mods/ExceedValue.hpp 
    mods/ExceedValue.cpp
    mods/DTWingsOnly.hpp 
    mods/DTWingsOnly.cpp
    mods/NothingCancelsBubble.hpp 
    mods/NothingCancelsBubble.cpp
    mods/NeroSuperMovesNoDT.hpp 
    mods/NeroSuperMovesNoDT.cpp
    mods/WeightReset.hpp 
    mods/WeightReset.cpp 
    mods/InfiniteSummonPowerup.hpp 
    mods/InfiniteSummonPowerup.cpp
    mods/EmpoweredCane.hpp 
    mods/EmpoweredCane.cpp 
    mods/InfiniteSummonVitality.hpp 
    mods/InfiniteSummonVitality.cpp
	mods/InfiniteGambits.hpp
	mods/InfiniteGambits.cpp
	mods/PetChargeNoInterrupt.hpp
	mods/PetChargeNoInterrupt.cpp
    mods/VergilNoAfterimages.hpp
    mods/VergilNoAfterimages.cpp
    mods/VergilSDTFormTracker.hpp
    mods/VergilSdtFormTracker.cpp
    mods/VergilDisableSDTAccumulate.hpp
    mods/VergilDisableSDTAccumulate.cpp
    mods/VergilSetMaxJJC.hpp
    mods/VergilSetMaxJJC.cpp
    mods/VergilAdditionalJJC.hpp
    mods/VergilSDTAccumulateRework.cpp
    mods/VergilSDTAccumulateRework.hpp
	mods/DifficultySelect.hpp
	mods/DifficultySelect.cpp
    mods/EnemySwapper.hpp
    mods/EnemySwapper.cpp
    mods/EnemyDataSettings.hpp
    mods/EnemyWaveSettings.hpp
    mods/EnemyWaveSettings.cpp
	mods/HoldToMash.hpp
	mods/HoldToMash.cpp
    mods/AerialPushback.hpp 
    mods/AerialPushback.cpp 
    mods/AerialPushbackVertical.hpp 
    mods/AerialPushbackVertical.cpp 
    mods/modNeroAlwaysInitialDT.hpp 
    mods/modNeroAlwaysInitialDT.cpp 
    mods/DisableGauntletStages.hpp 
    mods/DisableGauntletStages.cpp
    mods/VergilSDTNoConcentrationLose.hpp
    mods/VergilSDTNoConcentrationLose.cpp
    mods/VergilAirTrick.hpp
    mods/VergilAirTrick.cpp
    mods/VergilSDTTrickEfx.hpp
    mods/VergilSDTTrickEfx.cpp
    mods/DanteAirTrickSettings.hpp
    mods/DanteAirTrickSettings.cpp
    mods/GroundTrickNoDistanceRestriction.hpp
    mods/GroundTrickNoDistanceRestriction.cpp
    mods/InfiniteTrickUp.cpp
    mods/InfiniteTrickUp.hpp
    mods/CheckpointPos.hpp
    mods/CheckpointPos.cpp
    mods/MissionManager.cpp
    mods/MissionManager.hpp
    mods/EnemyWaveEditor.hpp
    mods/EnemyWaveEditor.cpp
    mods/SecretMissionTimer.hpp
    mods/SecretMissionTimer.cpp
    mods/DMC3JCE.hpp
    mods/DMC3JCE.cpp
    mods/JCENoMotivationLimit.hpp
    mods/JCENoMotivationLimit.cpp
    mods/BossDanteSetup.hpp
    mods/BossDanteSetup.cpp
    mods/LandCancels.hpp 
    mods/LandCancels.cpp 
    mods/TrickDodgeNoDisappear.hpp
    mods/TrickDodgeNoDisappear.cpp
    mods/VergilWalkingGuard.hpp
    mods/VergilWalkingGuard.cpp
    mods/VergilGuardYamatoBlock.hpp
    mods/VergilGuardYamatoBlock.cpp
    mods/AirTrickDodge.hpp
    mods/AirTrickDodge.cpp
    mods/DanteNoSdtStun.hpp
    mods/DanteNoSdtStun.cpp
    mods/DanteRedlineCav.hpp 
    mods/DanteRedlineCav.cpp 
    mods/DanteSDTRework.hpp 
    mods/DanteSDTRework.cpp 
    mods/NeroInfCalibur.hpp 
    mods/NeroInfCalibur.cpp 
    mods/JudgementCustomCost.hpp
    mods/OneHitKill.hpp 
    mods/OneHitKill.cpp
    mods/NoOneTakesDamage.hpp 
    mods/NoOneTakesDamage.cpp 
    mods/DanteDtNoActivationCost.hpp
    mods/NoSlowmoOrHitstop.hpp 
    mods/NoSlowmoOrHitstop.cpp 
    mods/InfHP.hpp 
    mods/InfHP.cpp 
    mods/VergilNoRoyalForkDelay.hpp
    mods/LongerRagtimeBubble.hpp 
    mods/LongerRagtimeBubble.cpp 
    mods/DamageType.hpp 
    mods/DamageType.cpp
    mods/VergilDoppelInitSetup.hpp
    mods/InstantDoppel.hpp
    mods/AprilFools.hpp
    mods/AprilFools.cpp
    mods/VergilTrickTrailsEfx.hpp
    mods/VergilTrickTrailsEfx.cpp
    mods/AirMoves.hpp
    mods/AirMoves.cpp
    mods/PlSetActionData.hpp
    mods/EndLvlHooks.hpp
    mods/EndLvlHooks.cpp
    mods/BossTrickUp.hpp
    mods/VergilSDTAlwaysCancels.hpp
    mods/GameplayStateTracker.hpp
    mods/GameplayStateTracker.cpp
    mods/DoppelNoComeBack.hpp
    mods/DanteSDTRegen.hpp
    mods/PosActionEditor.hpp
    mods/InputSystem.hpp
    mods/DanteSelectReleaseType.hpp
    mods/NoRoundtripCallback.hpp
    mods/EnemySpawner.hpp
    mods/EnemySpawner.cpp
    mods/BossVergilMoves.hpp
    mods/Pl0300ControllerManager.hpp
    mods/Pl0300ControllerManager.cpp
    mods/PlController.hpp
    mods/Pl0300Controller.hpp
    mods/Pl0300Controller.cpp
    mods/EnemyFixes.hpp
    mods/VergilGuardSlowMotion.hpp
    mods/BossVergilSettings.hpp
    mods/LockOnNoHold.hpp
    mods/Pl0000SlowWorldStop.hpp
    mods/QuickSilverController.hpp
    mods/QuickSilverController.cpp
    mods/PlayerQuicksilver.hpp
    mods/DanteQuickSilver.hpp
    mods/VergilQuickSilver.hpp
    mods/DanteAirMustang.hpp
    mods/WitchTime.hpp
    mods/ParryWithFinesse.hpp
)

set(HOOKS_SRC
	ControllerHook.hpp
	ControllerHook.cpp
    DInputHook.hpp
    DInputHook.cpp
    D3D11Hook.hpp
    D3D11Hook.cpp
    D3D12Hook.hpp
    D3D12Hook.cpp
    WindowsMessageHook.hpp
    WindowsMessageHook.cpp
    PositionHooks.hpp
    PositionHooks.cpp
	XInputHook.hpp
	XInputHook.cpp
)

set(IMGUI_EXTENSIONS_SRC
    ImGuiExtensions/ImGuiExtensions.h
    ImGuiExtensions/ImGuiExtensions.cpp
)

set(EVENTS_SRC
    events/Events.hpp
    events/EventArgs.hpp
    events/EventHandler.hpp
)

set(SDK_SRC
    sdk/DMC5.hpp
    sdk/Enums_Internal.hpp
    sdk/Offsets.hpp
	sdk/SDKHelpers.hpp
	sdk/SDKHelpers.cpp
)

set(UTILITY_SRC
    utility/Config.hpp
    utility/Config.cpp
    utility/FunctionHook.hpp
    utility/FunctionHook.cpp
)

set(FRAMEWORK_SRC
    Mod.hpp
    Mods.hpp
    Mods.cpp
    ModFramework.hpp
    ModFramework.cpp
    Config.hpp
	stb_image.h
	stb_image_resize.h
	KeyBinder.hpp
	KeyBinder.cpp
	logo.hpp
	icons.hpp
    InitPatternsManager.hpp
	ExceptionHandler.hpp
	ExceptionHandler.cpp
)

set(LISTEDITOR_SRC
    ListEditor/MimicList.hpp
)

set(COROUTINE_SRC
    Mods/Coroutine/Coroutines.hpp
)

set(PREFAB_FACTORY_SRC
    Mods/PrefabFactory/PrefabFactory.hpp
)

set(GAME_FUNC_SRC
    Mods/GameFunctions/GameFunc.hpp
    Mods/GameFunctions/PrefabInstantiate.hpp
    Mods/GameFunctions/CreateShell.hpp
    Mods/GameFunctions/PositionController.hpp
    Mods/GameFunctions/PlayerCheckNormalJump.hpp
    Mods/GameFunctions/RequestPlCameraShake.hpp
    Mods/GameFunctions/PlayerSetDT.hpp
    Mods/GameFunctions/PlVergilDoppel.hpp
    Mods/GameFunctions/GameModelSetAction.hpp
    Mods/GameFunctions/GameModelRequestSetEffect.hpp
)

set(IMGUI_SRC
	fw-imgui/font_robotomedium.hpp
    fw-imgui/fa_solid_900.h
    fw-imgui/font_awesome_5.h
    fw-imgui/imgui_impl_win32.h
    fw-imgui/imgui_impl_win32.cpp
    fw-imgui/imgui_impl_dx11.h
    fw-imgui/imgui_impl_dx11.cpp
    fw-imgui/imgui_impl_dx12.h
    fw-imgui/imgui_impl_dx12.cpp
    fw-imgui/imgui_notify.h
    fw-imgui/ui.cpp
    fw-imgui/ui.hpp
)

source_group("Hooks" FILES ${HOOKS_SRC})
source_group("ImGuiExtensions" FILES ${IMGUI_EXTENSIONS_SRC})
source_group("Mods" FILES ${MODS_SRC})
source_group("ListEditor" FILES ${LISTEDITOR_SRC})
source_group("Events" FILES ${EVENTS_SRC})
source_group("Mods/GameFunctions" FILES ${GAME_FUNC_SRC})
source_group("Mods/Coroutine" FILES ${COROUTINE_SRC})
source_group("Mods/PrefabFactory" FILES ${PREFAB_FACTORY_SRC})
source_group("SDK" FILES ${SDK_SRC})
source_group("Utility" FILES ${UTILITY_SRC})
source_group("Framework" FILES ${FRAMEWORK_SRC})
source_group("FW-ImGui" FILES ${IMGUI_SRC})

add_library(${CMAKE_PROJECT_NAME} SHARED
            Main.cpp
            LicenseStrings.hpp
            EnemyData.hpp
            EnemyData.cpp
            ${SDK_SRC}
            ${IMGUI_EXTENSIONS_SRC}
            ${MODS_SRC}
            ${GAME_FUNC_SRC}
            ${LISTEDITOR_SRC}
            ${COROUTINE_SRC}
            ${PREFAB_FACTORY_SRC}
            ${EVENTS_SRC}
            ${HOOKS_SRC}
            ${IMGUI_SRC}
            ${UTILITY_SRC}
            ${FRAMEWORK_SRC}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC DMC5SDK imgui minhook sol2_single lua d3d11 d3d12 glm_static shlwapi jsonstthm)
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC /GS- /GR- -fshort-wchar)
target_compile_features(${CMAKE_PROJECT_NAME} PUBLIC cxx_std_17)

include_directories(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE dependencies/reframework/shared)


set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES 
RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/fluffymod/Collab\ Trainer/Injectors
RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/RelWithDebInfo
RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/MinSize
OUTPUT_NAME sssiyan)

#add_custom_command(TARGET ${CMAKE_PROJECT_NAME}
                  # POST_BUILD
                   #COMMAND xcopy "$(MSBuildProjectDirectory)\\..\\..\\fluffymod" "$(SolutionDir)\\fluffymod\\" /E /Y /R VERBATIM)
                   target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC /EHa)
if(DEVELOPER_MODE)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC DEVELOPER)
    #target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC /EHa)
endif()