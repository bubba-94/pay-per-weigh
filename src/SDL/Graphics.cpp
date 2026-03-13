#include "SDL/Graphics.hpp"

using namespace SDL;

//  ======CONSTRUCTORS=========

// Creates the needed textures for application
// Change paths here if different images or fonts are wanted
Graphics::Graphics(WindowRenderer &renderer)
    : qr(renderer.getRawRenderer(), "QR Code", "assets/img/qr.png"),
      logo(renderer.getRawRenderer(), "Logo", "assets/img/pandema.png"),
      weight(renderer.getRawRenderer(), "Weight",
             "assets/fonts/Lato-Light.ttf"),
      timepoint(renderer.getRawRenderer(), "Timepoint",
                "assets/fonts/Lato-Light.ttf"),
      renderer(renderer),
      messages(renderer.getRawRenderer(), "assets/fonts/Lato-Light.ttf") {

  logo.setPosition(GraphicsCfg::LOGO_X, GraphicsCfg::LOGO_Y,
                   GraphicsCfg::LOGO_WIDTH, GraphicsCfg::LOGO_HEIGHT);

  qr.setPosition(GraphicsCfg::IMAGE_X, GraphicsCfg::IMAGE_Y,
                 GraphicsCfg::IMAGE_WIDTH, GraphicsCfg::IMAGE_HEIGHT);

  weight.setPosition(weight.getWidthOf(0), GraphicsCfg::IMAGE_Y,
                     weight.getXCursorOf(0), GraphicsCfg::IMAGE_HEIGHT);

  timepoint.setPosition(GraphicsCfg::TIME_X, GraphicsCfg::TIME_Y,
                        GraphicsCfg::TIME_WIDTH, GraphicsCfg::TIME_HEIGHT);

  weightTimer.state = false;
  paymentTimer.state = false;
  messageTimer.state = false;

  std::cout << "[UI] Initialized\n";
}

Graphics::~Graphics(){};

#ifdef RPI
void Graphics::render(const States::AppInput input, bool admin) {

  if (weight.check(input.values.weight))
    weight.update(input.values.weight);
  if (timepoint.check(input.values.timepoint))
    timepoint.update(input.values.timepoint);

  if (admin) {
    renderStates = RenderState::WEIGHT;
  }

  switch (renderStates) {

  case RenderState::WELCOME_MESSAGE:
    messageState(input, admin);
    break;

  case RenderState::QR:
    qrState(input, admin);
    break;

  // Render welcome message
  case RenderState::PROCESSING_PAYMENT:
    paymentState(input, admin);
    break;

  // Render weight surface when payment is succesful or admin mode is active.
  case RenderState::WEIGHT:
    weightState(input, admin);
    break;
  }

  presentLogoAndTime();
}

void Graphics::messageState(const States::AppInput input, bool admin) {
  if (!messageTimer.state) {
    messageTimer.tp = Clock::now();
    messageTimer.state = true;
  }

  for (size_t i = 0; i < 2; ++i) {
    SDL_RenderCopy(renderer.getRawRenderer(), messages.getRawMessage(i), NULL,
                   &messages.getSpecRect(i));
  }

  if (!admin && input.values.weight >= 1500 &&
      Clock::now() - messageTimer.tp >= std::chrono::seconds(5)) {

    messageTimer.state = false;
    renderStates = RenderState::QR;
  }
}

void Graphics::qrState(const States::AppInput input, bool admin) {
  // Message == IDLE
  if (!input.pins.keyEnabled && input.values.weight < 1500) {
    renderStates = RenderState::WELCOME_MESSAGE;
  }
  if (input.payments.status == States::PaymentStatus::SUCCESS &&
      input.payments.status != States::PaymentStatus::RECEIVED) {
    renderStates = RenderState::PROCESSING_PAYMENT;
    // If Payment done renderState = RenderState::WEIGHT
  }

  SDL_RenderCopy(renderer.getRawRenderer(), qr.getTex(), NULL,
                 &qr.getSpecRect());
}

void Graphics::paymentState(const States::AppInput input, bool admin) {
  size_t len = messages.sizeOfMessages();

  if (!paymentTimer.state) {
    paymentTimer.tp = Clock::now();
    paymentTimer.state = true;
  }

  paymentTimer.elapsed = Clock::now() - paymentTimer.tp;

  // Render "PROCESSING PAYMENT"
  for (size_t i = 2; i < len; ++i) {
    SDL_RenderCopy(renderer.getRawRenderer(), messages.getRawMessage(i), NULL,
                   &messages.getSpecRect(i));
  }

  if (paymentTimer.elapsed >= std::chrono::seconds(3)) {
    paymentTimer.state = false;
    renderStates = RenderState::WEIGHT;
  }
}

void Graphics::weightState(const States::AppInput input, bool admin) {

  if (admin) {
    SDL_RenderCopy(renderer.getRawRenderer(), weight.getTex(), NULL,
                   &weight.getSpecRect());
  }

  if (!input.pins.keyEnabled &&
      input.payments.status != States::PaymentStatus::SUCCESS && !admin) {
    renderStates = RenderState::WELCOME_MESSAGE;
    return;
  }

  if (!weightTimer.state && !admin) {
    weightTimer.tp = Clock::now();
    weightTimer.state = true;
  }

  weightTimer.elapsed = Clock::now() - weightTimer.tp;

  // Only start clock when payment is successful,
  // to view the weight for a certain amount of seconds
  if (weightTimer.elapsed < std::chrono::seconds(10)) {
    SDL_RenderCopy(renderer.getRawRenderer(), weight.getTex(), NULL,
                   &weight.getSpecRect());
  } else {
    // Reset values
    weightTimer.state = false;
    paymentReceived = true;
    renderStates = RenderState::WELCOME_MESSAGE;
  }
}

void Graphics::presentLogoAndTime() {
  SDL_RenderCopy(renderer.getRawRenderer(), logo.getTex(), NULL,
                 &logo.getSpecRect());
  SDL_RenderCopy(renderer.getRawRenderer(), timepoint.getTex(), NULL,
                 &timepoint.getSpecRect());
}

#endif