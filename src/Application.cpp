#include "Application.hpp"

// Load modules in the right order.
Application::Application(const std::string &windowTitle,
                         const std::string &port, const std::string &chip)
    : window(windowTitle), graphics(window)
#ifdef RPI
      ,
      device(port), gpio(chip)
#endif
{
  status = true;
}
Application::~Application() {
  // Shutdown when requested
}

bool Application::getStatus() const { return status; }

#ifdef RPI
void Application::update(States::AppInput &input,
                         States::PaymentData &payment) {

  // Poll pins for updates (PRIORITY)
  // Shutdown and override
  gpio.poll();
  input.pins = gpio.getState();

  if (input.pins.shutdownRequested) {
    // Shutdown when requested
    status = false;
  }

  if (input.pins.keyEnabled) {
    admin = true;
  } else
    admin = false;

  input.payments.id = payment.id;
  if (payment.status == States::PaymentStatus::SENT && !payment.requested) {
    payment.requested = true;
    input.payments.status = payment.status;
  }

  // Fill with values from device
  input.values.timepoint = device.getTimepoint();
  input.values.weight = device.getWeight();

  window.clear();
  graphics.render(input, admin);
  window.present();
}
#endif