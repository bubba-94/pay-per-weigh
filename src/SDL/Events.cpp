#include "SDL/Events.hpp"

using namespace SDL;

Events::Events() { status = true; }

Events::~Events() {}

void Events::pollEvents() {

  while (SDL_PollEvent(&event)) {

    events.push(event);

    while (hasEvent()) {

      event = getNext();

      switch (event.type) {

      case SDL_QUIT:
        std::cout << "[SDL] Closing SDL Window " << '\n';
        status = false;
        break;

      case SDL_KEYDOWN:
        showWeight = !showWeight;
        std::cout << "[SDL] Key pressed: "
                  << SDL_GetKeyName(event.key.keysym.sym) << '\n';
        break;

      case SDL_MOUSEBUTTONDOWN:
        std::cout << "[SDL] Switching texture | " << " X: " << event.button.x
                  << " Y: " << event.button.y << "\n";

        showWeight = !showWeight;
        break;
      default:
        break;
      }
    }
  }
}

// Checks if there are events queued
bool Events::hasEvent() const { return !events.empty(); }

// Pop an event an queues the next
SDL_Event Events::getNext() {
  if (events.empty())
    return SDL_Event{};
  event = events.front();
  events.pop();
  return event;
}