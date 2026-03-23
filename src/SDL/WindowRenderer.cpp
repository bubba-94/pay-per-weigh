#include "SDL/WindowRenderer.hpp"

using namespace SDL;

const std::string lf = "WindowRenderer.cpp";

WindowRenderer::WindowRenderer(const std::string &title, moody::Loggr &logger)
    : logger{logger} {
  logger.log(moody::Loggr::Level::INFO, "WINDOW", "Start initialization", {lf});
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    logger.log(moody::Loggr::Level::ERROR, "WINDOW", SDL_GetError(), {lf});
  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    logger.log(moody::Loggr::Level::ERROR, "WINDOW", SDL_GetError(), {lf});
  if (TTF_Init() < 0)
    logger.log(moody::Loggr::Level::ERROR, "WINDOW", SDL_GetError(), {lf});

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
    logger.log(moody::Loggr::Level::ERROR, "WINDOW", SDL_GetError(), {lf});

  // Assign renderer to window
  renderer.reset(SDL_CreateRenderer(getRawWindow(), -1, renderFlags));

  // Try fallback first
  if (!renderer)
    renderer.reset(
        SDL_CreateRenderer(getRawWindow(), -1, SDL_RENDERER_SOFTWARE));
  if (!renderer)
    logger.log(moody::Loggr::Level::ERROR, "WINDOW", SDL_GetError(), {lf});
}
WindowRenderer::~WindowRenderer() {
  logger.log(moody::Loggr::Level::FATAL, "WINDOW", "Shutting application down",
             {lf});

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