add_subdirectory(spdlog)

SET(GLM_TEST_ENABLE OFF CACHE BOOL "Build unit tests" FORCE)
add_subdirectory(glm)

project(minhook)

add_library(minhook STATIC
            minhook/src/buffer.c
            minhook/src/buffer.h
            minhook/src/hook.c
            minhook/src/trampoline.c
            minhook/src/trampoline.h
            minhook/src/hde/hde32.c
            minhook/src/hde/hde32.h
            minhook/src/hde/hde64.c
            minhook/src/hde/hde64.h
            minhook/src/hde/pstdint.h
            minhook/src/hde/table32.h
            minhook/src/hde/table64.h)

project(imgui)

add_library(imgui STATIC
            imgui/imconfig.h
            imgui/imgui_demo.cpp
            #imgui/imgui_widgets.cpp
            imgui/imgui_draw.cpp
            imgui/imgui.cpp
            imgui/imgui.h)

project(jsonstthm)

add_library(jsonstthm STATIC
            JsonStthm/JsonStthm.cpp
            JsonStthm/JsonStthm.h
            JsonStthm/JsonStthmConfig.h)

project(imwindow)

add_library(imwindow STATIC
            ImWindow/ImwContainer.cpp
            ImWindow/ImwContainer.h
            ImWindow/ImwMenu.cpp
            ImWindow/ImwMenu.h
            ImWindow/ImwPlatformWindow.cpp
            ImWindow/ImwPlatformWindow.h
            ImWindow/ImwStatusBar.cpp
            ImWindow/ImwStatusBar.h
            ImWindow/ImwToolBar.cpp
            ImWindow/ImwToolBar.h
            ImWindow/ImwWindow.cpp
            ImWindow/ImwWindow.h
            ImWindow/ImwWindowManager.cpp
            ImWindow/ImwWindowManager.h
            ImWindow/ImwConfig.h)

project(easywindow)

add_library(easywindow STATIC
            EasyWindow/EasyWindowWin32.cpp
            EasyWindow/EasyWindow.cpp
            EasyWindow/EasyWindow.h
)