#ifndef TEXTURES_HPP
#define TEXTURES_HPP
#include "../external-libs/loggr/moody/Loggr.hpp"
#include "Types.hpp"

namespace SDL {

using MsgVec = std::vector<std::string>;

class Texture {
public:
  Texture() = default;
  Texture(moody::Loggr &logger, SDL_Renderer *renderer,
          const std::string &title);
  virtual ~Texture() = default;

  virtual void load(const std::string &path) = 0;

  virtual void setPosition(Uint16 x, Uint16 y, Uint16 w, Uint16 h);
  virtual SDL_Texture *getTex() const;
  virtual const SDL_Rect &getSpecRect() const;
  virtual const SurfaceSpec &getSpec() const;

protected:
  moody::Loggr &logger;
  sdl_unique<SDL_Texture> texture;
  SurfaceSpec spec;
  SDL_Renderer *renderer;

private:
  const std::string TITLE;
};

class ImageTexture : public Texture {
public:
  ImageTexture(moody::Loggr &logger, SDL_Renderer *renderer,
               const std::string &textureName, const std::string &path);

private:
  void load(const std::string &path) override;
};

class FontTexture : public Texture {
public:
  FontTexture(const std::string &path);
  FontTexture(moody::Loggr &logger, SDL_Renderer *renderer,
              const std::string &textureName, const std::string &path);

  TTF_Font *getRawFont() const;

  bool check(int newWeight);
  bool check(std::string_view currentTimepoint);

  void update(int newWeight);
  void update(std::string_view currentTimepoint);
  int getWidthOf(int newWeight) const;
  int getXCursorOf(int newWeight) const;

private:
  int getLengthOf(int newWeight) const;

  void load(const std::string &path) override;

  // Member variables
  sdl_unique<TTF_Font> font;
};

class MessageTexture {
public:
  MessageTexture(moody::Loggr &logger, SDL_Renderer *renderer,
                 const std::string &path);
  void loadMessages(const std::string &path);
  void setMessagePositionOf(std::vector<std::string> &strings);

  SDL_Texture *getRawMessage(int index) const;
  const SDL_Rect &getSpecRect(int index) const;

  size_t sizeOfMessages() const;

private:
  moody::Loggr &logger;
  SDL_Renderer *renderer;
  sdl_unique<TTF_Font> font;
  std::vector<SDLMessage> output;
};

}; // namespace SDL

#endif