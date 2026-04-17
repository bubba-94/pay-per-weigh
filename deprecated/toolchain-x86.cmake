# Configuration for x86 

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# Coloring for status
set(CMAKE_COLOR_DIAGNOSTICS ON CACHE INTERNAL "Allow coloring in CMakeLists.txt")
string(ASCII 27 Esc)
set(YELLOW      "${Esc}[1;33m" CACHE INTERNAL "")
set(RESETYELLOW       "${Esc}[0m" CACHE INTERNAL "")
set(STARTYELLOW  "${YELLOW}------------------------------" CACHE INTERNAL "")
set(ENDYELLOW    "------------------------------${RESETYELLOW}" CACHE INTERNAL "")

set(CMAKE_COLOR_DIAGNOSTICS OFF)