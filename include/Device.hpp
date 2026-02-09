#ifndef DEVICE_HPP
#define DEVICE_HPP

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

constexpr uint8_t DELAY = 16;
constexpr uint8_t BUFFER_LENGTH = 32;

// Test port
constexpr const char *PORT_A = "/dev/ttyACM0";

/**
 * @class Device
 * @brief Class for handling the Raspberry Pi serial port reading.
 * @details Poll a serial port for incoming weight and present the result to
 * SDL Window.
 */
class Device {
public:
  /**
   * @brief Constructor that initiates state variable and connects to port
   */
  Device();

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
   * @brief Thread function that runs readFromSerial().
   *
   */
  void weightThread(const int DELAY_MS);

  /**
   * @brief Thread function that runs readTime().
   */
  void timeThread();

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
   * @return True - if response checksum and request checksum matches.
   */
  bool checkValidChecksum();

  /**
   * @brief Calculate checksum / might not be needed
   *
   * @return The checksum of response buffer.
   */
  uint16_t calculatedCheckSum();

  /**
   * @brief Convert incoming weight from response buffer to interger value .
   */
  void convertWeight();

  /**
   * @brief Reads fd and until condtion is met.
   *
   * Pushes a weight that is later converted to an int.
   */
  // void readFromSerial();

  /**
   * @brief Set the current time point.
   */
  void setTime();

  /**
   * @brief set the current timepoint
   */
  void initiateTime();

  /**
   * @brief Opens fd and configures the serial port.
   *
   * Called in the constructor to start a succesful connection before reading
   * from it.
   *
   * @return true if succesful.
   */
  bool connectToPort();

  /**
   * @brief Configuarion of a port to represent a common RS232
   *
   * @param settings of configured port.
   * @param baud rating of port, must match both ends.
   */
  void configureSerial(termios &settings, int baud);

  /**
   * @brief File descriptor of open port being used.
   */
  int fd;

  /**
   * @brief Threads running.
   */
  std::vector<std::thread> workers;

  /**
   * @brief Response buffer.
   *
   * @details
   * Fill with the response from request and extract the actual weight.
   * Flush and resize?
   */
  std::vector<uint8_t> response = {0};

  /**
   * @brief Constant request for retrieving weight.
   */
  const std::array<uint8_t, 7> REQ = {0xB0, 0x00, 0x41, 0x03, 0x02, 0x76, 0xC1};

  /**
   * @brief constant checksum of request byte array
   */
  uint16_t reqChkSum{};

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
  std::atomic<uint16_t> weight{};

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

#endif