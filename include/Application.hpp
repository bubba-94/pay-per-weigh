#ifndef APPLICATION_HPP
#define APPLICATION_HPP

// Graphics
#include "SDL/Events.hpp"
#include "SDL/Graphics.hpp"
#include "SDL/Types.hpp"
#include "SDL/WindowRenderer.hpp"

#ifdef RPI
// Raspberry Pi 5
#include "RasbPi/Device.hpp"
#include "RasbPi/Gpio.hpp"
#include "States.hpp"
#endif

class Application {
public:
  Application(const std::string &windowTitle, const std::string &chip,
              const std::string &port);
  ~Application();
#ifdef RPI
  void update(States::AppInput &input, const States::PaymentData &payment);
#endif
  bool getStatus() const;

private:
  /// @brief  State of application, false == SHUTDOWN REQUESTED
  bool status = false;

  /// @brief State of rendering, true == ADMIN MODE PRESENT WEIGHT
  bool adminMode = false;

  /// @brief One application, one window.
  SDL::WindowRenderer window;
  SDL::Graphics graphics;
  SDL::Events events;
#ifdef RPI
  RasbPi::Device device;
  RasbPi::GpioPi gpio;
  States::AppInput currentStates;
#endif
};

#endif