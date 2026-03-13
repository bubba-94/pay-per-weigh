#include "SDL/WindowRenderer.hpp"

using namespace SDL;

WindowRenderer::WindowRenderer(const std::string &title) {
  std::cout << "[UI] Start initialization" << "\n";
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    Prints::errMsg(winrenErr, SDL_GetError());
  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    Prints::errMsg(winrenErr, SDL_GetError());
  if (TTF_Init() < 0)
    Prints::errMsg(winrenErr, SDL_GetError());

  int windowFlags = SDL_WINDOW_SHOWN;
#ifdef RPI
  windowFlags |= (SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN);
#endif

  // Create window from specifics
  window.reset(SDL_CreateWindow(
      title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      GraphicsCfg::WINDOW_WIDTH, GraphicsCfg::WINDOW_HEIGHT, windowFlags));

  int renderFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  if (!window)
    Prints::errMsg(winrenErr, SDL_GetError());

  // Assign renderer to window
  renderer.reset(SDL_CreateRenderer(getRawWindow(), -1, renderFlags));

  // Try fallback first
  if (!renderer)
    renderer.reset(
        SDL_CreateRenderer(getRawWindow(), -1, SDL_RENDERER_SOFTWARE));
  if (!renderer)
    Prints::errMsg(winrenErr, SDL_GetError());
}
WindowRenderer::~WindowRenderer() {
  std::cout << "[UI] Application being shutdown...." << "\n";

  // End other libraries before SDL Library
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  std::exit(1);
}

void WindowRenderer::clear() {
  SDL_SetRenderDrawColor(getRawRenderer(), 0, 0, 0, 255);
  SDL_RenderClear(getRawRenderer());
}

void WindowRenderer::present() {
  SDL_RenderPresent(renderer.get());
  SDL_Delay(16);
}

SDL_Window *WindowRenderer::getRawWindow() const { return window.get(); }
SDL_Renderer *WindowRenderer::getRawRenderer() const { return renderer.get(); }