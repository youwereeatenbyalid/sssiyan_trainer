cmake_minimum_required(VERSION 3.1)

project(DMC5)

include_directories(${CMAKE_SOURCE_DIR}/dependencies)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/imgui)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/minhook/include)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/spdlog/include)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/glm)

# set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:threadSafeInit- /MP")

add_compile_options(/W0)
add_compile_options(/MT)
add_compile_options(/Ob1)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fasm-blocks -Xclang -Wno-narrowing")
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

# Disable exceptions
# string(REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

find_package(Git)

execute_process(COMMAND
        "${GIT_EXECUTABLE}" log -1 --pretty=\"%h\"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND
        "${GIT_EXECUTABLE}" log -1 --date short --pretty=\"%ad\"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_COMMITTER_DATE
ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

set(LOG_FILENAME "Collab Trainer/Log/${CMAKE_PROJECT_NAME}_framework_log.txt")
set(LOG_ENTRY "${CMAKE_PROJECT_NAME} ModFramework entry")
set(CONFIG_FILENAME "Collab Trainer/Configs/${CMAKE_PROJECT_NAME}_fw_config.ini")
set(KEYBIND_CONFIG_FILENAME "Collab Trainer/Configs/Keys.ini")
set(GIT_HASH ${GIT_COMMIT_HASH})
set(GIT_DATE ${GIT_COMMITTER_DATE})
configure_file(Config.hpp.in ${CMAKE_SOURCE_DIR}/src/Config.hpp @ONLY)
# Statically compile runtime
#string(REGEX REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
#string(REGEX REPLACE "/Ob2" "/Ob1" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

add_subdirectory(dependencies)
add_subdirectory(src)