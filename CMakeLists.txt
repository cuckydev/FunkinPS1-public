cmake_minimum_required(VERSION 3.8)

include(ExternalProject)

# Project
project(
	Funkin
	LANGUAGES    C CXX ASM
	VERSION      0.0.1
	DESCRIPTION  "Funkin"
	HOMEPAGE_URL "https://ckdev.org"
)

# CKSDK setup
set(CKSDK_DIR "${CMAKE_SOURCE_DIR}/CKSDK")

set(CKSDK_TOOLS_DIR "${CMAKE_BINARY_DIR}/CKSDK_Tools")
add_subdirectory("${CKSDK_DIR}/tools" ${CKSDK_TOOLS_DIR})

# Compile tools
add_subdirectory("tools")

# Scene compile functions
function(chr_compile name)
	add_custom_command(
		OUTPUT "${name}.chr"
		COMMAND MkChr "${CMAKE_SOURCE_DIR}/${name}.xml" "${name}.chr"
		DEPENDS MkChr "${CMAKE_SOURCE_DIR}/${name}.xml"
		COMMENT "Compiling ${name}.chr"
	)
endfunction()

function(msh_compile name)
	add_custom_command(
		OUTPUT "${name}.chr" "${name}.dma"
		COMMAND MkChr "${CMAKE_SOURCE_DIR}/${name}.xml" "${name}.chr" "${name}.dma"
		DEPENDS MkChr "${CMAKE_SOURCE_DIR}/${name}.xml"
		COMMENT "Compiling ${name}.chr"
	)
endfunction()

function(spr_compile name)
	add_custom_command(
		OUTPUT "${name}.spr" "${name}.dma"
		COMMAND MkSpr "${CMAKE_SOURCE_DIR}/${name}.xml" "${name}.spr" "${name}.dma"
		DEPENDS MkSpr "${CMAKE_SOURCE_DIR}/${name}.xml"
		COMMENT "Compiling ${name}.spr"
	)
endfunction()

function(scene_compile name chrs mshs sprs)
	set(SCENE_PERM "")
	set(SCENE_TEMP "${name}/perm.dma")
	set(SCENE_DATA
		"${name}/perm.mmp"
		"${name}/temp.mmp"
	)

	# Compile characters
	foreach(NAME IN LISTS chrs)
		chr_compile(${NAME})
		list(APPEND SCENE_PERM "${NAME}.chr")
	endforeach()

	# Compile meshes
	set(SCENE_DMAS "")

	foreach(NAME IN LISTS mshs)
		msh_compile(${NAME})
		
		list(APPEND SCENE_PERM "${NAME}.chr")
		list(APPEND SCENE_DMAS "${NAME}.dma")
	endforeach()

	# Compile sprites
	foreach(NAME IN LISTS sprs)
		spr_compile(${NAME})

		list(APPEND SCENE_PERM "${NAME}.spr")
		list(APPEND SCENE_DMAS "${NAME}.dma")
	endforeach()

	# Compile permanent dma
	add_custom_command(
		OUTPUT "${name}/perm.dma"
		COMMAND MkDma "${name}/perm.dma" ${SCENE_DMAS}
		DEPENDS MkDma ${SCENE_DMAS}
		COMMENT "Compiling ${name}/perm.dma"
	)

	# Compile scene
	add_custom_command(
		OUTPUT "${name}/perm.mmp"
		COMMAND MkMmp "${name}/perm.mmp" ${SCENE_PERM}
		DEPENDS MkMmp ${SCENE_PERM}
		COMMENT "Compiling ${name}/perm.mmp"
	)
	add_custom_command(
		OUTPUT "${name}/temp.mmp"
		COMMAND MkMmp "${name}/temp.mmp" ${SCENE_TEMP}
		DEPENDS MkMmp ${SCENE_TEMP}
		COMMENT "Compiling ${name}/temp.mmp"
	)
	add_custom_command(
		OUTPUT "${name}.cdp"
		COMMAND MkCdp "${name}.cdp" ${SCENE_DATA}
		DEPENDS MkCdp ${SCENE_DATA}
		COMMENT "Compiling ${name}.cdp"
	)
endfunction()

# General
chr_compile("chr/Loader")

# Boyfriend data (Regular weeks)
set(BF_CHRS) # Empty, week 4 and week 5 variations should use the same data
set(BF_MSHS
	"msh/Note"
	"msh/NoteSplash"
	"msh/Combo"
	"msh/Icons/IconBf"
	"msh/Icons/IconBfOld"
)
set(BF_SPRS
	"spr/Score"
)

# Menu
set(MENU_CHRS
	"chr/GfTitle"
)
set(MENU_MSHS
	"msh/Logo"
)
set(MENU_SPRS
	"spr/Bold"
	"spr/MainMenu"
	"spr/MenuDesat"
)
scene_compile("menu" "${MENU_CHRS}" "${MENU_MSHS}" "${MENU_SPRS}")

# Week 1
set(WEEK1_CHRS
	${BF_CHRS}
	"chr/Bf"
	"chr/Dad"
	"chr/Gf"
)
set(WEEK1_MSHS
	${BF_MSHS}
	"msh/Icons/IconDad"
	"msh/Week1/StageBack"
	"msh/Week1/StageFront"
	"msh/Week1/StageCurtains"
)
set(WEEK1_SPRS
	${BF_SPRS}
)
scene_compile("week1" "${WEEK1_CHRS}" "${WEEK1_MSHS}" "${WEEK1_SPRS}")

# Package data
set(DATA_CDP "${CMAKE_CURRENT_BINARY_DIR}/data.cdp")
set(DATA_DATA
	"menu.cdp"
	"week1.cdp"
)

add_custom_command(
	OUTPUT  ${DATA_CDP}
	COMMAND MkCdp ${DATA_CDP} ${DATA_DATA}
	DEPENDS MkCdp ${DATA_DATA}
	COMMENT "Compiling data.cdp"
)

add_custom_target(
	Funkin_DataCdp
	DEPENDS ${DATA_CDP}
)

# Convert charts
set(CHART_DIR "${CMAKE_BINARY_DIR}/cht")
set(CHARTS
	"bopeebo-new"
	"fresh-new"
	"dadbattle-new"
	"spookeez-new"
	"south-new"
	"monster-new"
	"pico-new"
	"philly-new"
	"blammed-new"
	"satin-panties-new"
	"high-new"
	"milf-new"
	"cocoa-new"
	"eggnog-new"
	"winter-horrorland-new"
	"senpai-new"
	"roses-new"
	"thorns-new"
	"ugh-new"
	"guns-new"
	"stress-new"
)

set(CHART_HEADERS "")
foreach(CHART IN LISTS CHARTS)
	list(APPEND CHART_HEADERS "${CHART_DIR}/${CHART}.h")
	add_custom_command(
		OUTPUT  "${CHART_DIR}/${CHART}.h"
		COMMAND MkCht "${CHART_DIR}/${CHART}.h" "${CMAKE_SOURCE_DIR}/assets/cht/${CHART}.json"
		DEPENDS MkCht "${CMAKE_SOURCE_DIR}/assets/cht/${CHART}.json"
		COMMENT "Compiling ${CHART}.h"
	)
endforeach()

add_custom_target(
	Funkin_ChartInc
	DEPENDS ${CHART_HEADERS}
)

# Convert headers
set(ASSET_DIR "${CMAKE_BINARY_DIR}/assetinc")
set(ASSETS
	"chr/Loader.chr"
)

set(ASSET_HEADERS "")
foreach(ASSET IN LISTS ASSETS)
	list(APPEND ASSET_HEADERS "${ASSET_DIR}/${ASSET}.h")
	add_custom_command(
		OUTPUT  "${ASSET_DIR}/${ASSET}.h"
		COMMAND MkHeader "${ASSET_DIR}/${ASSET}.h" "${ASSET}"
		DEPENDS MkHeader "${ASSET}"
		COMMENT "Compiling ${ASSET}.h"
	)
endforeach()

add_custom_target(
	Funkin_AssetInc
	DEPENDS ${ASSET_HEADERS}
)

# Compile source
set(Funkin_src_args
	-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}

	-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CKSDK_DIR}/cmake/Toolchain.cmake
	-DCKSDK_TOOLS_DIR:FILEPATH=${CKSDK_TOOLS_DIR}
	
	-DTOOLS_DIR:FILEPATH=${CMAKE_BINARY_DIR}/tools
	-DDATA_CDP:FILEPATH=${DATA_CDP}
	-DASSET_DIR:FILEPATH=${ASSET_DIR}
	-DCHART_DIR:FILEPATH=${CHART_DIR}
)

ExternalProject_Add(Funkin_src
	SOURCE_DIR       "${PROJECT_SOURCE_DIR}/src"
	BINARY_DIR       Funkin_src-build
	CMAKE_CACHE_ARGS ${Funkin_src_args}
	CMAKE_ARGS       ${Funkin_src_args}
	INSTALL_COMMAND  ""
	BUILD_ALWAYS     1
	DEPENDS          Funkin_Tools CKSDK_Tools Funkin_DataCdp Funkin_AssetInc Funkin_ChartInc
)
