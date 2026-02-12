#include "Device.hpp"
#include "Gpio.hpp"
#include "Graphics.hpp"

int main() {
  SDLManager sdl("pay-per-weigh");

#ifdef RPI
  Device pi("/dev/ttyACM0");
  GpioPi gpio("/dev/gpiochip4");
#endif

  std::string timePoint{};
  int currentWeight{0};

  while (sdl.getStatus()) {

#ifdef RPI
    currentWeight = pi.getWeight();
    timePoint = pi.getTimepoint();
    gpio.poll();
    sdl.poll(gpio.getState());
#else
    // For testing on desktop
    timePoint = "[TEST] 940601 - 13:37";
    sdl.pollEvents();
    currentWeight = 1337;
#endif

    sdl.render(currentWeight, timePoint);
  }

  return 0;
}