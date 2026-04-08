add_library(${ARCHIVE}
    STATIC
    
    SDL/Events.cpp
    SDL/WindowRenderer.cpp
    SDL/Textures.cpp
    SDL/Graphics.cpp
    RasbPi/Device.cpp
    RasbPi/Gpio.cpp
    Camera/Type.cpp
    Client.cpp
    Application.cpp
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