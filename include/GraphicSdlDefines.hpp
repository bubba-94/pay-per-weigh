#ifndef SDLTYPES_HPP
#define SDLTYPES_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <cstdint>
#include <memory>

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
