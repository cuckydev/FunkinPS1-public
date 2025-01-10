cmake_minimum_required(VERSION 3.8)

# clownlzss
project(clownlzss LANGUAGES CXX)
add_library(clownlzss STATIC
	"lib/clownlzss/comper.cpp"
	"lib/clownlzss/comper.h"
	"lib/clownlzss/common.cpp"
	"lib/clownlzss/common.h"
	"lib/clownlzss/memory_stream.cpp"
	"lib/clownlzss/memory_stream.h"
	"lib/clownlzss/clownlzss.h"
	"lib/clownlzss/clowncommon.h"
)

target_include_directories(clownlzss PUBLIC "lib/clownlzss")

# libimagequant
project(imagequant LANGUAGES C)
add_library(imagequant STATIC
	"lib/libimagequant/libimagequant.c"
	"lib/libimagequant/libimagequant.h"
	"lib/libimagequant/libimagequant_private.h"
	"lib/libimagequant/pam.c"
	"lib/libimagequant/pam.h"
	"lib/libimagequant/mediancut.c"
	"lib/libimagequant/mediancut.h"
	"lib/libimagequant/blur.c"
	"lib/libimagequant/blur.h"
	"lib/libimagequant/kmeans.c"
	"lib/libimagequant/kmeans.h"
	"lib/libimagequant/nearest.c"
	"lib/libimagequant/nearest.h"
	"lib/libimagequant/remap.c"
	"lib/libimagequant/remap.h"
	"lib/libimagequant/mempool.c"
	"lib/libimagequant/mempool.h"
)

target_include_directories(imagequant PUBLIC "lib/libimagequant")

# mkpsxiso
# NOTE: This builds tinyxml2
add_subdirectory("lib/mkpsxiso" EXCLUDE_FROM_ALL)

# FunkinAlgo
project(FunkinAlgo LANGUAGES CXX)
add_library(FunkinAlgo STATIC
	"lib/FunkinAlgo/FunkinAlgo.cpp"
	"lib/FunkinAlgo/FunkinAlgo.h"
	"lib/FunkinAlgo/stb_image.h"
	"lib/FunkinAlgo/stb_image_resize.h"
)

target_include_directories(FunkinAlgo PUBLIC "lib/FunkinAlgo")
target_link_libraries(FunkinAlgo PUBLIC clownlzss imagequant)

# MkCdp
project(MkCdp LANGUAGES CXX)
add_executable(MkCdp
	"MkPkg/MkPkg.cpp"
)

target_compile_definitions(MkCdp PRIVATE CDP)

# MkMmp
project(MkMmp LANGUAGES CXX)
add_executable(MkMmp
	"MkPkg/MkPkg.cpp"
)

target_compile_definitions(MkMmp PRIVATE MMP)

# MkChr
project(MkChr LANGUAGES CXX)
add_executable(MkChr
	"MkChr/MkChr.cpp"
)

target_link_libraries(MkChr PRIVATE FunkinAlgo tinyxml2)

# MkSpr
project(MkSpr LANGUAGES CXX)
add_executable(MkSpr
	"MkSpr/MkSpr.cpp"
)

target_link_libraries(MkSpr PRIVATE FunkinAlgo tinyxml2)

# MkDma
project(MkDma LANGUAGES CXX)
add_executable(MkDma
	"MkDma/MkDma.cpp"
)

# MkSym
project(MkSym LANGUAGES CXX)
add_executable(MkSym
	"MkSym/MkSym.cpp"
)

# MkHeader
project(MkHeader LANGUAGES CXX)
add_executable(MkHeader
	"MkHeader/MkHeader.cpp"
)

# MkCht
project(MkCht LANGUAGES CXX)
add_executable(MkCht
	"MkCht/MkCht.cpp"
)

# Dependency interface
project(Funkin_Tools)
add_library(Funkin_Tools INTERFACE)
add_dependencies(Funkin_Tools clownlzss libimagequant tinyxml2 FunkinAlgo MkCdp MkMmp MkChr MkDma MkSym MkHeader MkCht MkSpr mkpsxiso)
