#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "Textures.hpp"
#include "Types.hpp"
#include "WindowRenderer.hpp"

namespace SDL {

class Graphics {
  enum class RenderState : uint8_t {
    WELCOME_MESSAGE = 0,
    QR,
    PROCESSING_PAYMENT,
    WEIGHT,
  };

public:
  Graphics(WindowRenderer &renderer);
  ~Graphics();

#ifdef RPI
  void render(States::AppInput &input, bool admin = false);

  void messageState(States::AppInput &input);
  void qrState(States::AppInput &input);
  void paymentState(States::AppInput &input);
  void weightState(States::AppInput &input, bool admin);

  void presentLogoAndTime();
#endif

private:
  bool paymentReceived = false;

  StateTimer weightTimer;
  StateTimer paymentTimer;
  StateTimer messageTimer;

  ImageTexture qr;
  ImageTexture logo;
  FontTexture weight;
  FontTexture timepoint;
  WindowRenderer &renderer;
  MessageTexture messages;

  RenderState renderStates = RenderState::WELCOME_MESSAGE;
  FileError graphErr = GRAPHICS;
};
}; // namespace SDL

#endif