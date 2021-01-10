set(MODS_SRC
    mods/SimpleMod.hpp
    mods/SimpleMod.cpp
    mods/PlayerTracker.hpp 
    mods/PlayerTracker.cpp 
    mods/StyleRank.hpp 
    mods/StyleRank.cpp 
    mods/SinCoordinate.hpp 
    mods/SinCoordinate.cpp 
    mods/CosCoordinate.hpp 
    mods/CosCoordinate.cpp 
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
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME dinput8)

if(DEVELOPER_MODE)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC DEVELOPER)
    target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC /EHa)
endif()