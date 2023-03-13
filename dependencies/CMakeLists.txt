add_subdirectory(spdlog)

SET(GLM_TEST_ENABLE OFF CACHE BOOL "Build unit tests" FORCE)
add_subdirectory(glm)


project(lua)

	set(lua_SOURCES "")

	list(APPEND lua_SOURCES
		"lua/src/lapi.c"
		"lua/src/lauxlib.c"
		"lua/src/lbaselib.c"
		"lua/src/lcode.c"
		"lua/src/lcorolib.c"
		"lua/src/lctype.c"
		"lua/src/ldblib.c"
		"lua/src/ldebug.c"
		"lua/src/ldo.c"
		"lua/src/ldump.c"
		"lua/src/lfunc.c"
		"lua/src/lgc.c"
		"lua/src/linit.c"
		"lua/src/liolib.c"
		"lua/src/llex.c"
		"lua/src/lmathlib.c"
		"lua/src/lmem.c"
		"lua/src/loadlib.c"
		"lua/src/lobject.c"
		"lua/src/lopcodes.c"
		"lua/src/loslib.c"
		"lua/src/lparser.c"
		"lua/src/lstate.c"
		"lua/src/lstring.c"
		"lua/src/lstrlib.c"
		"lua/src/ltable.c"
		"lua/src/ltablib.c"
		"lua/src/ltm.c"
		"lua/src/lundump.c"
		"lua/src/lutf8lib.c"
		"lua/src/lvm.c"
		"lua/src/lzio.c"
	)

add_library(lua STATIC ${lua_SOURCES})


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

project(jsonstthm)

add_library(jsonstthm STATIC
            JsonStthm/JsonStthm.cpp
            JsonStthm/JsonStthm.h
            JsonStthm/JsonStthmConfig.h)

#pragma comment(lib, "jsonstthm.lib")

project(imgui)


add_library(imgui STATIC
            imgui/imconfig.h
            imgui/imgui.cpp
            imgui/imgui.h
            imgui/imgui_demo.cpp
            imgui/imgui_draw.cpp
            imgui/imgui_internal.h
            imgui/imgui_tables.cpp
            imgui/imgui_widgets.cpp
            imgui/imstb_rectpack.h
            imgui/imstb_textedit.h
            imgui/imstb_truetype.h
            imgui/misc/cpp/imgui_stdlib.cpp
            imgui/misc/cpp/imgui_stdlib.h
            imgui/misc/fonts/binary_to_compressed_c.cpp
            imgui/misc/single_file/imgui_single_file.h)

set(REF_BUILD_FRAMEWORK OFF CACHE BOOL "" FORCE)
set(REF_BUILD_DMC5_SDK ON CACHE BOOL "" FORCE)
set(REF_BUILD_DEPENDENCIES OFF CACHE BOOL "" FORCE)


add_subdirectory(reframework)
add_subdirectory(sol2/single)

target_include_directories(utility PRIVATE minhook/src/hde/)
target_include_directories(DMC5SDK PRIVATE minhook/src/hde/)
