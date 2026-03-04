add_library(${ARCHIVE}
    STATIC 
       Graphics.cpp
       Client.cpp
       Device.cpp
       Gpio.cpp
)

target_include_directories(${ARCHIVE}
    PUBLIC
        ${INCLUDE_DIR}
)

target_link_libraries(${ARCHIVE}
    PUBLIC
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
)