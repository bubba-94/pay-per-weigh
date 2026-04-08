#include "SDL/Graphics.hpp"

using namespace SDL;

const std::string LF = "Graphics.cpp";

//  ======CONSTRUCTORS=========

// Creates the needed textures for application
// Change paths here if different images or fonts are wanted
Graphics::Graphics(WindowRenderer &renderer, moody::Loggr &logger)
    : logger{logger},
      qr(logger, renderer.getRawRenderer(), "QR Code", "assets/img/qr.png"),
      logo(logger, renderer.getRawRenderer(), "Logo", "assets/img/pandema.png"),
      weight(logger, renderer.getRawRenderer(), "Weight",
             "assets/fonts/Lato-Light.ttf"),
      timepoint(logger, renderer.getRawRenderer(), "Timepoint",
                "assets/fonts/Lato-Light.ttf"),
      renderer(renderer), messages(logger, renderer.getRawRenderer(),
                                   "assets/fonts/Lato-Light.ttf") {

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

  logger.log(moody::Loggr::Level::INFO, "GRAPHICS", "Textures intiliazed",
             {LF});
}

Graphics::~Graphics(){};

#ifdef RPI
void Graphics::render(States::AppInput &input, bool admin) {

  if (weight.check(input.values.weight))
    weight.update(input.values.weight);
  if (timepoint.check(input.values.timepoint))
    timepoint.update(input.values.timepoint);

  if (admin) {
    renderStates = RenderState::WEIGHT;
  }

  switch (renderStates) {

  case RenderState::WELCOME_MESSAGE:
    messageState(input);
    break;

  case RenderState::QR:
    qrState(input);
    break;

  // Render welcome message
  case RenderState::PROCESSING_PAYMENT:
    paymentState(input);
    break;

  // Render weight surface when payment is succesful or admin mode is active.
  case RenderState::WEIGHT:
    weightState(input, admin);
    break;
  }

  presentLogoAndTime();
}

void Graphics::messageState(States::AppInput &input) {
  if (!messageTimer.state) {
    messageTimer.tp = Clock::now();
    messageTimer.state = true;
  }

  if (input.payments.status == States::PaymentStatus::SUCCESSFUL) {
    renderStates = RenderState::WEIGHT;
  }

  if (input.values.weight >= 1500 &&
      Clock::now() - messageTimer.tp >= std::chrono::seconds(5)) {

    messageTimer.state = false;
    renderStates = RenderState::QR;
  }

  for (size_t i = 0; i < 2; ++i) {
    SDL_RenderCopy(renderer.getRawRenderer(), messages.getRawMessage(i), NULL,
                   &messages.getSpecRect(i));
  }
}

void Graphics::qrState(States::AppInput &input) {
  if (input.payments.status == States::PaymentStatus::SUCCESSFUL) {
    renderStates = RenderState::WEIGHT;
  }
  // Message == IDLE
  if (!input.pins.keyEnabled && input.values.weight < 1500) {
    renderStates = RenderState::WELCOME_MESSAGE;
  }

  if (input.payments.status == States::PaymentStatus::SENT &&
      input.values.weight >= 1500) {
    input.payments.status = States::PaymentStatus::PROCESSING;
    renderStates = RenderState::PROCESSING_PAYMENT;
    // If Payment done renderState = RenderState::WEIGHT
  }

  SDL_RenderCopy(renderer.getRawRenderer(), qr.getTex(), NULL,
                 &qr.getSpecRect());
}

void Graphics::paymentState(States::AppInput &input) {
  size_t len = messages.sizeOfMessages();

  if (input.payments.status == States::PaymentStatus::SUCCESSFUL) {
    renderStates = RenderState::WEIGHT;
  }

  if (!paymentTimer.state) {
    paymentTimer.tp = Clock::now();
    paymentTimer.state = true;
  }

  paymentTimer.elapsed = Clock::now() - paymentTimer.tp;

  // Simulate successful payment
  if (paymentTimer.elapsed >= std::chrono::seconds(3)) {
    input.payments.status = States::PaymentStatus::SUCCESSFUL;
    paymentTimer.state = false;
    renderStates = RenderState::WEIGHT;
  }

  // Render "PROCESSING PAYMENT"
  for (size_t i = 2; i < len; ++i) {
    SDL_RenderCopy(renderer.getRawRenderer(), messages.getRawMessage(i), NULL,
                   &messages.getSpecRect(i));
  }
}

void Graphics::weightState(States::AppInput &input, bool admin) {

  // Render weight right away admin == true
  if (admin) {
    // Reset all timers when admin is active
    weightTimer.tp = Clock::now();
    paymentTimer.tp = Clock::now();
    messageTimer.tp = Clock::now();

    SDL_RenderCopy(renderer.getRawRenderer(), weight.getTex(), NULL,
                   &weight.getSpecRect());
    return;
  }

  // Disrupt admin mode.
  if (!input.pins.keyEnabled &&
      input.payments.status != States::PaymentStatus::SUCCESSFUL && !admin) {
    renderStates = RenderState::WELCOME_MESSAGE;
    return;
  }

  if (!weightTimer.state &&
      input.payments.status == States::PaymentStatus::SUCCESSFUL) {
    weightTimer.tp = Clock::now();
    weightTimer.state = true;
  }

  if (input.payments.status == States::PaymentStatus::SUCCESSFUL) {

    weightTimer.elapsed = Clock::now() - weightTimer.tp;

    if (weightTimer.elapsed >= std::chrono::seconds(10)) {
      // Reset for new payment
      input.payments.requested = false;
      weightTimer.state = false;
      paymentReceived = true;
      input.payments.status = States::PaymentStatus::NONE;
      renderStates = RenderState::WELCOME_MESSAGE;
      return;
    }
  }

  SDL_RenderCopy(renderer.getRawRenderer(), weight.getTex(), NULL,
                 &weight.getSpecRect());
}

void Graphics::presentLogoAndTime() {
  SDL_RenderCopy(renderer.getRawRenderer(), logo.getTex(), NULL,
                 &logo.getSpecRect());
  SDL_RenderCopy(renderer.getRawRenderer(), timepoint.getTex(), NULL,
                 &timepoint.getSpecRect());
}

#endif