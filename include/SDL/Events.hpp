#ifndef EVENTS_HPP
#define EVENTS_HPP

#include "Types.hpp"

namespace SDL {

/**
 * @class Events
 *
 * @brief Class used only on desktop for testing
 *
 * @details will not run on the aarch64 binary which is used right now
 */
class Events {

public:
  Events();
  ~Events();
  void pollEvents();

private:
  SDL_Event getNext();
  bool hasEvent() const;

  // Member variables
  bool status = false;
  bool showWeight = false;

  SDL_Event event;
  std::queue<SDL_Event> events;
};

}; // namespace SDL

#endif