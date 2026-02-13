#include "Graphics.hpp"

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
  window.reset(SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                WINDOW_HEIGHT, windowFlags));

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
void SDLManager::poll(const PinState &state) {

  // Exit on request
  if (state.shutdownRequested) {
    status = false;
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

  // Present welcome message
  case SurfaceState::MESSAGE:
    // If conditions are met and timer is over present QR. (IR Laser)
    if (!keyOverride && newWeight >= 1500) {
      renderStates = SurfaceState::QR;
    }

    // Render welcome message first.
    for (size_t i = 0; i < message.size(); ++i) {
      SDL_RenderCopy(getRawRenderer(), getRawMessage(i), NULL,
                     &message[i].spec.rect);
    }
    break;

  // Present QR Code for payment method
  case SurfaceState::QR:
    if (!keyOverride && newWeight < 1500) {
      renderStates = SurfaceState::MESSAGE;
    }
    // If Payment done renderState = SurfaceState::WEIGHT
    SDL_RenderCopy(getRawRenderer(), getRawImage(), NULL, &qrSpec.rect);
    break;

  // Weight presenter
  // Will be set when payment is valid or keyOverride is true
  case SurfaceState::WEIGHT:
    if (!keyOverride) {
      renderStates = SurfaceState::MESSAGE;
    }

    SDL_RenderCopy(getRawRenderer(), getRawWeight(), NULL, &weightSpec.rect);
    break;
  }
}

void SDLManager::setup() {

  const std::vector<std::string> MESSAGES = {"WELCOME", "PAY-PER-WEIGH"};

  createTextures("assets/img/pandema.png", "assets/img/qr.png",
                 "assets/fonts/Lato-Light.ttf");

  createWelcomeMessage(MESSAGES);

  // Set surface framings to default
  setSurfacePosition(&timeSpec, TIME_X, TIME_Y, TIME_WIDTH, TIME_HEIGHT);
  setSurfacePosition(&qrSpec, IMAGE_X, IMAGE_Y, IMAGE_WIDTH, IMAGE_HEIGHT);
  setSurfacePosition(&logoSpec, LOGO_X, LOGO_Y, LOGO_WIDTH, LOGO_HEIGHT);
  setSurfacePosition(&weightSpec, weightX, WEIGHT_Y, weightWidth,
                     WEIGHT_HEIGHT);
}

void SDLManager::printErrMsg(const char *errMsg) {
  std::cerr << "SDL_Error occured: " << errMsg << "\n";
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
void SDLManager::loadSurfaceOfIMG(const char *filepath) {
  surface.reset(IMG_Load(filepath));

  if (!surface)
    printErrMsg(SDL_GetError());
}

void SDLManager::loadFontSurface(const char *filepath) {

  font.reset(TTF_OpenFont(filepath, 250));

  if (!font)
    printErrMsg(SDL_GetError());

  const char *value = "0";

  surface.reset(TTF_RenderUTF8_Solid(getRawFont(), value, weightSpec.color));

  if (!surface)
    printErrMsg(SDL_GetError());
}

void SDLManager::createWelcomeMessage(
    const std::vector<std::string> &messages) {
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
  setSurfacePosition(&weightSpec, weightX, WEIGHT_Y, weightWidth,
                     WEIGHT_HEIGHT);

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

  if (weight > MAX_WEIGHT) {
    return false;
  }

  if (weight != previousWeight) {
    previousWeight = weight;
    std::cout << "[SDL] New weight: " << weight << "\n";
    updateWeightTexture(weight);
    return true;
  }

  previousWeight = weight;
  return false;
}

bool SDLManager::checkTime(std::string_view currentTimepoint) {
  static std::string previousTimepoint{};

  if (currentTimepoint == previousTimepoint.c_str()) {
    return false;
  } else {
    previousTimepoint = currentTimepoint;
    updateTimeTexture(currentTimepoint);
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

void SDLManager::setSurfacePosition(SDLSpec *surface, Uint16 x, Uint16 y,
                                    Uint16 w, Uint16 h) {
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
                                      std::vector<SDLMessage> &vec) {

  int charHeight = 0, charWidth = 0;
  int lineSpacing = 50;

  // Total height of both messages
  int totalHeight = 0;
  for (size_t i = 0; i < vec.size(); ++i) {
    if (i == 0) {
      charHeight = 100;
    } else {
      charHeight = 70;
    }
    totalHeight += charHeight;
  }
  totalHeight += (vec.size() - 1) * lineSpacing;

  // Start drawing from top so that messages are vertically centered
  int startY = ((WINDOW_HEIGHT - totalHeight) / 2) - 200;

  for (size_t i = 0; i < vec.size(); ++i) {
    if (i == 0) {
      charWidth = 120;
      charHeight = 240;
    } else {
      charWidth = 50;
      charHeight = 100;
    }

    // Set width and height
    vec[i].spec.rect.w = charWidth * strings[i].size();
    vec[i].spec.rect.h = charHeight;

    // Horizontal centering
    vec[i].spec.rect.x = (WINDOW_WIDTH - vec[i].spec.rect.w) / 2;

    // Vertical stacking
    vec[i].spec.rect.y = startY;
    startY += charHeight + lineSpacing * 4;

    // Set color to white
    vec[i].spec.color = {255, 255, 255, 255};
  }
}

int SDLManager::checkLengthOfWeight(int weight) {

  std::string value = std::to_string(weight);
  Uint8 length = value.length();

  return length;
}

void SDLManager::setWeightWidth(int weight) {
  int length = checkLengthOfWeight(weight);

  weightWidth = WEIGHT_CHAR_SIZE * length;

  weightX = ((WINDOW_WIDTH / 2) + weightWidth / 2) - weightWidth;

  setSurfacePosition(&weightSpec, weightX, WEIGHT_Y, weightWidth,
                     WEIGHT_HEIGHT);
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
