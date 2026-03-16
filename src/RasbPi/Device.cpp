#include "RasbPi/Device.hpp"

#include <iomanip>

using namespace RasbPi;

Device::Device(const std::string &port) : state{true} {
  if (!connectToPort(port.c_str())) {
    std::cout << "[DEVICE] Port connection failed\n";
  }

  // Start working threads
  workers.emplace_back(&Device::timeThread, this, 200);
  workers.emplace_back(&Device::weightThread, this, 100);
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

int Device::getWeight() {
  std::lock_guard<std::mutex> lock(dataMtx);
  return static_cast<int>(weight);
}
std::string_view Device::getTimepoint() const { return timepoint; }

void Device::weightThread(const int DELAY_MS) {
  auto nextRequest = std::chrono::steady_clock::now();

  while (state.load()) {

    nextRequest += std::chrono::milliseconds(DELAY_MS);

    sendRequest();

    readResponse();

    std::this_thread::sleep_until(nextRequest);
  }
}

void Device::timeThread(const int DELAY_MS) { setTime(DELAY_MS); }

void Device::sendRequest() {

  // Should write 1 byte at a time.
  for (size_t index = 0; index < REQ.size(); ++index) {
    ssize_t bytes = write(fd, &REQ[index], 1);

    // Break at no bytes
    if (bytes <= 0)
      break;
  }
}

bool Device::checkWeight() { return true; }

void Device::convertWeight() {

  // Aquire lock first
  std::lock_guard<std::mutex> lock(dataMtx);

  // Fill weight with buffer values
  weight = (response[5] << 8) | response[6];

  weight = mapIncomingValue(weight, 0, 158, 0, 15000);
}

int Device::mapIncomingValue(int value, int inMin, int inMax, int outMin,
                             int outMax) {
  if (inMax == inMin)
    return outMin;

  int result =
      static_cast<int>(value - inMin) * (outMax - outMin) / (inMax - inMin) +
      outMin;

  return static_cast<int>(result);
}

void Device::readResponse() {

  uint8_t byte;

  while (read(fd, &byte, 1) == 1) {

    switch (rwframe) {

    case Frame::FIND_START_FRAME:
      // Start frame
      if ((byte & 0xC0) == 0x80) {
        response.clear();
        response.push_back(byte);
        rwframe = Frame::IN_FRAME;
      }
      break;

    case Frame::IN_FRAME:
      response.push_back(byte);
      // Find end frame | MASK two highest bits
      if ((byte & 0xC0) == 0xC0) {

        // 1. Check valid checksum
        if (checkValidChecksum()) {
          // 2. Re apply bits
          reApplyBits();
          // 3. Convert weight
          convertWeight();
        }

        response.clear();
        // Find start frame again
        rwframe = Frame::FIND_START_FRAME;
      }
      break;
    }
  }
}

bool Device::checkValidChecksum() {

  if (calculateCheckSum() == getCheckSum(response.size() - 2)) {
    return true;
  }

  else {
    return false;
  }
}

uint16_t Device::getCheckSum(size_t startIndex) {
  uint16_t chkSum = 0;

  if (startIndex < response.size() - 1) {

    // Big endian (WRONG)
    // chkSum = (response[startIndex] << 8) | response[(startIndex +
    // sizeof(uint8_t))]

    // Little endian (RIGHT)
    chkSum =
        response[startIndex] | (response[(startIndex + sizeof(uint8_t))] << 8);

    return chkSum;
  }

  // Error message?
  return 1;
}

uint16_t Device::calculateCheckSum() {

  uint16_t resCheckSum = 0;

  for (size_t i = 0; i < response.size() - 2; i++) {
    resCheckSum += response[i];
  }

  resCheckSum = (((resCheckSum << 1) & 0xFF00) | (resCheckSum & 0x7F)) | 0xC000;

  return resCheckSum;
}

void Device::reApplyBits() {

  // Set MSBs to 1.
  if (response[7] & 0x01) {
    response[5] |= 0x80;
  }
  if (response[7] & 0x02) {
    response[6] |= 0x80;
  }
}

void Device::setTime(const int DELAY_MS) {
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
      std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
    }
  }
}

bool Device::connectToPort(const std::string &PORT) {

  // Open port before configuration
  fd = open(PORT.c_str(), O_RDWR | O_NOCTTY);
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
  configureSerial(pts);

  if (tcsetattr(fd, TCSANOW, &pts) != 0) {
    std::cout << "[DEVICE] Saving new setting not successful";
    return false;
  }

  std::cout << "[DEVICE] " << PORT << " is open." << "\n";

  return true;
}

void Device::configureSerial(termios &settings) {
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

  cfsetispeed(&settings, B2400);
  cfsetospeed(&settings, B2400);
}