#ifdef RPI
#ifndef GPIO_HPP
#define GPIO_HPP

#include <iostream>
#include <optional>

#include <gpiod.hpp>

#include "../States.hpp"
#include "../external-libs/loggr/moody/Loggr.hpp"

namespace RasbPi {

/**
 * @class GpioPi
 *
 * @brief
 * Initiates the Gpio lines tested on a Raspberry Pi 5
 *
 * @returns
 * A manager for the GPIO lines on the Raspberry Pi
 *
 * @details
 * The class handles edge event cases on certain defined Pins.
 * Sets a current state and forwards it to a PinState that is shared with the
 * SDLManager. The pins determine what can/will be shown and shutdown request.
 * Possibility to add more logic.
 */
class GpioPi {
public:
  /**
   * @brief Constructor for the Gpio chip
   *
   * On Raspberry Pi 5 the path is "/dev/gpiochip4/"
   */
  GpioPi(moody::Loggr &logger, const std::string &path);
  ~GpioPi();

  /**
   * @brief Poll for edge events
   *
   * Requests are added into a edge event buffer and are read at entry
   */
  void poll();

  /**
   * @brief Getter for states shared between Graphics and UI.
   */
  const States::PinState getState() const;

private:
  /**
   * @brief Configures the settings and builds the line offset.
   */
  void setup(gpiod::request_builder &builder);

  /**
   * @brief Handler for the shutdown button event.
   */
  void handleShutdown(const gpiod::edge_event &event);

  /**
   * @brief Handler for the key pin event.
   */
  void handleKey(const gpiod::edge_event &event);

  moody::Loggr &logger;
  gpiod::line_settings settings; // Configurations of lines.
  std::optional<gpiod::line_request>
      request;            // Handle information about requests.
  States::PinState state; // State variable of pins (HIGH / LOW).
};

/**
 * @brief Enum class for offset of GPIO Pins.
 *
 * Pin layout of Raspberry Pi 5: https://pinout.xyz/pinout/pin29_gpio5/
 */
enum class LogicalPin : unsigned int {
  KEY = 17,
  SHUTDOWN = 27,
};
#endif // HEADER GUARD
}; // namespace RasbPi
#endif // ifdef RPI