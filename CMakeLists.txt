cmake_minimum_required(VERSION 3.15)
project(PAY-PER-WEIGH LANGUAGES CXX)

set(DEBUG_FLAGS "-Wall -Wextra -g")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS}")

if(RPI)
    add_compile_definitions(RPI)
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(ARCH aarch64)
    set(ARCHIVE _ppw-pi5)
else()
    set(ARCH x86)
    set(ARCHIVE _ppw-x86)
endif()

set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/${ARCH}")

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")


    message(STATUS ${STARTYELLOW})
    
    if(NOT DEFINED TOOLCHAIN_PRINTED)
        # Prints when building for debugg
        message(STATUS "CMakeLists.txt output for x86")
        message(STATUS "C++ compiler            = ${CMAKE_CXX_COMPILER}")
        message(STATUS "C++ standard            = ${CMAKE_CXX_STANDARD}")
        message(STATUS "Compiler flags          = ${CMAKE_CXX_FLAGS}")
        message(STATUS "Static library filename = lib${ARCHIVE}.a")
        message(STATUS "Source file directory   = ${SRC_DIR}")
        message(STATUS "Header file diretory    = ${INCLUDE_DIR}")
        message(STATUS "Binary output directory = ${OUTPUT_DIR}")
        set(TOOLCHAIN_PRINTED TRUE CACHE INTERNAL "Prevent toolchain output twice")
    endif()

    list(APPEND CMAKE_PREFIX_PATH 
        ${SDL2_image_PATH}
        ${SDL2_ttf_PATH} 
    )

    find_package(SDL2 REQUIRED)

    find_package(SDL2_image QUIET CONFIG)
    if(NOT SDL2_image_FOUND)

        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

        add_library(SDL2_image::SDL2_image INTERFACE IMPORTED)
        target_include_directories(SDL2_image::SDL2_image INTERFACE ${SDL2_IMAGE_INCLUDE_DIRS})
        target_link_libraries(SDL2_image::SDL2_image INTERFACE ${SDL2_IMAGE_LIBRARIES})

    endif()

    find_package(SDL2_ttf QUIET CONFIG)
    if(NOT SDL2_ttf_FOUND)

        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

        add_library(SDL2_ttf::SDL2_ttf INTERFACE IMPORTED)
        target_include_directories(SDL2_ttf::SDL2_ttf INTERFACE ${SDL2_TTF_INCLUDE_DIRS})
        target_link_libraries(SDL2_ttf::SDL2_ttf INTERFACE ${SDL2_TTF_LIBRARIES})
    endif()
    message(STATUS ${ENDYELLOW})

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")

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
    
    if(NOT DEFINED TOOLCHAIN_PRINTED)

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
        message(STATUS "SDL2 sysroot lib        = ${SDL2_LIBRARY}")
        message(STATUS "SDL2 include dir        = ${SDL2_INCLUDE_DIR}")
        
        set(TOOLCHAIN_PRINTED TRUE CACHE INTERNAL "Prevent toolchain print twice")
    endif()

    # Make CMake look in sysroot for SDL2
    set(CMAKE_PREFIX_PATH "${SYSROOT}/usr/lib/cmake/SDL2" ${CMAKE_PREFIX_PATH})

    message(STATUS ${ENDGREEN})
endif() 

find_package(nlohmann_json REQUIRED)
# Target output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# Include src/CMakeLists.txt
add_subdirectory(${SRC_DIR} sdl2-archive)

# Executable
add_executable(pay-per-weigh main.cpp)

# Link the library defined in src/CMakeLists.txt
target_link_libraries(pay-per-weigh PRIVATE 
        ${ARCHIVE}
<<<<<<< HEAD
        nlohmann_json::nlohmann_json
=======
<<<<<<< HEAD
=======
        nlohmann_json::nlohmann_json
>>>>>>> 11b8a09 (Feat/payment method (#4))
>>>>>>> 4f54008 (Feat/payment method (#4))
        )

if(RPI)
    target_link_libraries(pay-per-weigh 
        PRIVATE 
            ${GPIOD_CXX_LIBRARY} 
            ${GPIOD_C_LIBRARY}
            )
endif()