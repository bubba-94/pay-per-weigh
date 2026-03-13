#include "RasbPi/Gpio.hpp"

#ifdef RPI

using namespace RasbPi;

GpioPi::GpioPi(const std::string &path) {
  gpiod::chip chip(path);

  if (!chip) {
    std::cerr << "[GPIO] Error opening " << path << "\n";
  }

  gpiod::request_builder builder = chip.prepare_request();

  setup(builder);

  request = builder.do_request();

  std::cout << "[GPIO] Offset " << static_cast<unsigned int>(LogicalPin::KEY)
            << " initialized\n";
  std::cout << "[GPIO] Offset "
            << static_cast<unsigned int>(LogicalPin::SHUTDOWN)
            << " initialized\n";
}

GpioPi::~GpioPi() {}

void GpioPi::poll() {

  // Return to top of function if no events found (constant)
  if (!request || !request->wait_edge_events(std::chrono::milliseconds{0}))
    return;

  // Store events in buffer
  gpiod::edge_event_buffer buffer;
  request->read_edge_events(buffer);

  // GPIO Event buffer
  for (const auto &event : buffer) {

    auto pin =
        static_cast<LogicalPin>(static_cast<unsigned int>(event.line_offset()));

    // Outcome if events occur (easy to add more)
    switch (pin) {

    case LogicalPin::SHUTDOWN:
      handleShutdown(event);
      break;
    case LogicalPin::KEY:
      handleKey(event);
      break;
    }
  }
}

const States::PinState GpioPi::getState() const { return state; }

void GpioPi::setup(gpiod::request_builder &builder) {

  // Settings for both lines
  settings.set_direction(gpiod::line::direction::INPUT);
  settings.set_edge_detection(gpiod::line::edge::BOTH);
  settings.set_active_low(false);

  // Add the settings after configurations
  builder.add_line_settings(
      gpiod::line::offset{static_cast<unsigned int>(LogicalPin::KEY)},
      settings);
  builder.add_line_settings(
      gpiod::line::offset{static_cast<unsigned int>(LogicalPin::SHUTDOWN)},
      settings);
}

void GpioPi::handleShutdown(const gpiod::edge_event &event) {
  // If event buffer is populated
    state.shutdownRequested =
      event.type() == gpiod::edge_event::event_type::RISING_EDGE;
}

void GpioPi::handleKey(const gpiod::edge_event &event) {
  // If event buffer is populated
  state.keyEnabled = event.type() == gpiod::edge_event::event_type::RISING_EDGE;
  
}

#endif