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
    mods/BufferedReversals.hpp 
    mods/BufferedReversals.cpp 
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
    mods/HideHUD.hpp 
    mods/HideHUD.cpp 
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
    mods/NeroNoDTCooldown.hpp 
    mods/NeroNoDTCooldown.cpp 
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
    mods/BufferedReversalsOriginal.hpp 
    mods/BufferedReversalsOriginal.cpp 
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
    mods/GilverNoDTCooldown.hpp 
    mods/GilverNoDTCooldown.cpp
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
	mods/DontHideWeaponWheel.hpp
	mods/DontHideWeaponWheel.cpp
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
)

set(HOOKS_SRC
    DInputHook.hpp
    DInputHook.cpp
    D3D11Hook.hpp
    D3D11Hook.cpp
    WindowsMessageHook.hpp
    WindowsMessageHook.cpp
    PositionHooks.hpp
    PositionHooks.cpp
)

set(SDK_SRC
    sdk/ReClass.hpp
    sdk/ReClass_Internal.hpp
    sdk/Enums_Internal.hpp
    sdk/Offsets.hpp
)

set(UTILITY_SRC
    utility/Address.hpp
    utility/Address.cpp
    utility/Config.hpp
    utility/Config.cpp
    utility/FunctionHook.hpp
    utility/FunctionHook.cpp
    utility/Memory.hpp
    utility/Memory.cpp
    utility/Module.hpp
    utility/Module.cpp
    utility/Patch.hpp
    utility/Patch.cpp
    utility/Pattern.hpp
    utility/Pattern.cpp
    utility/Scan.hpp
    utility/Scan.cpp
    utility/String.hpp
    utility/String.cpp
)

set(FRAMEWORK_SRC
    Mod.hpp
    Mods.hpp
    Mods.cpp
    ModFramework.hpp
    ModFramework.cpp
    Config.hpp
)

option(HOOK_D3D "Hook direct3D" OFF)

if(HOOK_D3D)
set(IMGUI_SRC
    fw-imgui/imgui_impl_win32.h
    fw-imgui/imgui_impl_win32.cpp
    fw-imgui/imgui_impl_dx11.h
    fw-imgui/imgui_impl_dx11.cpp
)
else()
set(IM_WINDOW_DX11
    ImWindowDX11/ImwPlatformWindowDX11.cpp  
    ImWindowDX11/ImwPlatformWindowDX11.h
    ImWindowDX11/ImwWindowManagerDX11.cpp   
    ImWindowDX11/ImwWindowManagerDX11.h  
    ImWindowDX11/Win32MessageHelper.cpp
    ImWindowDX11/Win32MessageHelper.h
)

set(IM_WINDOW_EASY_WINDOW
    ImWindowEasyWindow/ImwPlatformWindowEasyWindow.cpp  
    ImWindowEasyWindow/ImwPlatformWindowEasyWindow.h    
    ImWindowEasyWindow/ImwWindowManagerEasyWindow.cpp
    ImWindowEasyWindow/ImwWindowManagerEasyWindow.h
)
endif()

source_group("Hooks" FILES ${HOOKS_SRC})
source_group("Mods" FILES ${MODS_SRC})
source_group("SDK" FILES ${SDK_SRC})
source_group("Utility" FILES ${UTILITY_SRC})
source_group("Framework" FILES ${FRAMEWORK_SRC})

if(HOOK_D3D)
    source_group("imgui", FILES ${IMGUI_SRC})
else()
    source_group("ImWindowDX11" FILES ${IM_WINDOW_DX11})
    source_group("ImWindowEasyWindow" FILES ${IM_WINDOW_EASY_WINDOW})
endif()

add_library(${CMAKE_PROJECT_NAME} SHARED
            Main.cpp
            LicenseStrings.hpp
            ${SDK_SRC}
            ${MODS_SRC}
            ${HOOKS_SRC}
            ${IMGUI_SRC}
            ${UTILITY_SRC}
            ${IM_WINDOW_DX11}
            ${IM_WINDOW_EASY_WINDOW}
            ${FRAMEWORK_SRC}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC imgui minhook d3d11 glm_static jsonstthm imwindow easywindow shlwapi)
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC /GS- /GR-)
target_compile_features(${CMAKE_PROJECT_NAME} PUBLIC cxx_std_17)

include_directories(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR})


set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES 
RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/fluffymod/Collab\ Trainer/Injectors
RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/RelWithDebInfo
RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/MinSize
OUTPUT_NAME sssiyan)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME}
                   POST_BUILD
                   COMMAND xcopy "$(MSBuildProjectDirectory)\\..\\..\\fluffymod" "$(SolutionDir)fluffymod\\" /E /Y /R VERBATIM)
if(DEVELOPER_MODE)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC DEVELOPER)
    target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC /EHa)
endif()