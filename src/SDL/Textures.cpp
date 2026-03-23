#include "SDL/Textures.hpp"

using namespace SDL;

const std::string LF = "Textures.cpp";
//  ======CONSTRUCTORS=========

Texture::Texture(moody::Loggr &logger, SDL_Renderer *renderer,
                 const std::string &title)
    : logger{logger}, renderer(renderer), TITLE(title) {}

// Constructor for other Textures
ImageTexture::ImageTexture(moody::Loggr &logger, SDL_Renderer *renderer,
                           const std::string &textureTitle,
                           const std::string &path)
    : Texture(logger, renderer, textureTitle) {
  load(path);
}

FontTexture::FontTexture(moody::Loggr &logger, SDL_Renderer *renderer,
                         const std::string &textureTitle,
                         const std::string &path)
    : Texture(logger, renderer, textureTitle) {
  load(path);
}

MessageTexture::MessageTexture(moody::Loggr &logger, SDL_Renderer *renderer,
                               const std::string &path)
    : logger{logger}, renderer(renderer) {
  loadMessages(path);
}

// ============BASE TEXTURE==============

SDL_Texture *Texture::getTex() const { return texture.get(); }
const SurfaceSpec &Texture::getSpec() const { return spec; }
const SDL_Rect &Texture::getSpecRect() const { return spec.rect; }

// Positioning of a surface for ImageTexture and FontTexture
void Texture::setPosition(Uint16 x, Uint16 y, Uint16 w, Uint16 h) {
  spec.color.a = 255;
  spec.color.r = 255;
  spec.color.b = 255;
  spec.color.g = 255;

  spec.rect.x = x;
  spec.rect.y = y;
  spec.rect.w = w;
  spec.rect.h = h;
}

// ============IMAGE TEXTURE==============

// Load the images into memory. Static and wont change during runtime
void ImageTexture::load(const std::string &path) {

  // Will throw away after loaded into memory
  sdl_unique<SDL_Surface> surface;

  surface.reset(IMG_Load(path.c_str()));
  if (!surface)
    logger.log(moody::Loggr::Level::ERROR, "TEXTURE", SDL_GetError(), {LF});

  texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));
  if (!texture)
    logger.log(moody::Loggr::Level::ERROR, "TEXTURE", SDL_GetError(), {LF});
}

// ============FONT TEXTURE==============

TTF_Font *FontTexture::getRawFont() const { return font.get(); }

// Load the font into maybe (probably dont need one more each)
void FontTexture::load(const std::string &path) {

  sdl_unique<SDL_Surface> surface;

  font.reset(TTF_OpenFont(path.c_str(), 300));

  if (!font)
    logger.log(moody::Loggr::Level::ERROR, "TEXTURE", SDL_GetError(), {LF});

  const std::string VALUE = "0";

  surface.reset(TTF_RenderUTF8_Solid(font.get(), VALUE.c_str(), spec.color));

  if (!surface)
    logger.log(moody::Loggr::Level::ERROR, "TEXTURE", SDL_GetError(), {LF});

  texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));
  if (!texture)
    logger.log(moody::Loggr::Level::ERROR, "TEXTURE", SDL_GetError(), {LF});
}

// Check if weight needs an update
bool FontTexture::check(int newWeight) {
  static int previousWeight{};

  if (newWeight > GraphicsCfg::MAX_WEIGHT) {
    return false;
  }

  if (newWeight != previousWeight) {
    previousWeight = newWeight;
    logger.log(moody::Loggr::Level::INFO, "TEXTURES",
               "New weight: " + std::to_string(newWeight), {LF});
    return true;
  }

  previousWeight = newWeight;
  return false;
}

// Check if timepoint needs an update
bool FontTexture::check(std::string_view currentTimepoint) {
  static std::string previousTimepoint{};

  if (currentTimepoint == previousTimepoint) {
    return false;
  } else {
    previousTimepoint = currentTimepoint;
    logger.log(moody::Loggr::Level::INFO, "TEXTURES", previousTimepoint, {LF});
    return true;
  }
}

// Update if check is true
void FontTexture::update(int newWeight) {
  sdl_unique<SDL_Surface> surface;
  std::string value = std::to_string(newWeight);

  int weightX = getXCursorOf(newWeight);
  int weightWidth = getWidthOf(newWeight);

  setPosition(weightX, GraphicsCfg::WEIGHT_Y, weightWidth,
              GraphicsCfg::WEIGHT_HEIGHT);

  surface.reset(
      TTF_RenderUTF8_Blended(getRawFont(), value.c_str(), spec.color));
  if (!surface)
    logger.log(moody::Loggr::Level::ERROR, "TEXTURE", SDL_GetError(), {LF});

  texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));
  if (!texture)
    logger.log(moody::Loggr::Level::ERROR, "TEXTURE", SDL_GetError(), {LF});
}

// Update if check is true
void FontTexture::update(std::string_view currentTimepoint) {
  sdl_unique<SDL_Surface> surface;
  std::string timepoint = std::string(currentTimepoint);

  surface.reset(
      TTF_RenderUTF8_Solid(getRawFont(), timepoint.c_str(), spec.color));
  if (!surface)
    logger.log(moody::Loggr::Level::ERROR, "TEXTURE", SDL_GetError(), {LF});

  texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));

  if (!texture) {
    logger.log(moody::Loggr::Level::ERROR, "TEXTURE", SDL_GetError(), {LF});
  }
}

int FontTexture::getLengthOf(int newWeight) const {

  std::string value = std::to_string(newWeight);
  Uint8 length = value.length();

  // Width of new weight in amount of chars
  return length;
}

int FontTexture::getWidthOf(int newWeight) const {
  int length = getLengthOf(newWeight);

  int weightWidth = GraphicsCfg::WEIGHT_CHAR_SIZE * length;

  // New width for weight update (dynamic at runtime)
  return weightWidth;
}

int FontTexture::getXCursorOf(int newWeight) const {

  int weightWidth = getWidthOf(newWeight);

  int weightX =
      ((GraphicsCfg::WINDOW_WIDTH / 2) + weightWidth / 2) - weightWidth;

  // New x-cursor for weight update (dynamic at runtime)
  return weightX;
}

// ============MESSAGE TEXTURE==============

SDL_Texture *MessageTexture::getRawMessage(int index) const {
  return output[index].texture.get();
}

const SDL_Rect &MessageTexture::getSpecRect(int index) const {
  return output[index].spec.rect;
}

size_t MessageTexture::sizeOfMessages() const { return output.size(); }

// Load messages into memory
void MessageTexture::loadMessages(const std::string &path) {

  sdl_unique<SDL_Surface> surface;

  font.reset(TTF_OpenFont(path.c_str(), 300));

  if (!font)
    logger.log(moody::Loggr::Level::ERROR, "TEXTURE", SDL_GetError(), {LF});

  // For loading into memory, discarded later
  MsgVec tempStrings = {"WELCOME", "PAY-PER-WEIGH", "PROCESSING", "PAYMENT"};

  // Amount == strings in messages vector
  size_t amount = tempStrings.size();

  // Resize the output vector from messages provided
  output.resize(amount);

  setMessagePositionOf(tempStrings);

  for (size_t i = 0; i < amount; ++i) {
    // Render and map to the output vector
    surface.reset(TTF_RenderUTF8_Blended(font.get(), tempStrings[i].c_str(),
                                         output[i].spec.color));
    if (!surface)
      logger.log(moody::Loggr::Level::ERROR, "TEXTURE", SDL_GetError(), {LF});

    output[i].texture.reset(
        SDL_CreateTextureFromSurface(renderer, surface.get()));

    if (!output[i].texture)
      logger.log(moody::Loggr::Level::ERROR, "TEXTURE", SDL_GetError(), {LF});
  }
}

// Sets the positions of messages, centered on a standing screen (terminal mode)
void MessageTexture::setMessagePositionOf(std::vector<std::string> &strings) {
  int lineSpacing = 50;

  for (size_t group = 0; group < output.size(); group += 2) {

    int charHeightTop = 240;
    int charHeightBottom = 100;

    int totalHeight = charHeightTop + charHeightBottom + lineSpacing;

    int startY = ((GraphicsCfg::WINDOW_HEIGHT - totalHeight) / 2) - 200;

    for (size_t i = 0; i < 2 && (group + i) < output.size(); ++i) {

      int charWidth, charHeight;

      if (i == 1 && group == 0) {
        charWidth = 50;
        charHeight = 100;
      } else {
        charWidth = 100;
        charHeight = 200;
      }

      size_t idx = group + i;

      output[idx].spec.rect.w = charWidth * strings[idx].size();
      output[idx].spec.rect.h = charHeight;

      output[idx].spec.rect.x =
          (GraphicsCfg::WINDOW_WIDTH - output[idx].spec.rect.w) / 2;

      output[idx].spec.rect.y = startY;

      startY += charHeight + lineSpacing * 4;

      output[idx].spec.color = {255, 255, 255, 255};
    }
  }
}