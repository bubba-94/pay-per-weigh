#include "Device.hpp"

Device::Device() : state{true} {
  if (!connectToPort()) {
    std::cout << "[DEVICE] Port connection failed\n";
  }

  for (size_t i = 0; i < REQ.size() - sizeof(uint16_t); ++i) {
    reqChkSum += REQ[i];
  }
  reqChkSum = (((reqChkSum << 1) & 0xFF00) | (reqChkSum & 0x7F) | 0xC000);

  // Start working threads
  workers.emplace_back(&Device::timeThread, this);
  workers.emplace_back(&Device::weightThread, this, 2000);
}
Device::~Device() {

  // End sessions
  state = false;

  // Join all threads
  for (auto &thread : workers) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  close(fd);
}

int Device::getWeight() { return static_cast<int>(weight); }
std::string_view Device::getTimepoint() const { return timepoint; }

void Device::weightThread(const int DELAY_MS) {
  auto nextRequest = std::chrono::steady_clock::now();

  while (state.load()) {

    nextRequest += std::chrono::milliseconds(DELAY_MS);

    sendRequest();

    readResponse();

    // std::this_thread::sleep_until(nextRequest);
  }
}

void Device::timeThread() { setTime(); }

void Device::sendRequest() {

  // ssize_t written = 0;

  // Should write 1 byte at a time.
  for (size_t index = 0; index < REQ.size(); ++index) {
    ssize_t bytes = write(fd, &REQ[index], 1);

    // Break at no bytes
    if (bytes <= 0)
      break;

    // written += bytes;
  }

  // Reset index for next request
  std::cout << "[REQUEST] Checksum: " << static_cast<int>(reqChkSum) << ".\n";
}

bool Device::checkWeight() { return true; }

/*
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
*/

void Device::convertWeight() {
  std::lock_guard<std::mutex> lock(dataMtx);
  uint16_t raw = (response[5] << 8) | response[6] | (response[7] & 0x80) << 1;

  // Match protocol
  raw /= 10;

  // Map weight to 0 -> 9000
  weight = (raw - 0) * (9000 - 0) / (8 - 0) + 0;
}

void Device::readResponse() {

  bool valid = false;
  ssize_t bytes = 0;
  response.resize(16);

  for (size_t i = 0; i < response.size(); ++i) {
    // Break if response buffer is empty
    if (response.empty())
      std::cout << "[RESPONSE] Buffer empty\n";

    bytes = read(fd, &response[i], 1);

    if (bytes <= 0)
      break;
  }

  valid = checkValidChecksum();

  convertWeight();
  // Clear if not valid.
  if (valid) {
    // Convert and then clear for new weight
    convertWeight();
    response.clear();
  } else {
    std::cout << "[RESPONSE] Clearing buffer | No valid Checksum\n";
    response.clear();
  }
}

bool Device::checkValidChecksum() {
  if (reqChkSum == calculatedCheckSum()) {
    return true;
  } else
    return false;
}

uint16_t Device::calculatedCheckSum() {

  uint16_t resCheckSum = 0;
  // size_t curIndex = 0;
  // size_t startIndex = 0;

  for (size_t i = 0; i < response.size() - sizeof(uint16_t); i++) {
    resCheckSum += response[i];
  }

  // Construct 16 bit value
  resCheckSum = (((resCheckSum << 1) & 0xFF00) | (resCheckSum & 0x7F)) | 0xC000;
  std::cout << "[SERIAL] Response checksum: " << static_cast<int>(resCheckSum)
            << "\n";
  return resCheckSum;
}

// For testing with ARDUINO
// void Device::readFromSerial() {

//   char c;
//   while (state.load()) {
//     int bytes = read(fd, &c, 1);

//     if (bytes <= 0)
//       break;

//     std::lock_guard<std::mutex> lock(mutex);

//     // Clear on end of line
//     if (c == '\n' || c == '\r') {
//       // Convert to int
//       if (!incomingWeight.empty()) {
//         convertWeight();
//         incomingWeight.clear();
//       }
//     } else {
//       incomingWeight += c;
//     }
//   }
// }

void Device::setTime() {
  while (state.load()) {

    // Used for measuring clock updates
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    {
      // Aquire lock for updates.
      std::tm tm{};
      localtime_r(&t, &tm);
      std::lock_guard<std::mutex> lock(dataMtx);
      std::strftime(std::data(timeString), std::size(timeString),
                    "%d/%m-%y %H:%M", &tm);

      timepoint = timeString;
    }

    // Align to next real-world minute
    auto nextMinute = std::chrono::time_point_cast<std::chrono::minutes>(now) +
                      std::chrono::minutes(1);

    std::cout << "[DEVICE] " << timeString << "\n";

    // Shorten the sleep if needed. (shutdown)
    while (state.load() && std::chrono::system_clock::now() < nextMinute) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
  }
}

bool Device::connectToPort() {

  // Open port before configuration
  fd = open(PORT_A, O_RDWR | O_NOCTTY);
  if (fd < 0) {
    std::cout << "[DEVICE] Port opening failed\n";
    return false;
  }
  struct termios pts;

  if (tcgetattr(fd, &pts) != 0) {
    std::cout << "[DEVICE] Existing settings not read\n";
    return false;
  }
  // Terminal confiuration instance
  configureSerial(pts, 2400);

  if (tcsetattr(fd, TCSANOW, &pts) != 0) {
    std::cout << "[DEVICE] Saving new setting not successful";
    return false;
  }

  std::cout << "[DEVICE] " << PORT_A << " is open." << "\n";

  return true;
}

void Device::configureSerial(termios &settings, int baud) {
  // Control modes (how the data is packed)
  // Bit clearing (off)
  settings.c_cflag &= ~(PARENB | CSIZE | CRTSCTS);

  // Bit setting (on)
  settings.c_cflag |= (CS8 | CREAD | CLOCAL);

  // Local modes (how data is interpreted locally)
  // Clear local modes
  settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

  // Input modes
  // Clear special handling of bytes
  settings.c_iflag &= ~(IXON | IXOFF | IXANY);
  settings.c_iflag &=
      ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

  // Output modes
  // Clear special handling of bytes
  settings.c_oflag &= ~(OPOST | ONLCR);

  settings.c_cc[VTIME] = 10;
  settings.c_cc[VMIN] = 0;

  cfsetispeed(&settings, baud);
  cfsetospeed(&settings, baud);
}