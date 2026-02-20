#ifndef SDLTYPES_HPP
#define SDLTYPES_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

/// C++ Standard Library
#include <cstdint>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <memory>
#include <queue>
#include <string>

/**
 * @namespace SDLGraphicsCfg
 *
 * @brief
 * Configuration for the graphical positioning of surfaces.
 */
namespace SDLGraphicsCfg {
// Image paths ()
#ifdef RPI
// Windows specs for Raspberry Pi Monitor (standing)
constexpr Uint16 WINDOW_WIDTH = 1080;
constexpr Uint16 WINDOW_HEIGHT = 1920;
constexpr Uint16 WEIGHT_CHAR_SIZE = 200;
#else

// Windows specs for Dekstop Application ()
constexpr Uint16 WINDOW_WIDTH = 1920;
constexpr Uint16 WINDOW_HEIGHT = 1080;
constexpr Uint16 WEIGHT_CHAR_SIZE = 250;
#endif

// Surface sizes and limits
constexpr Uint16 WEIGHT_HEIGHT = 500;
constexpr Uint16 IMAGE_WIDTH = 300;
constexpr Uint16 IMAGE_HEIGHT = 300;
constexpr Uint16 LOGO_WIDTH = 242;
constexpr Uint16 LOGO_HEIGHT = 48;
constexpr Uint16 TIME_WIDTH = 300;
constexpr Uint16 TIME_HEIGHT = 48;

// Max allowed weight
constexpr int MAX_WEIGHT = 15001;

// Weight position (centered)
constexpr Uint16 WEIGHT_Y =
    ((WINDOW_HEIGHT / 2) + WEIGHT_HEIGHT / 2) - WEIGHT_HEIGHT;

// Image position (centered)
constexpr Uint16 IMAGE_X = ((WINDOW_WIDTH / 2) + IMAGE_WIDTH / 2) - IMAGE_WIDTH;
constexpr Uint16 IMAGE_Y =
    (WINDOW_HEIGHT / 2) + (IMAGE_HEIGHT / 2) - IMAGE_HEIGHT;

// Logo position (bottom right) with spacing
constexpr Uint16 LOGO_X = WINDOW_WIDTH - LOGO_WIDTH - 50;
constexpr Uint16 LOGO_Y = WINDOW_HEIGHT - LOGO_HEIGHT - 50;

// Time position (bottom left)
constexpr Uint16 TIME_X = 50;
constexpr Uint16 TIME_Y = WINDOW_HEIGHT - TIME_HEIGHT - 50;
}; // namespace SDLGraphicsCfg

/**
 * @brief Template for a custom SDL instance deleter
 * @param SDL Class for example (SDL_Window or SDL_Surface)
 */
template <typename T> struct SDLDeleter;

/**
 * @brief Alias for custom deleter
 *
 * @param T SDL Class (e.g. SDL_Window or SDL_Surface)
 *
 * @return A unique pointer of T.
 *
 * @details
 * Called when @param T goes out of scope and frees memory.
 */
template <typename T> using sdl_unique = std::unique_ptr<T, SDLDeleter<T>>;

template <> struct SDLDeleter<SDL_Window> {
  SDLDeleter() = default;
  void operator()(SDL_Window *p) const { SDL_DestroyWindow(p); }
};

template <> struct SDLDeleter<SDL_Renderer> {
  SDLDeleter() = default;
  void operator()(SDL_Renderer *p) const { SDL_DestroyRenderer(p); }
};

template <> struct SDLDeleter<SDL_Texture> {
  SDLDeleter() = default;
  void operator()(SDL_Texture *p) const { SDL_DestroyTexture(p); }
};

template <> struct SDLDeleter<SDL_Surface> {
  SDLDeleter() = default;
  void operator()(SDL_Surface *p) const { SDL_FreeSurface(p); }
};

template <> struct SDLDeleter<TTF_Font> {
  SDLDeleter() = default;
  void operator()(TTF_Font *p) const { TTF_CloseFont(p); }
};

/**
 * @brief Decide the size and location of a SDL Surface
 * @param color a,r,g,b for coloring of surface
 * @param rect x,y,w,h cooridnates for the size and cursor
 * @return Design for a surface
 */
typedef struct SDLSurfaceSpec {
  SDL_Color color;
  SDL_Rect rect;
} SDLSpec;

typedef struct SDLMessage {
  sdl_unique<SDL_Texture> texture;
  SDLSpec spec;
} SDLMessage;

#endif
