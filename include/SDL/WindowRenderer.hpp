#ifndef WINDOWRENDERER_HPP
#define WINDOWRENDERER_HPP

#include "States.hpp"
#include "Types.hpp"

namespace SDL {

class WindowRenderer {
public:
  WindowRenderer(const std::string &title);
  ~WindowRenderer();

  void clear();
  void present();

  SDL_Window *getRawWindow() const;
  SDL_Renderer *getRawRenderer() const;

private:
  sdl_unique<SDL_Window> window;
  sdl_unique<SDL_Renderer> renderer;

  /// @brief Status of the window. True in constructor
  bool status = false;

  /// @brief Evaluation for error messages
  FileError winrenErr = WIND_REND;
};
}; // namespace SDL

#endif