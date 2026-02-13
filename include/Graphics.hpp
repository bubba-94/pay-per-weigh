#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

/// C++ Standard Library
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#ifdef RPI
#include "PinState.hpp"
#endif

// File to keep this file
#include "GraphicSdlDefines.hpp"

// Image paths ()
#ifdef RPI
// Windows specs for Raspberry Pi Monitor (standing)
constexpr Uint16 WINDOW_WIDTH = 1080;
constexpr Uint16 WINDOW_HEIGHT = 1920;
constexpr Uint16 WEIGHT_CHAR_SIZE = 200;
#else

// Windows specs for Dekstop Application ()
constexpr Uint16 WINDOW_WIDTH = 1920;
constexpr Uint16 WINDOW_HEIGHT = 1080;
constexpr Uint16 WEIGHT_CHAR_SIZE = 250;
#endif

// Surface sizes and limits
constexpr Uint16 WEIGHT_HEIGHT = 500;
constexpr Uint16 IMAGE_WIDTH = 500;
constexpr Uint16 IMAGE_HEIGHT = 500;
constexpr Uint16 LOGO_WIDTH = 242;
constexpr Uint16 LOGO_HEIGHT = 48;
constexpr Uint16 TIME_WIDTH = 300;
constexpr Uint16 TIME_HEIGHT = 48;

// Max allowed weight
constexpr int MAX_WEIGHT = 15001;

// Weight position (centered)
constexpr Uint16 WEIGHT_Y =
    ((WINDOW_HEIGHT / 2) + WEIGHT_HEIGHT / 2) - WEIGHT_HEIGHT;

// Image position (centered)
constexpr Uint16 IMAGE_X = ((WINDOW_WIDTH / 2) + IMAGE_WIDTH / 2) - IMAGE_WIDTH;
constexpr Uint16 IMAGE_Y =
    (WINDOW_HEIGHT / 2) + (IMAGE_HEIGHT / 2) - IMAGE_HEIGHT;

// Logo position (bottom right) with spacing
constexpr Uint16 LOGO_X = WINDOW_WIDTH - LOGO_WIDTH - 50;
constexpr Uint16 LOGO_Y = WINDOW_HEIGHT - LOGO_HEIGHT - 50;

// Time position (bottom left)
constexpr Uint16 TIME_X = 50;
constexpr Uint16 TIME_Y = WINDOW_HEIGHT - TIME_HEIGHT - 50;

/**
 *
 * @class SDLManager
 *
 * @brief SDL Library wrapper for image rendering.
 *
 * @returns
 * A Window manager that frees memory when they reach out of scope.
 *
 * @details
 * Class that wraps simple SDL functions to create different textures for
 * rendering.
 */
class SDLManager {

public:
  /**
   * @brief Constructor that initializes the SDL window.
   *
   * Initializes memory needed and creates standardized window and renderer.
   *
   * @param windowTitle Name of the SDL Window
   */
  SDLManager(const std::string &windowTitle);

  /**
   * @brief Destructor for freeing memory and exiting the application
   */
  ~SDLManager();

#ifdef RPI
  /**
   * @brief Polls PinState
   *
   * Check for changes on GPIO pins and applies them to internal bools,
   * for manipulating certain states.
   */
  void poll(const PinState &state);
#endif

  /**
   * @brief Sets up the surface and window specifications
   */
  void setup();

  /**
   * @brief Rendering function.
   *
   * Called at the end of main to present the result of values genereted.
   *
   * @param weight actual weight that gets presented on application.
   * @param clock actual date and time presented by device
   */
  void render(int weight, std::string_view clock);

  /**
   * @brief Checks the status of member variable status.
   *
   * @return state of SDL Window.
   */
  bool getStatus();

  /**
   * @brief Event poller for desktop application.
   *
   * Polls event queue used for testing certain functionalities.
   */
  void pollEvents();

private:
  enum class SurfaceState : uint8_t { MESSAGE, QR, WEIGHT } Surface;

  /**
   * @brief Helper function for SDL errors.
   *
   * @param errMsg message generated from SDL_GetError().
   */
  void printErrMsg(const char *errMsg);

  /**
   * @brief Creates the texture from the provided surfaces.
   */
  void createTextures(const std::string &logoPath, const std::string &qrPath,
                      const std::string &fontPath);

  /**
   * @brief Loads the specified IMG (.png) used for rendereing.
   */
  void loadSurfaceOfIMG(const char *filepath);

  /**
   * @brief Loads the specified TTF (.ttf) used for rendering.
   *
   * Sets the pixel depth to 400 and creates the surface needer for weight
   * texture.
   *
   * @param filepath file path to the font (.ttf).
   * @param startWeight weight to start with set to 0.
   */
  void loadFontSurface(const char *filepath);

  /**
   * @brief A constructor for the Welcome Message.
   *
   * @param messages
   * Load the litreals into memory
   */
  void createWelcomeMessage(const std::vector<std::string> &messages);

  /**
   * @brief Updates weight texture if new weight has occured.
   *
   * Creates a new texture and sets the expected width and position of the new
   * weight.
   *
   * @param newWeight the new weight to present.
   */
  void updateWeightTexture(int newWeight);

  /**
   * @brief Updates time texture if new time has occured.
   *
   * Creates a new time texture of the updated time.
   * Position will always stay the same.
   *
   * @param timepoint measured by device.
   */
  void updateTimeTexture(std::string_view timepoint);

  /**
   * @brief update the timeString to present a new time
   */
  void updateTimestring();

  /**
   * @brief Function for determining if a weight update is needed.
   *
   * Saves the previous weight for comparing the incoming weight.
   *
   * @param weight incoming weight to compare.
   *
   * @returns
   * true - if incoming weight is not the same.
   * false - if weight is higher than MAX_WEIGHT.
   */
  bool checkWeight(int weight);

  /**
   * @brief Function for determining if a time update is needed
   *
   * Stores the previous time to compare the incoming time
   *
   * @param currentTimep incoming time generated by system
   *
   * @returns
   * true - if incoming weight is not the same
   * false - if no update is needed
   */
  bool checkTime(std::string_view currentTimepoint);

  /**
   * @brief State function to for handling evnet loop.
   *
   * @return True if event is found.
   */
  bool hasEvent() const;

  /**
   * @brief Pops previous and event a pushes next event forward.
   *
   * @return Next available event, or .empty().
   */
  SDL_Event getNext();

  /**
   * @brief Determine the background color of window.
   *
   * Clears previous window and set a new rendering color,
   * based on the amount of RGB and always set to ALPHA_OPAQUE.
   *
   * @param r amount of red.
   * @param g amount of green.
   * @param b amount of blue.
   */

  void setRenderingColor(Uint8 r, Uint8 g, Uint8 b);

  /**
   * @brief Standard specifics to determine position of textures.
   *
   * Represents where a certain texture will appear on the screen measured in
   * pixels.
   *
   * @param surface pointer used by window.
   * @param x cursor of textures top left corner.
   * @param y cursor of textures top left corner.
   * @param w width of texture.
   * @param h height of texture.
   */
  void setSurfacePosition(SDLSpec *surface, Uint16 x, Uint16 y, Uint16 w,
                          Uint16 h);

  /**
   * @brief
   * Configuarion for the positioning of messages.
   *
   * @param strings Reference to literals defined in setup()
   * @param vec reference to the SDLMessage configuration.
   */
  void setMessagePositionOf(const std::vector<std::string> &strings,
                            std::vector<SDLMessage> &vec);

  /**
   * @brief The length of new incoming weight.
   *
   * Checks the length and returns the right amount for setting a new font
   * width.
   *
   * @param weight incoming weight.
   *
   * @return the width of new weight.
   */
  int checkLengthOfWeight(int weight);

  /**
   * @brief Sets the wanted width of weight texture.
   *
   * Sets a new width relative to the weight.
   * Called whenever current weight gets updated.
   *
   * @param weight
   * Depending of the size of the weight.
   */
  void setWeightWidth(int weight);

  // Raw pointers to SDL instances.

  SDL_Window *getRawWindow() const;
  SDL_Renderer *getRawRenderer() const;
  SDL_Surface *getRawSurface() const;
  SDL_Texture *getRawLogo() const;
  SDL_Texture *getRawTime() const;
  SDL_Texture *getRawImage() const;
  SDL_Texture *getRawWeight() const;
  SDL_Texture *getRawMessage(int index) const;
  TTF_Font *getRawFont() const;

  // MEMBER VARIABLES

  /**
   * @brief The status of SDL Windows.
   *
   * Represents the state of the SDL Window. Set true when window is running,
   * and is set false when shutdown is requested.
   */
  bool status = true;

  /**
   * @brief
   *
   * true, presents a weight generated if set to false.
   */
  bool showWeight = false;

  bool keyOverride = false;

  /**
   * @brief State of rendering, present welcome message first.
   */
  SurfaceState renderStates = SurfaceState::MESSAGE;

  /**
   * @brief Event handler for dekstop application.
   *
   * Queue for handling events made on the SDL Application Window.
   * Only used on x86 build.
   */
  std::queue<SDL_Event> events;
  SDL_Event event; // Single event happening.

  int weightWidth = 0; // Width of font (dynamic during runtime).
  int weightX = 0;     // X cursor of font (dynamic during runtime).

  std::string timepoint; // Store the incoming timepoint

  SDLSpec logoSpec;   // Specs for the logo presented (bottom right).
  SDLSpec timeSpec;   // Specs for the time presented (bottom left).
  SDLSpec qrSpec;     // Specs for the qr images presented (centered).
  SDLSpec weightSpec; // Specs for the weight presented (centered).

  std::vector<SDLMessage> message;   // Vector for multiple lines.
  sdl_unique<SDL_Texture> logo;      // Texture for logo (always visible).
  sdl_unique<SDL_Texture> time;      // Texture for timestamp (always visible)
  sdl_unique<SDL_Texture> image;     // Texture for QR code.
  sdl_unique<SDL_Texture> weight;    // Texture for weight.
  sdl_unique<SDL_Surface> surface;   // Surface.
  sdl_unique<TTF_Font> font;         // Font.
  sdl_unique<SDL_Renderer> renderer; // Renderer.
  sdl_unique<SDL_Window> window;     // Window.
};

#endif