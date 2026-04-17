#include "../../include/deprecated/Graphics.hpp"

SDLManager::SDLManager(const std::string &windowTitle) {

  // Init SDL
  std::cout << "[SDL] Start initialization" << "\n";
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    printErrMsg(SDL_GetError());
  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    printErrMsg(SDL_GetError());
  if (TTF_Init() < 0)
    printErrMsg(SDL_GetError());

  int windowFlags = SDL_WINDOW_SHOWN;
#ifdef RPI
  windowFlags |= (SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN);
#endif

  // Create window from specifics
  window.reset(SDL_CreateWindow(
      windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      Graphics::SDLGraphicsCfg::WINDOW_WIDTH,
      Graphics::SDLGraphicsCfg::WINDOW_HEIGHT, windowFlags));

  int renderFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  if (!window)
    printErrMsg(SDL_GetError());

  // Assign renderer to window
  renderer.reset(SDL_CreateRenderer(getRawWindow(), -1, renderFlags));

  // Try fallback first
  if (!renderer)
    renderer.reset(
        SDL_CreateRenderer(getRawWindow(), -1, SDL_RENDERER_SOFTWARE));
  if (!renderer)
    printErrMsg(SDL_GetError());

  // State of window
  status = true;

  setup();
}

SDLManager::~SDLManager() {
  std::cout << "[SDL] Application being shutdown...." << "\n";

  // End other libraries before SDL Library
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  std::exit(1);
}

#ifdef RPI
void SDLManager::poll(const PinState &state, bool paymentStatus) {

  // Exit on request
  if (state.shutdownRequested) {
    status = false;
  }

  if (paymentStatus && !successfulPayment) {
    successfulPayment = true;
  }

  // Key switches the image shown
  if (state.keyEnabled) {
    // Override and always show WEIGHT
    keyOverride = true;
    renderStates = SurfaceState::WEIGHT;
  } else {
    keyOverride = false;
  }
}
#endif

bool SDLManager::getStatus() { return status; }

void SDLManager::render(int newWeight, std::string_view clock) {

  SDL_SetRenderDrawColor(getRawRenderer(), 0, 0, 0, 255);
  SDL_RenderClear(getRawRenderer());

  bool weightCheck = checkWeight(newWeight);
  if (weightCheck) {
    updateWeightTexture(newWeight);
  }
  bool timepointCheck = checkTime(clock);
  if (timepointCheck) {
    updateTimeTexture(clock);
  }

  switch (renderStates) {

  case SurfaceState::QR:

    // Message == IDLE
    if (!keyOverride && newWeight < 1500) {
      renderStates = SurfaceState::WELCOME_MESSAGE;
    }
    if (successfulPayment && !paymentReceived) {
      renderStates = SurfaceState::PROCESSING_PAYMENT;
      // If Payment done renderState = SurfaceState::WEIGHT
    }

    SDL_RenderCopy(getRawRenderer(), getRawImage(), NULL, &qrSpec.rect);
    break;

  // Render welcome message
  case SurfaceState::WELCOME_MESSAGE:

    if (!messageTimer.state) {
      messageTimer.tp = Graphics::Clock::now();
      messageTimer.state = true;
    }

    for (size_t i = 0; i < 2; ++i) {
      SDL_RenderCopy(getRawRenderer(), getRawMessage(i), NULL,
                     &message[i].spec.rect);
    }

    if (!keyOverride && newWeight >= 1500 &&
        Graphics::Clock::now() - messageTimer.tp >= std::chrono::seconds(5)) {

      messageTimer.state = false;
      renderStates = SurfaceState::QR;
    }
    break;

  case SurfaceState::PROCESSING_PAYMENT:
    if (!paymentProcessTimer.state) {
      paymentProcessTimer.tp = Graphics::Clock::now();
      paymentProcessTimer.state = true;
    }

    paymentProcessTimer.elapsed =
        Graphics::Clock::now() - paymentProcessTimer.tp;

    // Render "PROCESSING PAYMENT"
    for (size_t i = 2; i < message.size(); ++i) {
      SDL_RenderCopy(getRawRenderer(), getRawMessage(i), NULL,
                     &message[i].spec.rect);
    }

    if (paymentProcessTimer.elapsed >= std::chrono::seconds(3)) {
      paymentProcessTimer.state = false;
      renderStates = SurfaceState::WEIGHT;
    }

    break;

  // Render weight surface when payment is succesful or admin mode is active.
  case SurfaceState::WEIGHT:
    if (keyOverride) {
      SDL_RenderCopy(getRawRenderer(), getRawWeight(), NULL, &weightSpec.rect);
    }

    if (!keyOverride && !successfulPayment) {
      renderStates = SurfaceState::WELCOME_MESSAGE;
      break;
    }

    if (!weightTimer.state) {
      weightTimer.tp = Graphics::Clock::now();
      weightTimer.state = true;
    }

    weightTimer.elapsed = Graphics::Clock::now() - weightTimer.tp;

    // Only start clock when payment is successful,
    // to view the weight for a certain amount of seconds
    if (weightTimer.elapsed < std::chrono::seconds(10)) {
      SDL_RenderCopy(getRawRenderer(), getRawWeight(), NULL, &weightSpec.rect);
    } else {
      weightTimer.state = false;
      successfulPayment = false;
      paymentReceived = true;
      renderStates = SurfaceState::WELCOME_MESSAGE;
    }

    break;
  }

  SDL_RenderCopy(getRawRenderer(), getRawLogo(), NULL, &logoSpec.rect);
  SDL_RenderCopy(getRawRenderer(), getRawTime(), NULL, &timeSpec.rect);

  SDL_RenderPresent(getRawRenderer());
  SDL_Delay(16);
}

void SDLManager::setup() {

  const std::vector<std::string> MESSAGES = {"WELCOME", "PAY-PER-WEIGH",
                                             "PROCESSING", "PAYMENT"};

  createTextures("assets/img/pandema.png", "assets/img/qr.png",
                 "assets/fonts/Lato-Light.ttf");

  createMessages(MESSAGES);

  // Set surface framings to default
  setSurfacePosition(&timeSpec, Graphics::SDLGraphicsCfg::TIME_X,
                     Graphics::SDLGraphicsCfg::TIME_Y,
                     Graphics::SDLGraphicsCfg::TIME_WIDTH,
                     Graphics::SDLGraphicsCfg::TIME_HEIGHT);
  setSurfacePosition(&qrSpec, Graphics::SDLGraphicsCfg::IMAGE_X,
                     Graphics::SDLGraphicsCfg::IMAGE_Y,
                     Graphics::SDLGraphicsCfg::IMAGE_WIDTH,
                     Graphics::SDLGraphicsCfg::IMAGE_HEIGHT);
  setSurfacePosition(&logoSpec, Graphics::SDLGraphicsCfg::LOGO_X,
                     Graphics::SDLGraphicsCfg::LOGO_Y,
                     Graphics::SDLGraphicsCfg::LOGO_WIDTH,
                     Graphics::SDLGraphicsCfg::LOGO_HEIGHT);
  setSurfacePosition(&weightSpec, weightX, Graphics::SDLGraphicsCfg::WEIGHT_Y,
                     weightWidth, Graphics::SDLGraphicsCfg::WEIGHT_HEIGHT);
}

void SDLManager::printErrMsg(const std::string &errMsg) {
  std::cerr << "SDL_Error occured: " << errMsg.c_str() << "\n";
}

void SDLManager::createTextures(const std::string &logoPath,
                                const std::string &qrPath,
                                const std::string &fontPath) {

  // Loads the logo into memory
  loadSurfaceOfIMG(logoPath.c_str());
  logo.reset(SDL_CreateTextureFromSurface(getRawRenderer(), getRawSurface()));
  if (!logo)
    printErrMsg(SDL_GetError());

  loadSurfaceOfIMG(qrPath.c_str());

  image.reset(SDL_CreateTextureFromSurface(getRawRenderer(), getRawSurface()));
  if (!image)
    printErrMsg(SDL_GetError());

  // Loads the specified font into memory time and weight uses same font. (can
  // switch)
  loadFontSurface(fontPath.c_str());

  time.reset(SDL_CreateTextureFromSurface(getRawRenderer(), getRawSurface()));
  if (!time)
    printErrMsg(SDL_GetError());

  weight.reset(SDL_CreateTextureFromSurface(getRawRenderer(), getRawSurface()));
  if (!weight)
    printErrMsg(SDL_GetError());
}

// Imaging
void SDLManager::loadSurfaceOfIMG(const std::string &filepath) {
  surface.reset(IMG_Load(filepath.c_str()));

  if (!surface)
    printErrMsg(SDL_GetError());
}

void SDLManager::loadFontSurface(const std::string &filepath) {

  font.reset(TTF_OpenFont(filepath.c_str(), 250));

  if (!font)
    printErrMsg(SDL_GetError());

  const std::string VALUE = "0";

  surface.reset(
      TTF_RenderUTF8_Solid(getRawFont(), VALUE.c_str(), weightSpec.color));

  if (!surface)
    printErrMsg(SDL_GetError());
}

void SDLManager::createMessages(const std::vector<std::string> &messages) {
  message.resize(messages.size());

  setMessagePositionOf(messages, message);

  for (size_t i = 0; i < messages.size(); ++i) {
    surface.reset(TTF_RenderUTF8_Blended(getRawFont(), messages[i].c_str(),
                                         message[i].spec.color));
    if (!surface)
      printErrMsg(SDL_GetError());

    message[i].texture.reset(
        SDL_CreateTextureFromSurface(getRawRenderer(), getRawSurface()));

    if (!message[i].texture)
      printErrMsg(SDL_GetError());
  }
}

void SDLManager::updateWeightTexture(int newWeight) {

  std::string value = std::to_string(newWeight);

  setWeightWidth(newWeight);
  setSurfacePosition(&weightSpec, weightX, Graphics::SDLGraphicsCfg::WEIGHT_Y,
                     weightWidth, Graphics::SDLGraphicsCfg::WEIGHT_HEIGHT);

  surface.reset(
      TTF_RenderUTF8_Blended(getRawFont(), value.c_str(), weightSpec.color));
  if (!surface)
    printErrMsg(SDL_GetError());

  weight.reset(SDL_CreateTextureFromSurface(getRawRenderer(), getRawSurface()));
  if (!weight)
    printErrMsg(SDL_GetError());
}

void SDLManager::updateTimeTexture(std::string_view currentTimepoint) {

  timepoint = std::string(currentTimepoint);

  surface.reset(
      TTF_RenderUTF8_Solid(getRawFont(), timepoint.c_str(), weightSpec.color));
  if (!surface)
    printErrMsg(SDL_GetError());

  time.reset(SDL_CreateTextureFromSurface(getRawRenderer(), getRawSurface()));
  if (!time) {
    printErrMsg(SDL_GetError());
  }
}

bool SDLManager::checkWeight(int weight) {
  // Weight gets incremented in main loop
  // Set when parameters match
  static int previousWeight{};

  if (weight > Graphics::SDLGraphicsCfg::MAX_WEIGHT) {
    return false;
  }

  if (weight != previousWeight) {
    previousWeight = weight;
    std::cout << "[SDL] New weight: " << weight << "\n";
    return true;
  }

  previousWeight = weight;
  return false;
}

bool SDLManager::checkTime(std::string_view currentTimepoint) {
  static std::string previousTimepoint{};

  if (currentTimepoint == previousTimepoint) {
    return false;
  } else {
    previousTimepoint = currentTimepoint;
    return true;
  }
}

bool SDLManager::hasEvent() const { return !events.empty(); }

void SDLManager::pollEvents() {

  while (SDL_PollEvent(&event)) {

    events.push(event);

    while (hasEvent()) {

      event = getNext();

      switch (event.type) {

      case SDL_QUIT:
        std::cout << "[SDL] Closing SDL Window " << '\n';
        status = false;
        break;

      case SDL_KEYDOWN:
        showWeight = !showWeight;
        std::cout << "[SDL] Key pressed: "
                  << SDL_GetKeyName(event.key.keysym.sym) << '\n';
        break;

      case SDL_MOUSEBUTTONDOWN:
        std::cout << "[SDL] Switching texture | " << " X: " << event.button.x
                  << " Y: " << event.button.y << "\n";

        showWeight = !showWeight;
        break;
      default:
        break;
      }
    }
  }
}

SDL_Event SDLManager::getNext() {
  if (events.empty())
    return SDL_Event{}; // return empty event if none
  event = events.front();
  events.pop();
  return event;
}

void SDLManager::setRenderingColor(Uint8 r, Uint8 g, Uint8 b) {
  SDL_RenderClear(getRawRenderer());
  // Set a white window
  SDL_SetRenderDrawColor(getRawRenderer(), r, g, b, SDL_ALPHA_OPAQUE);
}

void SDLManager::setSurfacePosition(Graphics::SDLSurfaceSpec *surface, Uint16 x,
                                    Uint16 y, Uint16 w, Uint16 h) {
  // Standard white color
  surface->color.a = 255;
  surface->color.r = 255;
  surface->color.b = 255;
  surface->color.g = 255;

  surface->rect.x = x;
  surface->rect.y = y;
  surface->rect.w = w;
  surface->rect.h = h;
}
void SDLManager::setMessagePositionOf(const std::vector<std::string> &strings,
                                      std::vector<Graphics::SDLMessage> &vec) {
  int lineSpacing = 50;

  for (size_t group = 0; group < vec.size(); group += 2) {

    int charHeightTop = 240;
    int charHeightBottom = 100;

    int totalHeight = charHeightTop + charHeightBottom + lineSpacing;

    int startY =
        ((Graphics::SDLGraphicsCfg::WINDOW_HEIGHT - totalHeight) / 2) - 200;

    for (size_t i = 0; i < 2 && (group + i) < vec.size(); ++i) {

      int charWidth, charHeight;

      if (i == 1 && group == 0) {
        charWidth = 50;
        charHeight = 100;
      } else {
        charWidth = 100;
        charHeight = 200;
      }

      size_t idx = group + i;

      vec[idx].spec.rect.w = charWidth * strings[idx].size();
      vec[idx].spec.rect.h = charHeight;

      vec[idx].spec.rect.x =
          (Graphics::SDLGraphicsCfg::WINDOW_WIDTH - vec[idx].spec.rect.w) / 2;

      vec[idx].spec.rect.y = startY;

      startY += charHeight + lineSpacing * 4;

      vec[idx].spec.color = {255, 255, 255, 255};
    }
  }
}

int SDLManager::checkLengthOfWeight(int weight) {

  std::string value = std::to_string(weight);
  Uint8 length = value.length();

  return length;
}

void SDLManager::setWeightWidth(int weight) {
  int length = checkLengthOfWeight(weight);

  weightWidth = Graphics::SDLGraphicsCfg::WEIGHT_CHAR_SIZE * length;

  weightX = ((Graphics::SDLGraphicsCfg::WINDOW_WIDTH / 2) + weightWidth / 2) -
            weightWidth;

  setSurfacePosition(&weightSpec, weightX, Graphics::SDLGraphicsCfg::WEIGHT_Y,
                     weightWidth, Graphics::SDLGraphicsCfg::WEIGHT_HEIGHT);
}

SDL_Window *SDLManager::getRawWindow() const { return window.get(); }
SDL_Renderer *SDLManager::getRawRenderer() const { return renderer.get(); }
SDL_Surface *SDLManager::getRawSurface() const { return surface.get(); }
SDL_Texture *SDLManager::getRawLogo() const { return logo.get(); }
SDL_Texture *SDLManager::getRawTime() const { return time.get(); }
SDL_Texture *SDLManager::getRawImage() const { return image.get(); }
SDL_Texture *SDLManager::getRawWeight() const { return weight.get(); }
TTF_Font *SDLManager::getRawFont() const { return font.get(); }

SDL_Texture *SDLManager::getRawMessage(int index) const {
  return message[index].texture.get();
}
