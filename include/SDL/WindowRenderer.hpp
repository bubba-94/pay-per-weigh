#ifndef WINDOWRENDERER_HPP
#define WINDOWRENDERER_HPP

#include "../external-libs/loggr/moody/Loggr.hpp"

#include "Types.hpp"

namespace SDL {

class WindowRenderer {
public:
  WindowRenderer(const std::string &title, moody::Loggr &logger);
  ~WindowRenderer();

  void clear();
  void present();

  SDL_Window *getRawWindow() const;
  SDL_Renderer *getRawRenderer() const;

private:
  sdl_unique<SDL_Window> window;
  sdl_unique<SDL_Renderer> renderer;
  moody::Loggr &logger;

  /// @brief Status of the window. True in constructor
  bool status = false;
};
}; // namespace SDL

#endif