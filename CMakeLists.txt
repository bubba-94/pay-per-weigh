cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME pay-per-weigh)
project(${PROJECT_NAME} LANGUAGES CXX)

set(DEBUG_FLAGS "-Wall -Wextra -g -O2 -fno-omit-frame-pointer")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS}")


set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(ARCH aarch64)
set(ARCHIVE _ppw-pi5)

set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/${ARCH}")

list(APPEND CMAKE_PREFIX_PATH 
    ${SYSROOT}/usr/lib/cmake/SDL2
    ${SYSROOT}/usr/lib/cmake/libcamera
)

find_package(nlohmann_json REQUIRED)

# Locate _real_config.h
find_path(SDL2_INCLUDE_DIR
    SDL.h
    HINTS 
    "${SYSROOT}/usr/include/SDL2/"
    "${SYSROOT}/usr/include/aarch64-linux-gnu/SDL2/"
)

find_library(SDL2_LIBRARY
    NAMES SDL2
    HINTS "${SYSROOT}/usr/lib"
)

find_path(SDL2_IMAGE_INCLUDE_DIR
    SDL_image.h
    HINTS 
    "${SYSROOT}/usr/include/SDL2/"
    "${SYSROOT}/usr/include/aarch64-linux-gnu/SDL2/"
)

find_library(SDL2_IMAGE_LIBRARY
    NAMES SDL2_image
    HINTS "${SYSROOT}/usr/lib"
)

# SDL2 TTF library for fonts
find_path(SDL2_TTF_INCLUDE_DIR
    SDL_ttf.h
    HINTS 
    "${SYSROOT}/usr/include/SDL2/"
    "${SYSROOT}/usr/include/aarch64-linux-gnu/SDL2/"
)

find_library(SDL2_TTF_LIBRARY
    NAMES SDL2_ttf
    HINTS "${SYSROOT}/usr/lib"
)

# Create imported target
add_library(SDL2::SDL2 STATIC IMPORTED)
set_target_properties(SDL2::SDL2 PROPERTIES
    IMPORTED_LOCATION "${SDL2_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
)

add_library(SDL2_image::SDL2_image STATIC IMPORTED)
set_target_properties(SDL2_image::SDL2_image PROPERTIES
    IMPORTED_LOCATION ${SDL2_IMAGE_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${SDL2_IMAGE_INCLUDE_DIR}
)

add_library(SDL2_ttf::SDL2_ttf STATIC IMPORTED)
set_target_properties(SDL2_ttf::SDL2_ttf PROPERTIES
    IMPORTED_LOCATION ${SDL2_TTF_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${SDL2_TTF_INCLUDE_DIR}
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAM REQUIRED libcamera)
add_library(libcamera::libcamera INTERFACE IMPORTED)

target_include_directories(libcamera::libcamera INTERFACE
    ${LIBCAM_INCLUDE_DIRS}
)

target_link_libraries(libcamera::libcamera INTERFACE
    ${LIBCAM_LIBRARIES}
)


find_library(GPIOD_CXX_LIBRARY
    NAMES gpiodcxx
    HINTS "${SYSROOT}/usr/lib/aarch64-linux-gnu" 
    NO_DEFAULT_PATH
)

find_library(GPIOD_C_LIBRARY
    NAMES gpiod
    HINTS "${SYSROOT}/usr/lib/aarch64-linux-gnu" 
    NO_DEFAULT_PATH
)

message(STATUS ${STARTGREEN})

# Prints when building for debugging
message(STATUS "CMakeLists.txt output for aarch64")
message(STATUS "C++ compiler            = ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard            = ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler flags          = ${CMAKE_CXX_FLAGS}")
message(STATUS "Target OS               = ${CMAKE_SYSTEM_NAME}")
message(STATUS "Target processor        = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Sysroot                 = ${SYSROOT}")
message(STATUS "Source files directory  = ${SRC_DIR}" )
message(STATUS "Headers directory       = ${INCLUDE_DIR}")
message(STATUS "Binary output directory = ${OUTPUT_DIR}")
message(STATUS "External libraries:")
message(STATUS "SDL2 lib                = ${SDL2_LIBRARY}")
message(STATUS "SDL2 headers            = ${SDL2_INCLUDE_DIR}")
message(STATUS "gpiod lib               = ${GPIOD_CXX_LIBRARY}")
message(STATUS "libcamera headers       = ${LIBCAM_INCLUDE_DIRS}")

set(TOOLCHAIN_PRINTED TRUE CACHE INTERNAL "Prevent toolchain print twice")

message(STATUS ${ENDGREEN})

# Target output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# Include src/CMakeLists.txt
add_subdirectory(${SRC_DIR} sdl2-archive)

# Executable
add_executable(${PROJECT_NAME} main.cpp)

# Link the library defined in src/CMakeLists.txt
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        ${ARCHIVE}
        nlohmann_json::nlohmann_json
)

target_link_libraries(${PROJECT_NAME}

    PRIVATE
        ${GPIOD_CXX_LIBRARY} 
        ${GPIOD_C_LIBRARY}
)