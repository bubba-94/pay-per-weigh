#ifdef RPI
#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "../external-libs/loggr/moody/Loggr.hpp"

// Serial/terminal communication
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// C++ Standard
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// Timestamp
#include <ctime>

// Thread safe specifics
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

namespace RasbPi {

/**
 * @class Device
 *
 * @brief
 * Class for handling the Raspberry Pi serial port reading.
 *
 * @details
 * Poll a serial port for real-time data and present a valid response to the UI
 */
class Device {
public:
  enum class Frame : std::uint8_t { FIND_START_FRAME, IN_FRAME };

  /**
   * @brief Constructor that initiates state variable and connects to port
   *
   * @param port Provide a port to open (ttyACM 0 or 1 in my case)
   */
  Device(moody::Loggr &logger, const std::string &port);

  /**
   * @brief Destructor that sets the state and joins threads.
   */
  ~Device();

  /**
   * @brief Getter for the weight.
   *
   * Gets the current weight in main logic.
   */
  int getWeight();

  /**
   * @brief Getter for the clock string
   *
   * Gets the current timePoint string
   */
  std::string_view getTimepoint() const;

private:
  /**
   * @brief
   * Weight thread that handles the request and response made to the C Vibe
   *
   * @param DELAY_MS
   * Delay before a new request is made.
   */
  void weightThread(const int DELAY_MS);

  /**
   * @brief Thread function that runs readTime().
   *
   *
   */
  void timeThread(const int DELAY_MS);

  /**
   * @brief Send request every delayMs.
   *
   * @return true when bytes are available
   */
  void sendRequest();

  /**
   * @brief Function to determine if an update is needed
   *
   * @return true if update is needed.
   */
  bool checkWeight();

  /**
   * @brief Read the response from the C Vibe
   *
   * @return false if not successful
   */
  void readResponse();

  /**
   * @brief Evaluate two checksum.
   *
   * @return True if incoming and evaluated checksums match.
   */
  bool checkValidChecksum();

  /**
   * @brief
   * Used for evaluating a packet frames integrity.
   * Construct a 16 bit value from the last two provided in the buffer.
   *
   * @param index
   * Start index at last two indexes of response buffer.
   *
   * @return
   * Checksum of current packet.
   */
  uint16_t getCheckSum(size_t index);

  /**
   * @brief
   * Calculate checksum / might not be needed
   *
   * @return The checksum of response buffer.
   */
  uint16_t calculateCheckSum();

  /**
   * @brief
   * Reapply neccesary bits so the weight value is correct.
   */
  void reApplyBits();

  /**
   * @brief
   * Convert incoming weight from response buffer to interger value .
   */
  void convertWeight();

  /**
   * @brief
   * Map the incoming buffer values to a certain range
   *
   * @param value The varaible to map/convert
   * @param inMin Minimum value of from Hub
   * @param inMax Maximum value of from Hub
   * @param outMin Minimum value of desired presented value
   * @param outMax Maximum value of desired presented value
   */
  int mapIncomingValue(int value, int inMin, int inMax, int outMin, int outMax);

  /**
   * @brief
   * Reads fd and until condtion is met.
   *
   * Pushes a weight that is later converted to an int.
   */
  // void readFromSerial();

  /**
   * @brief Set the current time point.
   *
   * @param DELAY_MS
   * Amount of time before application is shut down after a shutdown request is
   * made
   */
  void setTime(const int DELAY_MS);

  /**
   * @brief set the current timepoint
   */
  void initiateTime();

  /**
   * @brief
   * Opens fd and configures the serial port.
   *
   * Called in the constructor to start a succesful connection before reading
   * from it.
   *
   * @return true if succesful.
   */
  bool connectToPort(const std::string &PORT);

  /**
   * @brief
   * Configuarion of a port to represent a common RS232
   *
   * @param settings of configured port.
   */
  void configureSerial(termios &settings);

  /**
   * @brief Reference to the logger object
   *
   */
  moody::Loggr &logger;

  /**
   * @brief File descriptor of open port being used.
   */
  int fd;

  /**
   * @brief Threads running.
   */
  std::vector<std::thread> workers;

  /**
   * @brief State variable for reading from buffer.
   */
  Frame rwframe = Frame::FIND_START_FRAME;

  /**
   * @brief Response buffer
   */
  std::vector<uint8_t> response = {0};

  /**
   * @brief Constant request for retrieving weight.
   */
  const std::array<uint8_t, 7> REQ = {0xB0, 0x00, 0x41, 0x03, 0x02, 0x76, 0xC1};

  /**
   * @brief Variable to store the incoming weight.
   */
  std::string incomingWeight{};

  /**
   * @brief Variable for data protection
   */
  std::mutex dataMtx{};

  /**
   * @brief Variable for storing the converted weight.
   */
  std::atomic<uint16_t> weight = {0};

  /**
   * @brief State variable used for thread.
   */
  std::atomic<bool> state{};

  /**
   * @brief Thread safe variable for updating the timepoint variable.
   *
   * Will always be presented on the SDL Window.
   */
  std::string timepoint;

  //! Design of the timepoint.
  char timeString[std::size("dd/mm-yy hh:mm")]; // Store converted local time.
};

}; // namespace RasbPi

#endif // header guard

#endif // RPI