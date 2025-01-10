cmake_minimum_required(VERSION 3.8)

# Options
set(TOOLS_DIR "" CACHE STRING "")

# Find tools
find_program(MKCDP MkCdp HINTS "${TOOLS_DIR}")
find_program(MKSYM MkSym HINTS "${TOOLS_DIR}")
find_program(MKPSXISO mkpsxiso HINTS "${TOOLS_DIR}/lib/mkpsxiso")

# Project
project(Funkin LANGUAGES C CXX ASM)

# Common definition interface
add_library(common_defs INTERFACE)

target_include_directories(common_defs INTERFACE ".")
target_include_directories(common_defs INTERFACE "${ASSET_DIR}")
target_include_directories(common_defs INTERFACE "${CHART_DIR}")

# Main executable
set(TARGET_EXE "${CMAKE_CURRENT_BINARY_DIR}/Funkin${CKSDK_EXECUTABLE_SUFFIX}")
set(TARGET_MAP "${CMAKE_CURRENT_BINARY_DIR}/Funkin${CKSDK_SYMBOL_MAP_SUFFIX}")
cksdk_executable(Funkin ${TARGET_EXE} ${TARGET_MAP}
	# Boot code
	"Boot/Funkin.cpp"
	"Boot/Funkin.h"
	"Boot/Character.cpp"
	"Boot/Character.h"
	"Boot/Compress.cpp"
	"Boot/Compress.h"
	"Boot/Random.cpp"
	"Boot/Random.h"
	"Boot/Timer.cpp"
	"Boot/Timer.h"
	"Boot/CDP.cpp"
	"Boot/CDP.h"
	"Boot/MMP.cpp"
	"Boot/MMP.h"
	"Boot/Loader.cpp"
	"Boot/Loader.h"
	"Boot/Wipe.cpp"
	"Boot/Wipe.h"
	"Boot/DATracker.cpp"
	"Boot/DATracker.h"
	"Boot/Profiler.cpp"
	"Boot/Profiler.h"
)

target_link_libraries(Funkin PRIVATE common_defs)

# Play State static library
cksdk_dll_static_library(PlayState
	# Play State core
	"PlayState/PlayState.cpp"
	"PlayState/PlayState.h"

	# Play State objects
	"PlayState/Singer.cpp"
	"PlayState/Singer.h"
	"PlayState/Object.cpp"
	"PlayState/Object.h"
	"PlayState/Combo.cpp"
	"PlayState/Combo.h"
	"PlayState/Camera.cpp"
	"PlayState/Camera.h"
)

target_link_libraries(PlayState PRIVATE common_defs)

# DLLs
set(DLL_DATA "")
function(scene_dll name)
	set(TARGET_DLL "${CMAKE_CURRENT_BINARY_DIR}/${name}${CKSDK_SHARED_LIBRARY_SUFFIX}")
	cksdk_dll(${name} ${TARGET_DLL} ${ARGN})
	set(DLL_DATA "${DLL_DATA};${TARGET_DLL}" PARENT_SCOPE)
endfunction()

# Menu scene
scene_dll(Menu
	# Menu state
	"Menu/Menu.cpp"
	"Menu/Menu.h"

	# Menu substates
	"Menu/OpeningSubstate.cpp"
	"Menu/OpeningSubstate.h"
	"Menu/TitleSubstate.cpp"
	"Menu/TitleSubstate.h"
)
target_link_libraries(Menu PRIVATE common_defs)

# Week1 scene
scene_dll(Week1
	# Week 1 state
	"Week1/Week1.cpp"
)
target_link_libraries(Week1 PRIVATE common_defs PlayState)

# Compile DLL package
set(DLL_CDP "${CMAKE_CURRENT_BINARY_DIR}/dll.cdp")

add_custom_command(
	OUTPUT  ${DLL_CDP}
	COMMAND ${MKCDP} ${DLL_CDP} ${DLL_DATA}
	DEPENDS ${MKCDP} ${DLL_DATA}
	COMMENT "Compiling dll.cdp"
)

# Compile symbols
set(SYM "Funkin.sym")
add_custom_command(
	OUTPUT  ${SYM}
	COMMAND ${MKSYM} ${SYM} ${TARGET_MAP}
	DEPENDS ${MKSYM} ${TARGET_MAP}
	COMMENT "Compiling symbols"
)

# Compile all package
set(ALL_CDP "${CMAKE_CURRENT_BINARY_DIR}/all.cdp")
set(ALL_DATA
	${SYM}
	"${CMAKE_CURRENT_BINARY_DIR}/dll.cdp"
	${DATA_CDP}
)

add_custom_command(
	OUTPUT  ${ALL_CDP}
	COMMAND ${MKCDP} ${ALL_CDP} ${ALL_DATA}
	DEPENDS ${MKCDP} ${ALL_DATA}
	COMMENT "Compiling all.cdp"
)

# CD image
set(CD_BIN "funkin.bin")
set(CD_CUE "funkin.cue")
set(CD_EXE "FUNK_000.01")

set(ISO_DIR "${CMAKE_SOURCE_DIR}/../iso")

configure_file("${CMAKE_SOURCE_DIR}/Funkin.xml" "Funkin.xml")

file(
    CONFIGURE
    OUTPUT  SYSTEM.CNF
    CONTENT [[
BOOT=cdrom:\\${CD_EXE};1
TCB=4
EVENT=10
STACK=801FFFF0
]]
    NEWLINE_STYLE LF
)

add_custom_command(
	OUTPUT  ${CD_BIN}
	COMMAND ${MKPSXISO} -y Funkin.xml
	DEPENDS ${TARGET_EXE} ${ALL_CDP} "Funkin.xml" "SYSTEM.CNF"
	COMMENT "Building CD image"
)
add_custom_target(Funkin_BinCue ALL DEPENDS ${CD_BIN})
